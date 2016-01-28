#!/usr/bin/env python

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
         print "mauvais nb de valeurs"
         print "Attention, ne pas renter d'Id"
      if table == "TableGroupesRef" : matable.insereLigne(enregistrement)
      else : matable.insereLigneAutoId(enregistrement)
      maBase.close()



if __name__ == "__main__":
     from optparse import OptionParser
     p=OptionParser()
     p.add_option('-d',dest='database',default="myMesh.db",help='nom de la database')
     p.add_option('-t',dest='table',help='nom de la table a completer')
     options, args = p.parse_args()
     if  options.table==None : 
         print  "table obligatoire"
         exit()
     if  options.table not in ("TableMaillages","TableMailleurs","TableGroupesRef","TableVersions") : 
         print  "la table doit etre : TableMaillages ou TableMailleurs ou TableGroupesRef ou TableVersions" 
         exit()
     enregistrement=tuple(args)
     completeDatabase(options.database,options.table,enregistrement)

