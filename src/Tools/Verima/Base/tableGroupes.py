from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableGroupes (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"Groupes")
          self.setField(('Groupe','Maillage','Version','Entite','NbEntite'))
          self.setTypeField(('str','int','int','str','int'),('nomGroupe','idMaillage','idVersion','Entite'))


      def createSqlTable(self):
          query=QSqlQuery()


          texteQuery ="create table Groupes( nomGroupe varchar(40),idMaillage int, idVersion int,"
          texteQuery+="Entite var(40), NbEntite int,"
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="primary key (nomGroupe,idMaillage,idVersion,Entite));"

          print "Creation de TableGroupes : ", query.exec_(texteQuery)


      def getVal(self,nomGroupe,idMaillage,idVersion,typeMaille):
          val=0                          # Valeur si l enregistrement n existe pas
          query=QSqlQuery()
          texteQuery ='select NbEntite from Groupes  where nomGroupe ="' + nomGroupe +'"'
          texteQuery +=' and idMaillage=' + str(idMaillage)
          texteQuery +=' and idVersion = ' + str(idVersion)
          texteQuery +=' and Entite ="'  + str(typeMaille)   + '";' 
          query.exec_(texteQuery)
          while (query.next()) :
              val=query.value(0).toInt()[0]
          while (query.next()) :
              print "plusieurs enregistrements dans groupe pour ", nomGroupe," ",str(idMaillage)," ",str(idVersion),"\n"
          return val


      def getAllEntity(self):
            query=QSqlQuery()
            texteQuery ="select distinct Entite from Groupes;"
            query.exec_(texteQuery)
            maListe=[]
            while (query.next()) :
                maListe.append(str(query.value(0).toString()))
            return maListe
