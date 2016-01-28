from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableMailles (TableDeBase):
      def __init__(self):
         TableDeBase.__init__(self,"Mailles")
         self.setField(('Maillage','Version','Entite','NbEntite'))
         self.setTypeField(('int','int','str','int'),('idMaillage','idVersion','Entite'))


      def createSqlTable(self):
          query=QSqlQuery()


          texteQuery ="create table Mailles(idMaillage int, idVersion int,  Entite var(40), NbEntite int, "
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="primary key (idMaillage,idVersion,Entite));"

          print "Creation de TableMailles : " , query.exec_(texteQuery)

 
      def getVal(self,idMaillage, idVersion,  Entite):
            query=QSqlQuery()
            texteQuery ='select NbEntite from Mailles where idMaillage='+str(idMaillage) 
            texteQuery+=' and  idVersion='+str(idVersion) 
            texteQuery+=" and Entite='" + str(Entite)  +"';"
            query.exec_(texteQuery)
            nb=0
            val=0                          # Valeur si l enregistrement n existe pas
            while (query.next()) :
               val=query.value(0).toInt()[0]
               nb=nb+1
            if nb > 1 : print "Double valeur de Reference dans la table des mailles"
            return val


      def getAllEntity(self):
            query=QSqlQuery()
            texteQuery ="select distinct Entite from Mailles;"
            query.exec_(texteQuery)
            maListe=[]
            while (query.next()) :
                maListe.append(str(query.value(0).toString()))
            return maListe
