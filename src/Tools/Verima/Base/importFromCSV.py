#!/usr/bin/env python3

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
