import sys
from PyQt5.QtCore import pyqtSignal

from serial import SerialException
from serial.tools.list_ports import comports
from PyQt5.QtWidgets import QWidget, QTextEdit, QApplication, QLineEdit, QPushButton, QHBoxLayout, \
    QVBoxLayout, QLabel, QComboBox, QErrorMessage


class ControllerUi(QApplication):
    _newLog = pyqtSignal(str)

    def __init__(self, controller):
        super().__init__(sys.argv)
        self._controller = controller
        self.setStyle('fusio')
        self._window = QWidget()
        self.mbox = QTextEdit()
        self._setUI()
        self._newLog.connect(self.mbox.insertPlainText)

    def addLog(self, text):
        self._newLog.emit(text)

    def _setUI(self):
        self.mbox.setReadOnly(True)
        self._commandComboBox = QComboBox()
        for command in self._controller.COMMANDS:
            self._commandComboBox.addItem(command.name, command)

        self._commandEdit = QLineEdit()
        self._commandEdit.returnPressed.connect(self._onSendCommand)
        line_btn = QPushButton("Send", self._window)
        line_btn.setShortcut("Enter")
        line_btn.clicked.connect(self._onSendCommand)

        prompt = QHBoxLayout()

        prompt.addWidget(self._commandComboBox)
        prompt.addWidget(self._commandEdit)
        prompt.addWidget(line_btn)

        leftBox = QVBoxLayout()
        leftBox.addWidget(self.mbox)
        leftBox.addLayout(prompt)

        refresh_btn = QPushButton('Refresh Ports', self._window)
        refresh_btn.clicked.connect(self._refreshPortList)

        Label_1 = QLabel("Controller")
        self._pointingSystemPortComboBox = QComboBox(self._window)
        controller_btn = QPushButton("Connect", self._window)
        controller_btn.clicked.connect(lambda: self._catchSerialError(
            self._controller.setPointingSystemPort, self._pointingSystemPortComboBox.currentText()))

        Label_2 = QLabel("RTK - Balloon A")
        self._rtkAPortComboBox = QComboBox(self._window)
        RTK_A_btn = QPushButton("Connect", self._window)
        RTK_A_btn.clicked.connect(lambda: self._catchSerialError(
            self._controller.setRtkAPort, self._rtkAPortComboBox.currentText()))

        Label_3 = QLabel("RTK - Balloon B")
        self._rtkBPortComboBox = QComboBox(self._window)
        RTK_B_btn = QPushButton("Connect", self._window)
        RTK_B_btn.clicked.connect(lambda: self._catchSerialError(
            self._controller.setRtkBPort, self._rtkBPortComboBox.currentText()))
        self._refreshPortList()

        Label_4 = QLabel('Target')
        combo_balloon = QComboBox(self._window)
        combo_balloon.addItem("Balloon A")
        combo_balloon.addItem("Balloon B")
        balloon_btn = QPushButton("Select Target", self._window)
        balloon_btn.clicked.connect(lambda: self._catchSerialError(
            self._controller.setPointingTarget, combo_balloon.currentIndex()))

        rightBox = QVBoxLayout()
        rightBox.addWidget(refresh_btn)

        rightBox.addWidget(Label_1)
        rightBox.addWidget(self._pointingSystemPortComboBox)
        rightBox.addWidget(controller_btn)

        rightBox.addWidget(Label_2)
        rightBox.addWidget(self._rtkAPortComboBox)
        rightBox.addWidget(RTK_A_btn)

        rightBox.addWidget(Label_3)
        rightBox.addWidget(self._rtkBPortComboBox)
        rightBox.addWidget(RTK_B_btn)

        rightBox.addWidget(Label_4)
        rightBox.addWidget(combo_balloon)
        rightBox.addWidget(balloon_btn)

        w = QHBoxLayout()

        w.addLayout(leftBox)
        w.addLayout(rightBox)

        self._window.setLayout(w)

        self._window.setGeometry(300, 300, 1000, 500)
        self._window.setWindowTitle('Laser Pointing Controller')
        self._window.show()

    def _onSendCommand(self):
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
                index = ports.index(currentValue)
                if index != -1:
                    comboBox.setCurrentIndex(index)

    def _catchSerialError(self, operation, *args, **kwargs):
        """
        Execute an operation and catch any serial exceptions.

        :param operation: The operation to execute.
        :param args: Arguments for the operation.
        :param kwargs: Keywards arguments for the operation.
        """
        try:
            operation(*args, **kwargs)
        except SerialException as error:
            QErrorMessage(self._window).showMessage(f'Operation failed:\n\n{error}')


if __name__ == '__main__':
    class MockController:
        @staticmethod
        def sendCommand(command, arguments):
            print(f'Sending {command.name} (arguments: {arguments})')

        @staticmethod
        def setPointingSystemPort(port):
            print("Controller : connection to the port " + port + "...")

        @staticmethod
        def setRtkAPort(port):
            print("RTK A : connection to the port " + port + "...")

        @staticmethod
        def setRtkBPort(port):
            print("RTK B : connection to the port " + port + "...")

        @staticmethod
        def setPointingTarget(target):
            print("Set the target to " + target)


    sys.exit(ControllerUi(MockController()).exec())
