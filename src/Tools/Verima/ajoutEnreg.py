#!/usr/bin/env python3

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

