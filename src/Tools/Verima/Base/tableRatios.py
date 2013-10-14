from PyQt4 import QtSql, QtCore
from tableDeBase import TableDeBase

class TableRatios (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"Ratios")
          self.setField(('Maillage','Version','Machine','RatioMax','RatioMin','Quartile1','Mediane','Quartile3','Moyenne'))
          self.setTypeField(('int','int','str','float','float','float','float','float','float'),('idMaillage','idVersion','Machine'))

      def createSqlTable(self):
          query=QtSql.QSqlQuery()

          texteQuery ="create table Ratios(idMaillage int, idVersion int, Machine varchar(10),"
          texteQuery+="RatioMax float, RatioMin float, "
          texteQuery+="Q1 float,"
          texteQuery+="Mediane float,"
          texteQuery+="Q3 float,"
          texteQuery+="Moyenne float,"
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="foreign key (Machine) references Machines(nomMachine)," 
          texteQuery+="primary key (idMaillage,idVersion,Machine));"

          print "Creation de TableRatios : " , query.exec_(texteQuery)

      def getVal(self,idMaillage, idVersion, Machine, Entite):
          query=QtSql.QSqlQuery()
          texteQuery ='select '+ str(Entite) + ' from Ratios where idMaillage='+str(idMaillage)
          texteQuery+=' and  idVersion='+str(idVersion)
          texteQuery+=" and Machine='" + str(Machine) +"';"
          query.exec_(texteQuery)
          nb=0
          val=0                          # Valeur si l enregistrement n existe pas
          while (query.next()) :
              val=query.value(0).toInt()[0]
              nb=nb+1
          if nb > 1 : print "Double valeur de Reference dans la table des mailles"
          return val

