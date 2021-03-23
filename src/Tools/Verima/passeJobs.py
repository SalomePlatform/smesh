#!/usr/bin/env python3
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
from Base.versions import Chercheversion


if __name__ == "__main__":
    from argparse import ArgumentParser
    p=ArgumentParser()
    p.add_argument('-a',dest='all_tests',action="store_true", default=False,help='passe l ensemble des Tests')
    p.add_argument('-s',dest='salomePath',help='chemin du lanceur salome',default="Appli")
    p.add_argument('-v',dest='version',help='id de la version')
    p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
    p.add_argument('-f',dest='force',default=True,help='force la passage des jobs meme si l execution a deja eu lieu sur cette machine pour cette version de salome')
    p.add_argument('job', nargs='*')
    args = p.parse_args()
    if len(args.job) == 0  and args.all_tests == False:
        print("Enter -a ou un numero de job")
        print(2)
        exit()
    if args.salomePath is None :
        print("chemin du lanceur salome obligatoire")
        exit()
    if args.version is None :
        args.version=Chercheversion(args.salomePath)
    maBase=Base(args.database)
    maBase.initialise()
    maBase.passeJobs(args.all_tests,args.salomePath,args.version,args.force,args.job)
    maBase.close()
