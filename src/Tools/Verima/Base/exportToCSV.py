#!/usr/bin/env python

import sys,os

pathRacine=os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..'))
if os.path.dirname(pathRacine) not in sys.path :
   sys.path.insert(0,pathRacine)

from dataBase import Base

if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-p',dest='partiel',action="store_true", default=False,help='export de machine, groupe, ratio Maille et Perf uniquement')
      p.add_option('-d',dest='database',default="../myMesh.db",help='nom de la database')
      options, args = p.parse_args()

      maBase=Base(options.database)
      maBase.initialise()
      maBase.exportToCSV(options.partiel)
      maBase.close()

