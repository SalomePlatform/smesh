#!/usr/bin/env python

import sys, os
rep=os.path.dirname(os.path.abspath(__file__))
installDir=os.path.join(rep,'..')
sys.path.insert(0,installDir)

from PyQt4 import QtGui,QtCore,QtSql
from Base.dataBase import Base

def completeDatabase(fichier,version):
      maBase=Base(fichier)
      maBase.initialise()
      message= maBase.maTableVersions.remplaceVersion(version)
      print message 
      maBase.close()



if __name__ == "__main__":
     from optparse import OptionParser
     p=OptionParser()
     p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
     options, args = p.parse_args()
     if len(args) != 1 :
        print "Entrer la version qui doit devenir la version de reference"
     version=args[0]
     completeDatabase(options.database,version)

