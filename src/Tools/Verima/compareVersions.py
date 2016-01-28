#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from Base.dataBase import Base
from Base.versions import  Chercheversion


if __name__ == "__main__":
      from optparse import OptionParser
      p=OptionParser()
      p.add_option('-s',dest='salomePath',help='chemin du runAppli de la version a comparer',default="Appli")
      p.add_option('-v',dest='version',help='id de la version a comparer',default=None)
      p.add_option('-r',dest='versionRef',help='liste des id des versions de reference',default=None)
      p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
      p.add_option('-f',dest='fileHtml',default="/tmp/toto.htm",help='nom du fichier produit')
      options, args = p.parse_args()

      if options.version==None : 
         options.version=Chercheversion(options.salomePath)
      exit

      maBase=Base(options.database)
      maBase.initialise()
      maBase.compare(options.version,options.versionRef,options.fileHtml)
      maBase.close()

