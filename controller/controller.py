import sys
import struct

from enum import Enum
from threading import Thread, Condition, Lock

from serial import Serial

from ui import ControllerUi
from gpsParser import GPSParser


class CommandId(Enum):
    PING = 0
    GPS = 1
    CALIBRATE_MOTORS = 2
    SET_LOCATION = 3
    SET_MOTOR_POSITION = 4


class CommandSerializer:
    @classmethod
    def serializePing(cls):
        return cls._serializeCommand(CommandId.PING)

    @classmethod
    def serializeGps(cls, latitude, longitude, altitude):
        return cls._serializeCommand(
            CommandId.GPS, struct.pack('<ddd', float(latitude), float(longitude), float(altitude)))

    @classmethod
    def serializeCalibrateMotors(cls):
        return cls._serializeCommand(CommandId.CALIBRATE_MOTORS)

    @classmethod
    def serializeSetLocation(cls, latitude, longitude, altitude, orientation):
        return cls._serializeCommand(CommandId.SET_LOCATION, struct.pack(
            '<dddd', float(latitude), float(longitude), float(altitude), float(orientation)))

    @classmethod
    def serializeSetMotorPosition(cls, motor, angle):
        return cls._serializeCommand(CommandId.SET_MOTOR_POSITION, struct.pack(
            '<Bd', int(motor), float(angle)))

    @staticmethod
    def _serializeCommand(commandId, payload=b''):
        return struct.pack('<BBB', 0xAA, 0x55, commandId.value) + payload


class ConnectionThread(Thread):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._openCondition = Condition()
        self._isOpen = False
        self._isStopped = False

    def open(self, *args):
        self._isOpen = True
        with self._openCondition:
            self._openCondition.notify()

    def close(self):
        self._isOpen = False

    def stop(self):
        self._isStopped = True
        self.close()
        with self._openCondition:
            self._openCondition.notify()

    def _runCondition(self):
        while not self._isStopped and not self._isOpen:
            with self._openCondition:
                self._openCondition.wait()
        return not self._isStopped


class LaserPointingConnection(ConnectionThread):
    def __init__(self, controller):
        super().__init__(daemon=True)
        self._controller = controller
        self._connection = Serial(baudrate=9600, timeout=1)
        self._connection.set_buffer_size(rx_size=640000)
        self._sendLock = Lock()

    def open(self, port):
        self._connection.setPort(port)
        self._connection.open()
        super().open()

    def send(self, data):
        with self._sendLock:
            self._connection.write(data)

    def run(self):
        while self._runCondition():
            data = self._connection.read(self._connection.inWaiting() or 1)
            self._controller.onNewLog(data)
        self._connection.close()


class GpsParserThread(ConnectionThread, GPSParser):
    def __init__(self, controller, heightOffset=0):
        super().__init__(daemon=True)
        self._controller = controller
        self._heightOffset = heightOffset
        self._lastLocation = None

    def open(self, port):
        super().connect(port)
        super().open()

    @property
    def lastLocation(self):
        return self._lastLocation

    def _handleParsedLocation(self, location):
        location.altitude -= self._heightOffset
        self._lastLocation = location
        self._controller.onNewLocation(location)
        super()._handleParsedLocation(location)


class Controller:
    _SERIALIZERS = {
        CommandId.PING: CommandSerializer.serializePing,
        CommandId.GPS: CommandSerializer.serializeGps,
        CommandId.CALIBRATE_MOTORS: CommandSerializer.serializeCalibrateMotors,
        CommandId.SET_LOCATION: CommandSerializer.serializeSetLocation,
        CommandId.SET_MOTOR_POSITION: CommandSerializer.serializeSetMotorPosition,
    }

    def __init__(self):
        super().__init__()
        self._connection = LaserPointingConnection(self)
        self._pointingTarget = 0
        self._balloonAGpsParser = GpsParserThread(self)
        self._balloonBGpsParser = GpsParserThread(self)
        self._ui = ControllerUi(self)

    def run(self):
        threads = [self._connection, self._balloonAGpsParser, self._balloonBGpsParser]
        for thread in threads:
            thread.start()
        result = self._ui.exec()
        print('Exiting...')
        for thread in threads:
            thread.stop()
        for thread in threads:
            thread.join()
        return result

    def sendCommand(self, commandText):
        arguments = commandText.split()
        command = arguments.pop(0)
        try:
            commandId = CommandId[command.upper()]
        except KeyError:
            raise ValueError(f'Invalid command: "{command}"')
        try:
            commandData = self._SERIALIZERS[commandId](*arguments)
        except TypeError as error:
            raise ValueError(f'Invalid argument(s): "{error}"')
        self._connection.send(commandData)

    def setPointingSystemPort(self, port):
        print(f'Connecting to port {port}...')
        self._connection.open(port)

    def setRtkAPort(self, port):
        print(f'RTK A : connection to the port {port}...')
        self._balloonAGpsParser.connect(port)

    def setRtkBPort(self, port):
        print(f'RTK B : connection to the port {port}...')
        self._balloonBGpsParser.connect(port)

    def setPointingTarget(self, target):
        print(f'Set the target to {target}')
        self._pointingTarget = target

    def onNewLocation(self, source, location):
        if self._pointingTarget != [self._balloonAGpsParser, self._balloonBGpsParser].index(source):
            command = self._SERIALIZERS[CommandId.GPS](
                location.latitude, location.longitude, location.altitude)
            self._connection.send(command)

    def onNewLog(self, data):
        text = data.decode('utf-8', errors='replace')
        print(text, end='', flush=True)
        self._ui.addLog(text)


def main():
    controller = Controller()
    return controller.run()


if __name__ == '__main__':
    sys.exit(main())
