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

import sys, os
rep=os.path.dirname(os.path.abspath(__file__))
installDir=os.path.join(rep,'..')
sys.path.insert(0,installDir)

from qtsalome import *
from Base.dataBase import Base

def completeDatabase(fichier,table,enregistrement):
      maBase=Base(fichier)
      maBase.initialise()
      nomTable="ma"+str(table)
      matable=getattr(maBase,nomTable)
      model= QSqlTableModel()
      model.setTable(matable.nom)
      nbCols=model.columnCount() -1
      if table == "TableGroupesRef" : nbCols==nbCols+1
      if len(enregistrement) != nbCols  :
         print("mauvais nb de valeurs")
         print("Attention, ne pas renter d'Id")
      if table == "TableGroupesRef" : matable.insereLigne(enregistrement)
      else : matable.insereLigneAutoId(enregistrement)
      maBase.close()



if __name__ == "__main__":
     from argparse import ArgumentParser
     p=ArgumentParser()
     p.add_argument('-d',dest='database',default="myMesh.db",help='nom de la database')
     p.add_argument('table',help='nom de la table a completer')
     p.add_argument('enregistrement', nargs="+")
     args = p.parse_args()
     if args.table is None : 
         print("table obligatoire")
         exit()
     good_tables = ("TableMaillages","TableMailleurs","TableGroupesRef","TableVersions")
     if args.table not in good_tables:
         print("la table doit etre : %s" % ' ou '.join(good_tables))
         exit()
     completeDatabase(args.database,args.table,args.enregistrement)

