import sys

from serial.tools.list_ports import comports
from PyQt5.QtWidgets import QWidget, QTextEdit, QApplication, QLineEdit, QPushButton, QHBoxLayout, \
    QVBoxLayout, QLabel, QComboBox


def sendCommand(commandText):
    print(commandText)


def setPointingSystemPort(port):
    print("Controller : connection to the port " + port + "...")


def setRtkAPort(port):
    print("RTK A : connection to the port " + port + "...")


def setRtkBPort(port):
    print("RTK B : connection to the port " + port + "...")


def setPointingTarget(target):
    print("Set the target to " + target)


class ControllerUi(QApplication):
    def __init__(self):
        super().__init__(sys.argv)
        self.setStyle('fusio')
        self._window = QWidget()
        self.mbox = QTextEdit()
        self._setUI()

    def addLog(self, text):
        self.mbox.insertPlainText(text)

    def _setUI(self):
        self.mbox.setReadOnly(True)
        self._commandEdit = QLineEdit()
        line_btn = QPushButton("Send", self._window)
        line_btn.setShortcut("Enter")
        line_btn.clicked.connect(self._onSendCommand)

        prompt = QHBoxLayout()

        prompt.addWidget(self._commandEdit)
        prompt.addWidget(line_btn)

        leftBox = QVBoxLayout()
        leftBox.addWidget(self.mbox)
        leftBox.addLayout(prompt)

        ports = list(comports())

        Label_1 = QLabel("Controller")
        combo_controller = QComboBox(self._window)
        combo_controller.addItems(ports)
        controller_btn = QPushButton("Connect", self._window)
        controller_btn.clicked.connect(
            lambda: setPointingSystemPort(combo_controller.currentText()))

        Label_2 = QLabel("RTK - Balloon A")
        combo_RTK_A = QComboBox(self._window)
        combo_RTK_A.addItems(ports)
        RTK_A_btn = QPushButton("Connect", self._window)
        RTK_A_btn.clicked.connect(lambda: setRtkAPort(combo_RTK_A.currentText()))

        Label_3 = QLabel("RTK - Balloon B")
        combo_RTK_B = QComboBox(self._window)
        combo_RTK_B.addItems(ports)
        RTK_B_btn = QPushButton("Connect", self._window)
        RTK_B_btn.clicked.connect(lambda: setRtkBPort(combo_RTK_B.currentText()))

        Label_4 = QLabel('Target')
        combo_balloon = QComboBox(self._window)
        combo_balloon.addItem("Balloon A")
        combo_balloon.addItem("Balloon B")
        balloon_btn = QPushButton("Select Target", self._window)
        balloon_btn.clicked.connect(lambda: setPointingTarget(combo_balloon.currentText()))

        rightBox = QVBoxLayout()

        rightBox.addWidget(Label_1)
        rightBox.addWidget(combo_controller)
        rightBox.addWidget(controller_btn)

        rightBox.addWidget(Label_2)
        rightBox.addWidget(combo_RTK_A)
        rightBox.addWidget(RTK_A_btn)

        rightBox.addWidget(Label_3)
        rightBox.addWidget(combo_RTK_B)
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
        sendCommand(self._commandEdit.text())
        self._commandEdit.clear()


if __name__ == '__main__':
    sys.exit(ControllerUi().exec())
