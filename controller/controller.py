import struct

from enum import Enum
from threading import Thread
from contextlib import contextmanager

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
        self._connection = Serial(baudrate=9600, timeout=1)
        self._connection.set_buffer_size(rx_size=640000)
        self._isOpen = False

    @contextmanager
    def open(self):
        self._connection.setPort('COM18')
        self._connection.open()
        self._isOpen = True
        yield
        self._isOpen = False

    def send(self, data):
        self._connection.write(data)

    def run(self):
        while self._isOpen:
            data = self._connection.read(self._connection.inWaiting() or 1)
            print(data.decode('utf-8', errors='replace'), end='', flush=True)
        self._connection.close()


class Controller:
    _SERIALIZERS = {
        CommandId.PING: CommandSerializer.serializePing,
        CommandId.GPS: CommandSerializer.serializeGps,
    }

    def __init__(self):
        super().__init__()
        self._connection = Connection()

    def run(self):
        with self._connection.open():
            self._connection.start()
            while True:
                inputData = input()
                arguments = inputData.split()
                command = arguments.pop(0)
                if command == 'q' or command == 'quit':
                    break
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
        print('Exiting...')
        self._connection.join()


def main():
    controller = Controller()
    controller.run()


if __name__ == '__main__':
    main()
