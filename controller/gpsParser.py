#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from time import strftime
from argparse import ArgumentParser
from contextlib import nullcontext
from dataclasses import dataclass

from serial import Serial, SerialException
from serial.tools.list_ports import comports


@dataclass
class Location:
    """ A GPS location. """
    time: float  # The GPS time when the location was recorded.
    latitude: float  # The latitude of the location in degrees.
    longitude: float  # The longitude of the location in degrees.
    altitude: float  # The height of the location in meter.


class GPSParser:
    """ Allow parsing of GPS messages from an RTK module. """

    def __init__(self, outputDir=None, storeRawLog=False, storeLocationLog=False):
        """
        Initialize the parser.

        :param outputDir: The directory where logs are stored. Defaults to the current directory.
        :param storeRawLog: Whether to store the raw communication. The storage location will be
                            in a 'raw' subdirectory of the outputDir.
        :param storeLocationLog: Whether to store the received locations. The storage location
                                 will be in a 'location' subdirectory of the outputDir.
        """
        super().__init__()
        if outputDir is not None:
            os.makedirs(outputDir, exist_ok=True)
        else:
            outputDir = '.'
        if storeRawLog:
            self._rawLogDir = os.path.join(outputDir, 'raw')
            os.makedirs(self._rawLogDir, exist_ok=True)
        else:
            self._rawLogDir = None
        self._locationLogFile = None
        self._outputDir = outputDir
        self._storeLocationLog = storeLocationLog
        self._connection = Serial(baudrate=115200, timeout=1)
        self._connection.set_buffer_size(rx_size=640000)

    def open(self, port):
        """
        Connect to the RTK serial port.

        :param port: The serial port.
        """
        if self._storeLocationLog:
            locationLogDir = os.path.join(self._outputDir, 'location')
            os.makedirs(locationLogDir, exist_ok=True)
            self._locationLogFile = open(self._getLogPath(locationLogDir, '.csv'), 'a')
        else:
            self._locationLogFile = None
        self._connection.setPort(port)
        self._connection.open()

    def close(self):
        """ Disconnect from the serial port. """
        self._connection.close()
        locationLogFile = self._locationLogFile
        if locationLogFile:
            self._locationLogFile = None
            locationLogFile.close()

    def parse(self, runCondition=None):
        """
        Parse GPS messages from the serial connection until it is closed.

        :param runCondition: A function that can return False to stop the parser loop.
        """
        buffer = b''
        with nullcontext() if self._rawLogDir is None else \
                open(self._getLogPath(self._rawLogDir, '.bin'), 'wb') as recordingFile:
            while runCondition is None or runCondition():
                try:
                    newData = self._connection.read(self._connection.inWaiting() or 1)
                except SerialException as error:
                    print(f'Reading from RTK serial port failed: {error}')
                    self.close()
                    continue
                if recordingFile is not None:
                    recordingFile.write(newData)
                buffer += newData
                lines = buffer.split(b'\n')
                buffer = lines.pop()
                for line in lines:
                    self._parseLocationFrom(line)

    def _parseLocationFrom(self, line):
        """
        Try to parse a location for the received line.

        :param line: The line which was received.
        """
        if not line.startswith(b'$GNGGA,'):
            return
        parts = line.split(b',')
        if len(parts) < 15:
            return
        try:
            location = Location(
                time=float(parts[1]),
                latitude=self._parseAngle(parts[2], degreeDecimals=2) * (
                    -1 if parts[3] == b'S' else 1),
                longitude=self._parseAngle(parts[4], degreeDecimals=3) * (
                    -1 if parts[5] == b'W' else 1),
                altitude=float(parts[9]),
            )
        except ValueError as error:
            print(f'Failed to parse location line: {error}')
            return
        self._handleParsedLocation(location)

    @staticmethod
    def _parseAngle(value, degreeDecimals):
        """
        Parse an angle in degrees from a degree and minute format.

        :param value: The angle encoded in a degree and minute format.
        :param degreeDecimals: How many decimals at the beginning belong to the degree.
        :return: The parsed angle in degrees.
        """
        minutes = float(value[degreeDecimals:])
        return int(value[:degreeDecimals]) + minutes * (1 / 60)

    def _handleParsedLocation(self, location):
        """
        Handle a new parsed location.

        :param location: The parsed location.
        """
        self._saveLocation(location)

    def _saveLocation(self, location):
        """
        Write the location to the location log file.

        :param location: The location to write.
        """
        if self._locationLogFile is not None:
            self._locationLogFile.write(
                f'{location.time},{location.latitude},{location.longitude},{location.altitude}\n')

    @staticmethod
    def _getLogPath(directory, ending):
        """
        Create a log file path with the given ending.

        :param directory: The directory to create the log file path in.
        :param ending: The file extension.
        :return: A path to a log file.
        """
        return os.path.join(directory, strftime("%Y%m%d-%H%M%S") + ending)


def main():
    """
    Allow printing and recording of location and raw communication date from an RTK GPS module.

    :return: The return code of the program.
    """
    parser = ArgumentParser(description='RTK GPS parser')
    parser.add_argument('-p', '--port', default=None,
                        help='The serial port that the RTK GPS receiver is connected to')
    parser.add_argument('-r', '--raw', action='store_true', help='Enable storing raw received data')
    parser.add_argument('-l', '--locations', action='store_true',
                        help='Enable storing the received locations')
    parser.add_argument('-o', '--outputs', help='A path to a directory to save recordings to')
    arguments = parser.parse_args()
    port = arguments.port
    if port is None:
        allPorts = comports()
        if len(allPorts) == 1:
            port = allPorts[0].device
        elif allPorts:
            print('Please specify one of the following ports using the --port argument:')
            for port in allPorts:
                print(f'{port.device} ({port.description})')
            return 1
        else:
            print('No serial ports found')
            return 1

    class PrintingGPSParser(GPSParser):
        """ A GPSParser that prints the received locations. """

        def _handleParsedLocation(self, location):
            print(location)
            super()._handleParsedLocation(location)

    parser = PrintingGPSParser(
        arguments.output, storeRawLog=arguments.raw, storeLocationLog=arguments.locations)
    parser.open(port)
    parser.parse()
    parser.close()


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass
