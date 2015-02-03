#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from dataBase import Base



if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-p',dest='partiel',action="store_true", default=False,help='import de machine, groupe, ratio Maille et Perf uniquement')
      p.add_option('-f',dest='force',action="store_true", default=False,help='ecrasement des valeurs dans la base par les valeurs dans les fichiers en cas de meme clef primaire')
      p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
      options, args = p.parse_args()
      if len(args) != 1 : 
	 print "entrer SVP le nom de la directory ou sont rangees les fichiers a charger"
         exit()
      folder=args[0]

      if not(os.path.isdir(folder)):
         print folder , " n existe pas"
         exit()

      maBase=Base(options.database)
      maBase.create()
      maBase.initialise()
      maBase.importFromCSV(folder,options.partiel,options.force)
      maBase.close()

