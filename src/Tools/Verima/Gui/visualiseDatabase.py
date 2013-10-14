#!/usr/bin/env python

import sys,os
rep=os.path.dirname(os.path.abspath(__file__))
installDir=os.path.join(rep,'..')
sys.path.insert(0,installDir)

from PyQt4 import QtGui,QtCore
from Gui.maFenetreChoix import MaFenetreChoix
from Base.dataBase import Base



if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
      options, args = p.parse_args()

      app = QtGui.QApplication(sys.argv)
      maBase=Base(options.database)
      maBase.initialise()
      window = MaFenetreChoix(maBase)
      window.show()
      sys.exit(app.exec_())

