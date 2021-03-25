#!/usr/bin/env python3
# Copyright (C) 2013-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

from qtsalome import *
from .Gui.myMain_ui import Ui_Gestion
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
         self.BExp.clicked.connect(self.BExpPressed)
         self.BImp.clicked.connect(self.BImpPressed)
         self.BCree.clicked.connect(self.BCreePressed)
         self.BVue.clicked.connect(BVuePressed)
         self.BStat.clicked.connect(BStatPressed)


     def BExpPressed(self):
         pass

     def BImpPressed(self):
         pass

     def BCreePressed(self):
         from .Gui.monNomBase import DataBaseName
         maW=DataBaseName(self)
         maW.exec_()
         from .createDatabase import creeDatabase
         creeDatabase(self.nomBase)

     def BVuePressed(self):
         if self.nomBase == "" :
            from .Gui.monNomBase import DataBaseName
            maW=DataBaseName(self)
            maW.exec_()
         from .Gui.maFenetreChoix import MaFenetreChoix
         from .Base.dataBase import Base
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


