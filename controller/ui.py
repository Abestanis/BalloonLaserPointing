import sys

from serial import SerialException
from serial.tools.list_ports import comports
from PyQt5.QtGui import QTextCursor
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QWidget, QPlainTextEdit, QApplication, QLineEdit, QPushButton, \
    QHBoxLayout, QVBoxLayout, QLabel, QComboBox, QErrorMessage


class ControllerUi(QApplication):
    """ A user interface for the controller. """
    _newLog = pyqtSignal(str)  # A signal that will get emitted when new logging data arrives.

    def __init__(self, controller):
        """
        Initialize the controller user interface and show it.
        
        :param controller: The controller who communicates with the RTKs and the Arduino. 
        """
        super().__init__(sys.argv)
        self._controller = controller
        self.setStyle('fusio')
        self._window = QWidget()
        self._logTextWidget = QPlainTextEdit()
        self._setUI()
        self._newLog.connect(self._appendLog)

    def addLog(self, text):
        """
        Add some new log to be displayed in the UI.
        This function can be called from any thread.

        :param text: The text of the log that should be added.
        """
        self._newLog.emit(text)

    def _setUI(self):
        """ Initialize and show the user interface. """
        self._logTextWidget.setReadOnly(True)
        self._commandComboBox = QComboBox()
        for command in self._controller.COMMANDS:
            self._commandComboBox.addItem(command.name, command)

        self._commandEdit = QLineEdit()
        self._commandEdit.returnPressed.connect(self._onSendCommand)
        sendButton = QPushButton('Send', self._window)
        sendButton.setShortcut('Enter')
        sendButton.clicked.connect(self._onSendCommand)

        prompt = QHBoxLayout()

        prompt.addWidget(self._commandComboBox)
        prompt.addWidget(self._commandEdit)
        prompt.addWidget(sendButton)

        leftBox = QVBoxLayout()
        leftBox.addWidget(self._logTextWidget)
        leftBox.addLayout(prompt)

        refreshButton = QPushButton('Refresh Ports', self._window)
        refreshButton.clicked.connect(self._refreshPortList)

        controllerLabel = QLabel('Controller')
        self._pointingSystemPortComboBox = QComboBox(self._window)
        controllerButton = QPushButton('Connect', self._window)
        controllerButton.clicked.connect(lambda: self._catchSerialError(
            self._controller.setPointingSystemPort, self._pointingSystemPortComboBox.currentText()))

        rtkALabel = QLabel('RTK - Balloon A')
        self._rtkAPortComboBox = QComboBox(self._window)
        rtkAButton = QPushButton("Connect", self._window)
        rtkAButton.clicked.connect(lambda: self._catchSerialError(
            self._controller.setRtkAPort, self._rtkAPortComboBox.currentText()))

        rtkBLabel = QLabel('RTK - Balloon B')
        self._rtkBPortComboBox = QComboBox(self._window)
        rtkBButton = QPushButton('Connect', self._window)
        rtkBButton.clicked.connect(lambda: self._catchSerialError(
            self._controller.setRtkBPort, self._rtkBPortComboBox.currentText()))
        self._refreshPortList()

        targetLabel = QLabel('Target')
        targetComboBox = QComboBox(self._window)
        targetComboBox.addItem('Balloon A')
        targetComboBox.addItem('Balloon B')
        targetButton = QPushButton('Select Target', self._window)
        targetButton.clicked.connect(lambda: self._catchSerialError(
            self._controller.setPointingTarget, targetComboBox.currentIndex()))

        rightBox = QVBoxLayout()
        rightBox.addWidget(refreshButton)

        rightBox.addWidget(controllerLabel)
        rightBox.addWidget(self._pointingSystemPortComboBox)
        rightBox.addWidget(controllerButton)

        rightBox.addWidget(rtkALabel)
        rightBox.addWidget(self._rtkAPortComboBox)
        rightBox.addWidget(rtkAButton)

        rightBox.addWidget(rtkBLabel)
        rightBox.addWidget(self._rtkBPortComboBox)
        rightBox.addWidget(rtkBButton)

        rightBox.addWidget(targetLabel)
        rightBox.addWidget(targetComboBox)
        rightBox.addWidget(targetButton)

        mainLayout = QHBoxLayout()

        mainLayout.addLayout(leftBox)
        mainLayout.addLayout(rightBox)

        self._window.setLayout(mainLayout)

        self._window.setGeometry(300, 300, 1000, 500)
        self._window.setWindowTitle('Laser Pointing Controller')
        self._window.show()

    def _onSendCommand(self):
        """ Called when clicking on the 'Send' button. Send the currently entered command. """
        try:
            self._controller.sendCommand(
                self._commandComboBox.currentData(), self._commandEdit.text().split())
            self._commandEdit.clear()
        except (ValueError, SerialException) as error:
            QErrorMessage(self._window).showMessage(f'Failed to send command:\n\n{error}')

    def _refreshPortList(self):
        """ Refresh the lists of ports shown in the UI. """
        ports = [portinfo.device for portinfo in comports()]
        for comboBox in [self._pointingSystemPortComboBox,
                         self._rtkAPortComboBox, self._rtkBPortComboBox]:
            currentValue = comboBox.currentText()
            comboBox.clear()
            comboBox.addItems(ports)
            if currentValue:
                try:
                    index = ports.index(currentValue)
                    comboBox.setCurrentIndex(index)
                except ValueError:
                    pass

    def _catchSerialError(self, operation, *args, **kwargs):
        """
        Execute an operation, catch any serial exceptions and show an error dialog.

        :param operation: The operation to execute.
        :param args: Arguments for the operation.
        :param kwargs: Keyword arguments for the operation.
        """
        try:
            operation(*args, **kwargs)
        except SerialException as error:
            QErrorMessage(self._window).showMessage(f'Operation failed:\n\n{error}')

    def _appendLog(self, text):
        """
        Append text to the log window.

        :param text: The text to add.
        """
        self._logTextWidget.moveCursor(QTextCursor.End)
        self._logTextWidget.insertPlainText(text)


if __name__ == '__main__':
    class MockController:
        """ A controller that can be used for testing the UI. """

        @staticmethod
        def sendCommand(command, arguments):
            print(f'Sending {command.name} (arguments: {arguments})')

        @staticmethod
        def setPointingSystemPort(port):
            print(f'Controller : connection to the port {port}...')

        @staticmethod
        def setRtkAPort(port):
            print(f'RTK A : connection to the port {port}...')

        @staticmethod
        def setRtkBPort(port):
            print(f'RTK B : connection to the port {port}...')

        @staticmethod
        def setPointingTarget(target):
            print('Set the target to ' + target)


    sys.exit(ControllerUi(MockController()).exec())
