import sys
from argparse import ArgumentParser
from contextlib import nullcontext
from dataclasses import dataclass

from serial import Serial
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

    def __init__(self, rawLogFilePath=None, locationLogFile=None):
        """
        Initialize the parser.

        :param rawLogFilePath: An optional path to a log file where
                               raw communication will be recorded to.
        :param locationLogFile: An optional open file to write received locations to.
        """
        super().__init__()
        self._rawLogFilePath = rawLogFilePath
        self._locationLogFile = locationLogFile
        self._connection = Serial(baudrate=115200)
        self._connection.set_buffer_size(rx_size=640000)

    def connect(self, port):
        """
        Connect to the RTK serial port.

        :param port: The serial port.
        """
        self._connection.setPort(port)
        self._connection.open()

    def parse(self):
        """ Parse GPS messages from the serial connection until it is closed. """
        buffer = b''
        while True:
            with nullcontext() if self._rawLogFilePath is None else \
                    open(self._rawLogFilePath, 'wb') as recordingFile:
                newData = self._connection.read(self._connection.inWaiting() or 1)
                if not newData:
                    break
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


def main():
    """
    Allow printing and recording of location and raw communication date from an RTK GPS module.

    :return: The return code of the program.
    """
    parser = ArgumentParser(description='RTK GPS parser')
    parser.add_argument('-p', '--port', default=None,
                        help='The serial port that the RTK GPS receiver is connected to')
    parser.add_argument('-r', '--raw', help='A path to a log file to save the raw received data to')
    parser.add_argument('-l', '--locations',
                        help='A path to a log file to save the received locations to')
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

    with nullcontext() if arguments.locations is None else \
            open(arguments.locations, 'w') as locationsFile:
        parser = PrintingGPSParser(rawLogFilePath=arguments.raw, locationLogFile=locationsFile)
        parser.connect(port)
        parser.parse()


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass
