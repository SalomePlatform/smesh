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

import os
from dataBase import Base


if __name__ == "__main__":
    from argparse import ArgumentParser
    p=ArgumentParser()
    p.add_argument('-p',dest='partiel',action="store_true", default=False,help='import de machine, groupe, ratio Maille et Perf uniquement')
    p.add_argument('-f',dest='force',action="store_true", default=False,help='ecrasement des valeurs dans la base par les valeurs dans les fichiers en cas de meme clef primaire')
    p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
    p.add_argument('folder', help='Repertoire ou sont ranges les fichiers a charger')
    args = p.parse_args()

    if not(os.path.isdir(args.folder)):
        print(args.folder , " n existe pas")
        exit()

    maBase=Base(args.database)
    maBase.create()
    maBase.initialise()
    maBase.importFromCSV(args.folder,args.partiel,args.force)
    maBase.close()
