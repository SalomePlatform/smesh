#!/usr/bin/env python

from PyQt4  import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from Gui.myMain_ui import Ui_Gestion
import sys


class MonAppli(Ui_Gestion,QWidget):
     def __init__(self) :
         QWidget.__init__(self)
         Ui_Gestion.__init__(self)
         self.setupUi(self)
         self.connectSignaux()
         self.nomBase=""
         self.salomePath=""
         self.version=""

     def connectSignaux(self) :
         self.connect(self.BExp,SIGNAL("clicked()"),self.BExpPressed)
         self.connect(self.BImp,SIGNAL("clicked()"),self.BImpPressed)
         self.connect(self.BCree,SIGNAL("clicked()"),self.BCreePressed)
         self.connect(self.BVue,SIGNAL("clicked()"),self.BVuePressed)
         self.connect(self.BStat,SIGNAL("clicked()"),self.BStatPressed)


     def BExpPressed(self):
         pass

     def BImpPressed(self):
         pass

     def BCreePressed(self):
         from Gui.monNomBase import DataBaseName
         maW=DataBaseName(self)
         maW.exec_()
         from createDatabase import creeDatabase
         creeDatabase(self.nomBase)

     def BVuePressed(self):
         if self.nomBase == "" :
            from Gui.monNomBase import DataBaseName
            maW=DataBaseName(self)
            maW.exec_()
         from Gui.maFenetreChoix import MaFenetreChoix
         from Base.dataBase import Base
         maBase=Base(self.nomBase)
         maBase.initialise()
         window = MaFenetreChoix(maBase)
         #window.show()
         window.exec_()

         

     def BStatPressed(self):
         pass


if __name__ == "__main__":
      app = QApplication(sys.argv)
      window = MonAppli()
      window.show()
      sys.exit(app.exec_())


