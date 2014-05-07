from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtSql import *
from nomBase_ui import Ui_LEDataBaseName


class DataBaseName(Ui_LEDataBaseName,QDialog):

     def __init__(self, parent):
         QDialog.__init__(self,parent)
         self.setupUi(self)
         self.setModal(True)
         self.parent=parent
         self.connect(self.LEBaseName, SIGNAL("returnPressed()"), self.LEDBreturnPressed)


     def LEDBreturnPressed(self):
         self.parent.nomBase=self.LEBaseName.text()
         self.close()
