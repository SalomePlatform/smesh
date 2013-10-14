from PyQt4 import QtSql, QtCore
from tableDeBase import TableDeBase

class TableGroupesRef (TableDeBase):
	def __init__(self):
            TableDeBase.__init__(self,"GroupesRef")
            self.setField(("nomGroupe","idMaillage"))
            self.setTypeField(('str','int'),('nomGroupe'))

        def createSqlTable(self):
            query=QtSql.QSqlQuery()
            texteQuery ="create table GroupesRef(nomGroupe varchar(40), idMaillage int,"
            texteQuery+="foreign key (idMaillage) references Maillages(idMaillage),"
            texteQuery+="primary key (nomGroupe,idMaillage));"
            print "Creation de TableGroupesRef : " , query.exec_(texteQuery)

        def getVals(self,idMaillage):
            query=QtSql.QSqlQuery()
            texteQuery ='select NomGroupe from GroupesRef where idMaillage='+str(idMaillage) +";"
            listeGroupes=[]
            query.exec_(texteQuery)
            while (query.next()) :
               listeGroupes.append(str(query.value(0).toString()))
            return listeGroupes


        def remplit(self):
            self.insereLigne(('FRONT_07',2))
            self.insereLigne(('FOND_07',2))
            self.insereLigne(('PAROI_07',2))
            self.insereLigne(('ROCHE_07',2))
            self.insereLigne(('FOND_16',2))
            self.insereLigne(('PAROI_16',2))
            self.insereLigne(('ROCHE_16',2))
