from qtsalome import *
from nomBase_ui import Ui_LEDataBaseName


class DataBaseName(Ui_LEDataBaseName,QDialog):

     def __init__(self, parent):
         QDialog.__init__(self,parent)
         self.setupUi(self)
         self.setModal(True)
         self.parent=parent
         self.LEBaseName.returnPressed.connect(self.LEDBreturnPressed)


     def LEDBreturnPressed(self):
         self.parent.nomBase=self.LEBaseName.text()
         self.close()
