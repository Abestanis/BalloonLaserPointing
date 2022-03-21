#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from threading import Thread, Condition, Lock

from serial import Serial, SerialException

from ui import ControllerUi
from gpsParser import GPSParser


class Command:
    """ A telecommand that can be sent to the pointing system. """

    def __init__(self, name, parameterSerializer=None):
        """
        Initialize a new telecommand.

        :param name: The name of the telecommand.
        :param parameterSerializer: An optional serializer for parameters.
        """
        super().__init__()
        self._name = name
        self._parameterSerializer = parameterSerializer

    @property
    def name(self):
        """
        :return: The name of this telecommand.
        """
        return self._name

    def serialize(self, *args, **kwargs):
        """
        Serialize this telecommand, including any required parameters.

        :param args: Parameters for the telecommand, will be passed to the serializer.
        :param kwargs: Parameters for the telecommand, will be passed to the serializer.
        :return: The serialized command.
        """
        commandId = Controller.COMMANDS.index(self)
        payload = b'' if self._parameterSerializer is None else \
            self._parameterSerializer(*args, **kwargs)
        return struct.pack('<BBB', 0xAA, 0x55, commandId) + payload


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

    def run(self):
        self.parse(self._runCondition)

    def open(self, port):
        super().connect(port)
        super().open()

    @property
    def lastLocation(self):
        return self._lastLocation

    def _handleParsedLocation(self, location):
        location.altitude -= self._heightOffset
        self._lastLocation = location
        self._controller.onNewLocation(self, location)
        super()._handleParsedLocation(location)


class Controller:
    COMMANDS = [
        # Send a PING, expect a PONG back.
        Command('PING'),
        # Set the GPS position of the pointing target.
        Command('GPS', lambda latitude, longitude, altitude: struct.pack(
            '<ddd', float(latitude), float(longitude), float(altitude))),
        # Trigger the automatic calibration of the motors.
        Command('CALIBRATE_MOTORS'),
        # Set the position and zero pointing orientation of the structure.
        Command('SET_LOCATION', lambda latitude, longitude, altitude, orientation: struct.pack(
            '<dddd', float(latitude), float(longitude), float(altitude), float(orientation))),
        # Manually set the motor position to a specific angle.
        Command('SET_MOTOR_POSITION', lambda motor: struct.pack('<Bd', int(motor))),
        # Set the calibration angle of a motor to the current angle.
        Command('SET_CALIBRATION_POINT'),
    ]

    def __init__(self):
        super().__init__()
        self._connection = LaserPointingConnection(self)
        self._pointingTarget = 0
        self._balloonAGpsParser = GpsParserThread(self)
        self._balloonBGpsParser = GpsParserThread(self)
        self._ui = ControllerUi(self)
        self._gpsCommand = self._findCommand('GPS')

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
        command = self._findCommand(arguments.pop(0))
        try:
            commandData = command.serialize(*arguments)
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
        activeSource = [self._balloonAGpsParser, self._balloonBGpsParser][target]
        self.onNewLocation(activeSource, activeSource.lastLocation)

    def onNewLocation(self, source, location):
        if self._pointingTarget == [self._balloonAGpsParser, self._balloonBGpsParser].index(source):
            command = self._gpsCommand.serialize(
                location.latitude, location.longitude, location.altitude)
            try:
                self._connection.send(command)
            except SerialException as error:
                print(f'Failed to send location: {error}')

    def onNewLog(self, data):
        text = data.decode('utf-8', errors='replace')
        print(text, end='', flush=True)
        self._ui.addLog(text)

    def _findCommand(self, name):
        try:
            return next(command for command in self.COMMANDS if command.name == name)
        except StopIteration:
            raise ValueError(f'Invalid command: "{name}"')


def main():
    controller = Controller()
    return controller.run()


if __name__ == '__main__':
    sys.exit(main())
