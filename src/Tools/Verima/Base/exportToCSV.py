#!/usr/bin/env python3

import sys,os

pathRacine=os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..'))
if os.path.dirname(pathRacine) not in sys.path :
   sys.path.insert(0,pathRacine)

from .dataBase import Base

if __name__ == "__main__":
      from argparse import ArgumentParser
      p=ArgumentParser()
      p.add_argument('-p',dest='partiel',action="store_true", default=False,help='export de machine, groupe, ratio Maille et Perf uniquement')
      p.add_argument('-d',dest='database',default="../myMesh.db",help='nom de la database')
      args = p.parse_args()

      maBase=Base(args.database)
      maBase.initialise()
      maBase.exportToCSV(args.partiel)
      maBase.close()

