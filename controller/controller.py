#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import struct

from time import strftime
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
    """ A thread that manages a connection which can be opened and closed multiple times. """

    def __init__(self, *args, **kwargs):
        """
        Initialize a new connection thread.

        :param args: Arguments for the thread constructor.
        :param kwargs: Keyword arguments for the thread constructor.
        """
        super().__init__(*args, **kwargs)
        self._openCondition = Condition()
        self._isOpen = False
        self._isStopped = False

    def open(self, *args):
        """
        Open the connection and start reading on the thread.

        :param args: Arguments to the open call.
        """
        self._isOpen = True
        with self._openCondition:
            self._openCondition.notify()

    def close(self):
        """ Close the connection and put the thread into idle mode. """
        self._isOpen = False

    def stop(self):
        """ Close the connection and stop the thread. """
        self._isStopped = True
        self.close()
        with self._openCondition:
            self._openCondition.notify()

    def _runCondition(self):
        """
        A run condition that should be checked by the run function of this thread from time to time.
        If it returns False, the tread should exit.

        :return: Whether the thread should continue to run.
        """
        while not self._isStopped and not self._isOpen:
            with self._openCondition:
                self._openCondition.wait()
        return not self._isStopped


class LaserPointingConnection(ConnectionThread):
    """ The serial connection to the laser pointing system. """

    def __init__(self, controller, logDirectory):
        """
        Initialize the connection to the laser pointing system.
        This will not open the connection yet.

        :param controller: The laser pointing controller.
        """
        super().__init__(daemon=True)
        self._controller = controller
        self._connection = Serial(baudrate=9600, timeout=1)
        self._connection.set_buffer_size(rx_size=640000)
        self._sendLock = Lock()
        self._logFile = None
        self._logDirectory = logDirectory
        if self._logDirectory is not None:
            os.makedirs(self._logDirectory, exist_ok=True)

    def open(self, port):
        """
        Open the connection to the laser pointing system.

        :param port: The port that the laser pointing system is connected on.
        """
        self._connection.setPort(port)
        self._connection.open()
        if self._logDirectory:
            self._logFile = open(
                os.path.join(self._logDirectory, strftime("%Y%m%d-%H%M%S") + '.bin'), 'wb')
        super().open()

    def close(self):
        super().close()
        if self._logFile is not None:
            self._logFile.close()
            self._logFile = None

    def send(self, data):
        """
        Send some data to the laser pointing system.
        This function is thread save and will block if another thread is currently sending data.

        :param data: The data to send.
        """
        with self._sendLock:
            self._connection.write(data)

    def run(self):
        """ Read data from the laser pointing system and forward it to the controller. """
        while self._runCondition():
            try:
                data = self._connection.read(self._connection.inWaiting() or 1)
            except SerialException as error:
                print(f'Reading from pointing system serial port failed: {error}')
                self.close()
                continue
            if self._logFile is not None:
                self._logFile.write(data)
            self._controller.onNewLog(data)
        self._connection.close()


class GpsParserThread(ConnectionThread, GPSParser):
    """ A thread to read from the RTK Gps. """

    def __init__(self, controller, heightOffset=0):
        """
        Initialize a GpsParser thread, but don't connect yet.

        :param controller: The laser pointing controller.
        :param heightOffset: The height offset to subtract from the received location
                             to get the actual pointing target.
        """
        ConnectionThread.__init__(self, daemon=True)
        GPSParser.__init__(self, outputDir='logs', storeRawLog=True, storeLocationLog=True)
        self._controller = controller
        self._heightOffset = heightOffset
        self._lastLocation = None

    def run(self):
        self.parse(self._runCondition)

    def open(self, port):
        """
        Open the connection to the RTK Gps system.

        :param port: The port where the RTK is connected to.
        """
        GPSParser.open(self, port)
        super().open()

    def close(self):
        """ Close the connection with the RTK Gps system. """
        GPSParser.close(self)
        super().close()

    @property
    def lastLocation(self):
        """
        :return: The last location of this receiver.
        """
        return self._lastLocation

    def _handleParsedLocation(self, location):
        """
        Handle a new received parsed location, forward it to the controller and remember it
        as the last location. Also apply the height offset.

        :param location: The location parsed from the RTK GPS device.
        """
        location.altitude -= self._heightOffset
        self._lastLocation = location
        self._controller.onNewLocation(self, location)
        super()._handleParsedLocation(location)


class Controller:
    """ The main controller coordinating the GPS and laser pointing connection and UI. """

    # A list of all commands known to the controller.
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
        Command('SET_MOTOR_POSITION',
                lambda motor, angle: struct.pack('<Bd', int(motor), float(angle))),
        # Set the calibration angle of a motor to the current angle.
        Command('SET_CALIBRATION_POINT', lambda motor: struct.pack('<B', int(motor))),
    ]

    def __init__(self):
        """ Initialize a new controller. """
        super().__init__()
        self._connection = LaserPointingConnection(self, os.path.join('logs', 'laser'))
        self._pointingTarget = 0
        self._balloonAGpsParser = GpsParserThread(self, heightOffset=0.26)
        self._balloonBGpsParser = GpsParserThread(self, heightOffset=0.26)
        self._ui = ControllerUi(self)
        self._gpsCommand = self._findCommand('GPS')

    def run(self):
        """ Run the controller, this will show the UI. """
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

    def sendCommand(self, command, arguments):
        """
        Send a command to the laser pointing system.

        :param command: The command to send.
        :param arguments: A list of arguments for the command.
        """
        try:
            commandData = command.serialize(*arguments)
        except TypeError as error:
            raise ValueError(f'Invalid argument(s): "{error}"')
        self._connection.send(commandData)

    def setPointingSystemPort(self, port):
        """
        Set the port of the pointing system and connect to it.

        :param port: The new port of the pointing system.
        """
        print(f'Connecting to port {port}...')
        self._connection.open(port)

    def setRtkAPort(self, port):
        """
        Set the port of the RTK GPS receiver for balloon A and connect to it.

        :param port: The new port of the RTK GPS receiver for balloon A.
        """
        print(f'RTK A : connection to the port {port}...')
        self._balloonAGpsParser.open(port)

    def setRtkBPort(self, port):
        """
        Set the port of the RTK GPS receiver for balloon B and connect to it.

        :param port: The new port of the RTK GPS receiver for balloon B.
        """
        print(f'RTK B : connection to the port {port}...')
        self._balloonBGpsParser.open(port)

    def setPointingTarget(self, target):
        """
        Set the target that should be pointed to.

        :param target: The index of the target balloon.
        """
        print(f'Set the target to {target}')
        self._pointingTarget = target
        activeSource = [self._balloonAGpsParser, self._balloonBGpsParser][target]
        if activeSource.lastLocation is not None:
            self.onNewLocation(activeSource, activeSource.lastLocation)

    def onNewLocation(self, source, location):
        """
        Called when a new location is available.

        :param source: The connection that generated the location.
        :param location: The new location.
        """
        if self._pointingTarget == [self._balloonAGpsParser, self._balloonBGpsParser].index(source):
            command = self._gpsCommand.serialize(
                location.latitude, location.longitude, location.altitude)
            try:
                self._connection.send(command)
            except SerialException as error:
                print(f'Failed to send location: {error}')

    def onNewLog(self, data):
        """
        Called when there is new log available for the laser pointing system.

        :param data: The new log data as bytes.
        """
        text = data.decode('utf-8', errors='replace')
        print(text, end='', flush=True)
        self._ui.addLog(text)

    def _findCommand(self, name):
        """
        Find a command by its name.

        :raises ValueError: If no command exists with that name.
        :param name: The name of the command.
        :return: The command with that name.
        """
        try:
            return next(command for command in self.COMMANDS if command.name == name)
        except StopIteration:
            raise ValueError(f'Invalid command: "{name}"')


def main():
    """
    Run the controller program.

    :return: The exit code of the controller program.
    """
    controller = Controller()
    return controller.run()


if __name__ == '__main__':
    sys.exit(main())
