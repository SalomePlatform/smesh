from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TablePerfs (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"Perfs")
          self.setField(('Maillage','Version','Machine','NbSecCpu','Memoire'))
          self.setTypeField(('int','int','str','int','int'),('idMaillage','idVersion','Machine'))

      def createSqlTable(self):
          query=QSqlQuery()
          texteQuery ="create table Perfs(idMaillage int, idVersion int, Machine varchar(10),  NbCpu int, Mem int, "
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="foreign key (Machine) references Machines(nomMachine)," 
          texteQuery+="primary key (idMaillage, idVersion, Machine));"

          print "Creation de TablePerfs : " , query.exec_(texteQuery)

      def getVal(self,idMaillage,idVersion,Machine):
          query=QSqlQuery()
          texteQuery ='select NbCpu from Perfs  where idMaillage=' + str(idMaillage)
          texteQuery +=' and idVersion = ' + str(idVersion)
          texteQuery +=" and Machine ='"  + Machine + "';" 
          query.exec_(texteQuery)
          cpu=None
          while (query.next()) :
              cpu=query.value(0).toInt()[0]
          while (query.next()) :
              print "plusieurs enregistrements dans perf pour ",str(idMaillage)," ",str(idVersion)," ",Machine
          if cpu==None :
              print "pas d enregistrement dans perf pour ",str(idMaillage)," ",str(idVersion)," ",Machine
          return cpu
