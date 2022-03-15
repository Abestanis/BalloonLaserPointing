import struct

from enum import Enum
from threading import Thread

from serial import Serial


class CommandId(Enum):
    PING = 0
    GPS = 1


class CommandSerializer:
    @classmethod
    def serializePing(cls):
        return cls._serializeCommand(CommandId.PING)

    @classmethod
    def serializeGps(cls, latitude, longitude, altitude):
        return cls._serializeCommand(
            CommandId.GPS, struct.pack('<ddd', float(latitude), float(longitude), float(altitude)))

    @staticmethod
    def _serializeCommand(commandId, payload=b''):
        return struct.pack('<BBB', 0xAA, 0x55, commandId.value) + payload


class Connection(Thread):
    def __init__(self):
        super().__init__(daemon=True)
        self._connection = Serial(baudrate=9600)
        self._connection.set_buffer_size(rx_size=640000)

    def open(self):
        self._connection.setPort('COM18')
        self._connection.open()

    def send(self, data):
        self._connection.write(data)

    def run(self):
        while True:
            data = self._connection.read(self._connection.inWaiting() or 1)
            print(data.decode('utf-8', errors='replace'), end='', flush=True)


class Controller:
    _SERIALIZERS = {
        CommandId.PING: CommandSerializer.serializePing,
        CommandId.GPS: CommandSerializer.serializeGps,
    }

    def __init__(self):
        super().__init__()
        self._connection = Connection()

    def run(self):
        self._connection.open()
        self._connection.start()
        while True:
            inputData = input()
            arguments = inputData.split()
            command = arguments.pop(0)
            try:
                commandId = CommandId[command.upper()]
            except KeyError:
                print('=' * 30 + f'\nInvalid command: "{command}"\n' + '=' * 30)
                continue
            try:
                commandData = self._SERIALIZERS[commandId](*arguments)
            except TypeError as error:
                print('=' * 30 + f'\nInvalid argument(s): "{error}"\n' + '=' * 30)
                continue
            self._connection.send(commandData)


def main():
    controller = Controller()
    controller.run()


if __name__ == '__main__':
    main()
