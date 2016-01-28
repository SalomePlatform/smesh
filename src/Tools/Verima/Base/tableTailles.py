from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableTailles (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"Tailles")
          self.setField(('Maillage','Version','TailleMax','TailleMin','Quartile1','Mediane','Quartile3','Moyenne'))
          self.setTypeField(('int','int','str','float','float','float','float','float','float'),('idMaillage','idVersion'))

      def createSqlTable(self):
          query=QSqlQuery()

          texteQuery ="create table Tailles(idMaillage int, idVersion int, "
          texteQuery+="TailleMax float, TailleMin float, "
          texteQuery+="Q1 float, "
          texteQuery+="Mediane float, "
          texteQuery+="Q3 float, "
          texteQuery+="Moyenne float, "
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="primary key (idMaillage,idVersion));"

          print "Creation de TableTailles : " , query.exec_(texteQuery)

      def getVal(self,idMaillage, idVersion, Entite):
          query=QSqlQuery()
          texteQuery ='select '+ str(Entite) + ' from Tailles where idMaillage='+str(idMaillage)
          texteQuery+=' and  idVersion='+str(idVersion)
          query.exec_(texteQuery)
          nb=0
          val=0                          # Valeur si l enregistrement n existe pas
          while (query.next()) :
              val=query.value(0).toFloat()[0]
              nb=nb+1
          if nb > 1 : print "Double valeur de Reference dans la table des mailles"
          return val

