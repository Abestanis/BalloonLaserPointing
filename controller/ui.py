import sys

from PyQt5.QtWidgets import QWidget, QTextEdit, QApplication, QLineEdit, QPushButton, QHBoxLayout, \
    QVBoxLayout, QLabel, QComboBox


def sendCommand(commandText):
    print(commandText)


def getPorts():
    return ['Port1', 'Port2']


def setPointingSystemPort(port):
    print("Controller : connection to the port " + port + "...")


def setRtkAPort(port):
    print("RTK A : connection to the port " + port + "...")


def setRtkBPort(port):
    print("RTK B : connection to the port " + port + "...")


def setPointingTarget(target):
    print("Set the target to " + target)


class ControllerUi(QWidget):
    def __init__(self):
        super().__init__()
        self.mbox = QTextEdit()
        self._setUI()

    def addLog(self, text):
        self.mbox.insertPlainText(text)

    def _setUI(self):
        self.mbox.setReadOnly(True)
        self._commandEdit = QLineEdit()
        line_btn = QPushButton("Send", self)
        line_btn.setShortcut("Enter")
        line_btn.clicked.connect(self._onSendCommand)

        prompt = QHBoxLayout()

        prompt.addWidget(self._commandEdit)
        prompt.addWidget(line_btn)

        leftBox = QVBoxLayout()
        leftBox.addWidget(self.mbox)
        leftBox.addLayout(prompt)

        Ports = getPorts()

        Label_1 = QLabel("Controller")
        combo_controller = QComboBox(self)
        combo_controller.addItem(Ports[0])
        combo_controller.addItem(Ports[1])
        controller_btn = QPushButton("Connect", self)
        controller_btn.clicked.connect(
            lambda: setPointingSystemPort(combo_controller.currentText()))

        Label_2 = QLabel("RTK - Balloon A")
        combo_RTK_A = QComboBox(self)
        combo_RTK_A.addItem(Ports[0])
        combo_RTK_A.addItem(Ports[1])
        RTK_A_btn = QPushButton("Connect", self)
        RTK_A_btn.clicked.connect(lambda: setRtkAPort(combo_RTK_A.currentText()))

        Label_3 = QLabel("RTK - Balloon B")
        combo_RTK_B = QComboBox(self)
        combo_RTK_B.addItem(Ports[0])
        combo_RTK_B.addItem(Ports[1])
        RTK_B_btn = QPushButton("Connect", self)
        RTK_B_btn.clicked.connect(lambda: setRtkBPort(combo_RTK_B.currentText()))

        Label_4 = QLabel('Target')
        combo_balloon = QComboBox(self)
        combo_balloon.addItem("Balloon A")
        combo_balloon.addItem("Balloon B")
        balloon_btn = QPushButton("Select Target", self)
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

        self.setLayout(w)

        self.setGeometry(300, 300, 1000, 500)
        self.setWindowTitle('Fenêtre principale')
        self.show()

    def _onSendCommand(self):
        sendCommand(self._commandEdit.text())
        self._commandEdit.clear()


def showControllerWindow():
    app = QApplication(sys.argv)
    app.setStyle('fusio')
    window = ControllerUi()
    return app.exec()


if __name__ == '__main__':
    sys.exit(showControllerWindow())
