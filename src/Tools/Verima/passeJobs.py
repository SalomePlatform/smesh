#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from Base.dataBase import Base
from Base.versions import Chercheversion


if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-a',dest='all',action="store_true", default=False,help='passe l ensemble des Tests')
      p.add_option('-s',dest='salomePath',help='chemin du runAppli',default="Appli")
      p.add_option('-v',dest='version',help='id de la version')
      p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
      p.add_option('-f',dest='force',default=True,help='force la passage des jobs meme si l execution a deja eu lieu sur cette machine pour cette version de salome')
      options, args = p.parse_args()
      if len(args) == 0  and options.all== False: 
	 print "Enter -a ou un numero de job"
         print 2
         exit()
      if options.salomePath==None : 
         print  "chemin du runAppli obligatoire"
         exit()
      if options.version==None : 
         options.version=Chercheversion(options.salomePath)
      maBase=Base(options.database)
      maBase.initialise()
      maBase.passeJobs(options.all,options.salomePath,options.version,options.force,args)
      maBase.close()

