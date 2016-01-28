from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableGroupeRatios (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"GroupesRatios")
          self.setField(('Maillage','Version','Groupe','RatioMax','RatioMin','Quartile1','Mediane','Quartile3','Moyenne'))
          self.setTypeField(('int','int','str','float','float','float','float','float','float'),('idMaillage','idVersion','Groupe'))

      def createSqlTable(self):
          query=QSqlQuery()

          texteQuery ="create table GroupesRatios(idMaillage int, idVersion int,  Groupe varchar(40),"
          texteQuery+="RatioMax float, RatioMin float, "
          texteQuery+="Q1 float,"
          texteQuery+="Mediane float,"
          texteQuery+="Q3 float,"
          texteQuery+="Moyenne float,"
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="foreign key (Groupe) references GroupesRef(nomGroupe)," 
          texteQuery+="primary key (idMaillage,idVersion,Groupe));"
          print "Creation de TableGroupeRatios : " , query.exec_(texteQuery)

      def getVal(self,idMaillage, idVersion, Groupe, Entite):
          query=QSqlQuery()
          texteQuery ='select '+ str(Entite) + ' from GroupesRatios where idMaillage='+str(idMaillage)
          texteQuery+=' and  idVersion='+str(idVersion)
          texteQuery+=" and Groupe='" + str(Groupe) +"';"
          query.exec_(texteQuery)
          nb=0
          val=0                          # Valeur si l enregistrement n existe pas
          while (query.next()) :
              val=query.value(0).toFloat()[0]
              nb=nb+1
          if nb > 1 : print "Double valeur de Reference dans la table des mailles"
          return val

