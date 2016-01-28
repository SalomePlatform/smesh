from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableGroupesRef (TableDeBase):
	def __init__(self):
            TableDeBase.__init__(self,"GroupesRef")
            self.setField(("nomGroupe","idMaillage"))
            self.setTypeField(('str','int'),('nomGroupe'))

        def createSqlTable(self):
            query=QSqlQuery()
            texteQuery ="create table GroupesRef(nomGroupe varchar(40), idMaillage int,"
            texteQuery+="foreign key (idMaillage) references Maillages(idMaillage),"
            texteQuery+="primary key (nomGroupe,idMaillage));"
            print "Creation de TableGroupesRef : " , query.exec_(texteQuery)

        def getVals(self,idMaillage):
            query=QSqlQuery()
            texteQuery ='select NomGroupe from GroupesRef where idMaillage='+str(idMaillage) +";"
            listeGroupes=[]
            query.exec_(texteQuery)
            while (query.next()) :
               listeGroupes.append(str(query.value(0).toString()))
            return listeGroupes


#        def remplit(self):
            
# Groupe pour le script du tunnel (fiche 7566)
#            self.insereLigne(('FRONT_07',1))
#            self.insereLigne(('FOND_07',1))
#            self.insereLigne(('PAROI_07',1))
#            self.insereLigne(('ROCHE_07',1))
#            self.insereLigne(('FOND_16',1))
#            self.insereLigne(('PAROI_16',1))
#            self.insereLigne(('ROCHE_16',1))
# Groupe pour le script de l'attache d'une ailette (fiche 7957)
#            self.insereLigne(('RAIN_JONC',2))
#            self.insereLigne(('EXT_CRAN',2))
#            self.insereLigne(('ENV_E',2))
#            self.insereLigne(('ENV_I',2))
