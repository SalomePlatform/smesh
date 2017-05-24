#!/usr/bin/env python3

import sys,os
rep=os.path.dirname(os.path.abspath(__file__))
installDir=os.path.join(rep,'..')
sys.path.insert(0,installDir)

from qtsalome import QApplication
from Gui.maFenetreChoix import MaFenetreChoix
from Base.dataBase import Base



if __name__ == "__main__":
      from argparse import ArgumentParser
      p=ArgumentParser()
      p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
      args = p.parse_args()

      app = QApplication(sys.argv)
      maBase=Base(args.database)
      maBase.initialise()
      window = MaFenetreChoix(maBase)
      window.show()
      sys.exit(app.exec_())

