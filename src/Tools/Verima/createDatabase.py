#!/usr/bin/env python

import sys
from Base.dataBase import Base

def creeDatabase(fichier):
      maBase=Base(fichier)
      maBase.create()
      maBase.initialise()
      maBase.remplit()
      maBase.close()



if __name__ == "__main__":
     from optparse import OptionParser
     p=OptionParser()
     p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
     options, args = p.parse_args()
     creeDatabase(options.database )

