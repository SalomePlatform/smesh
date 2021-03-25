#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2013-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

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

