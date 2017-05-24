#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from Base.dataBase import Base
from Base.versions import  Chercheversion


if __name__ == "__main__":
      from argparse import ArgumentParser
      p=ArgumentParser()
      p.add_argument('-s',dest='salomePath',help='chemin du lanceur salome de la version a comparer',default="Appli")
      p.add_argument('-v',dest='version',help='id de la version a comparer',default=None)
      p.add_argument('-r',dest='versionRef',help='liste des id des versions de reference',default=None)
      p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
      p.add_argument('-f',dest='fileHtml',default="/tmp/toto.htm",help='nom du fichier produit')
      args = p.parse_args()

      if args.version is None : 
         args.version=Chercheversion(args.salomePath)
      exit

      maBase=Base(args.database)
      maBase.initialise()
      maBase.compare(args.version,args.versionRef,args.fileHtml)
      maBase.close()

