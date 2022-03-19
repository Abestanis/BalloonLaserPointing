import sys
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *


def sendCommand(commandText):
    printlog(commandText + '\n')
    pass


def getPorts():
    return ['Port1', 'Port2']


def setPointingSystemPort(port):
    printlog("Controller : connection to the port " + port + "...\n")
    pass


def setRtkAPort(port):
    printlog("RTK A : connection to the port " + port + "...\n")
    pass


def setRtkBPort(port):
    printlog("RTK B : connection to the port " + port + "...\n")
    pass


def setPointingTarget(target):
    printlog("Set the target to " + target + '\n')
    pass


def printlog(test):
    window.addLog(test)


class ControllerUi(QWidget):
    def __init__(self):
        super().__init__()
        self.mbox = QTextEdit()
        self.setUI()

    def addLog(self, text):
        self.mbox.insertPlainText(text)

    def setUI(self):
        self.mbox.setReadOnly(True)
        lineedit = QLineEdit()
        line_btn = QPushButton("Send", self)
        line_btn.setShortcut("Enter")
        line_btn.clicked.connect(lambda: sendCommand(lineedit.text()))
        line_btn.clicked.connect(lambda: lineedit.clear())

        prompt = QHBoxLayout()

        prompt.addWidget(lineedit)
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


app = QApplication(sys.argv)
app.setStyle('fusio')
window = ControllerUi()
sys.exit(app.exec())
