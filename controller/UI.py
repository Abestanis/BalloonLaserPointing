import sys
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *


def sendCommand(commandText):
	printlog(commandText+'\n')
	pass

def getPorts():
	return ['Port1', 'Port2']

def setPointingSystemPort(port):
	printlog("Controller : connection to the port "+port+"...\n")
	pass

def setRtkAport(port):
	printlog("RTK A : connection to the port "+port+"...\n")
	pass

def setRtkBport(port):
	printlog("RTK B : connection to the port "+port+"...\n")
	pass

def setPointingTarget(target):
	printlog("Set the target to "+target+'\n')
	pass

def printlog(test):

	fenetre.newlog(test)

class Principale(QWidget):
	
	

	def __init__(self):
		super().__init__()
		self.mbox = QTextEdit()
		self.setUI()
		
	
	
	def newlog(self, text):
		self.mbox.insertPlainText(text)
		
	def setUI(self):
		self.mbox.setReadOnly(True)
	


		lineedit=QLineEdit()
		line_btn=QPushButton("Send",self)
		line_btn.setShortcut("Enter")
		line_btn.clicked.connect(lambda:sendCommand(lineedit.text()))
		line_btn.clicked.connect(lambda:lineedit.clear())

		prompt=QHBoxLayout()

		prompt.addWidget(lineedit)
		prompt.addWidget(line_btn)

		leftbox=QVBoxLayout()
		leftbox.addWidget(self.mbox)
		leftbox.addLayout(prompt)


		Ports=getPorts()
		
		Label_1=QLabel("Controller")
		combo_controller=QComboBox(self)
		combo_controller.addItem(Ports[0])
		combo_controller.addItem(Ports[1])
		controller_btn=QPushButton("Connect",self)
		controller_btn.clicked.connect(lambda:setPointingSystemPort(combo_controller.currentText()))
	
		Label_2=QLabel("RTK - Ballon A")
		combo_RTK_A=QComboBox(self)
		combo_RTK_A.addItem(Ports[0])
		combo_RTK_A.addItem(Ports[1])
		RTK_A_btn=QPushButton("Connect",self)
		RTK_A_btn.clicked.connect(lambda:setRtkAport(combo_RTK_A.currentText()))

		Label_3=QLabel("RTK - Ballon B")
		combo_RTK_B=QComboBox(self)
		combo_RTK_B.addItem(Ports[0])
		combo_RTK_B.addItem(Ports[1])
		RTK_B_btn=QPushButton("Connect",self)
		RTK_B_btn.clicked.connect(lambda:setRtkBport(combo_RTK_B.currentText()))

		Label_4=QLabel('Target')
		combo_balloon=QComboBox(self)
		combo_balloon.addItem("Balloon A")
		combo_balloon.addItem("Balloon B")
		balloon_btn=QPushButton("Select Target",self)
		balloon_btn.clicked.connect(lambda:setPointingTarget(combo_balloon.currentText()))

		
		

		rightbox=QVBoxLayout()

		rightbox.addWidget(Label_1)
		rightbox.addWidget(combo_controller)
		rightbox.addWidget(controller_btn)

		rightbox.addWidget(Label_2)
		rightbox.addWidget(combo_RTK_A)
		rightbox.addWidget(RTK_A_btn)

		rightbox.addWidget(Label_3)
		rightbox.addWidget(combo_RTK_B)
		rightbox.addWidget(RTK_B_btn)

		rightbox.addWidget(Label_4)
		rightbox.addWidget(combo_balloon)
		rightbox.addWidget(balloon_btn)


	
		w=QHBoxLayout()

		

		w.addLayout(leftbox)
		w.addLayout(rightbox)

		self.setLayout(w)

		
		
		
		
		self.setGeometry(300,300,1000,500)
		self.setWindowTitle('Fenêtre principale')
		self.show()
monApp=QApplication(sys.argv)
monApp.setStyle('fusio')
fenetre=Principale()
sys.exit(monApp.exec_())



