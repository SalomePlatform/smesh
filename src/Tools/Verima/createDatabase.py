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
     from argparse import ArgumentParser
     p=ArgumentParser()
     p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
     args = p.parse_args()
     creeDatabase(args.database )

