from PyQt4 import QtSql, QtCore
from tableDeBase import TableDeBase

class TableGroupes (TableDeBase):
      def __init__(self):
          TableDeBase.__init__(self,"Groupes")
          #self.setField(('Groupe','Maillage','Version','Machine', 'Node','0D','Edge','Quad_Edge','Triangle','Quad_Triangle','BiQuad_Triangle','Quadrangle','Quad_Quadrangle','BiQuad_Quadrangle','Polygon','Quad_Polygon','Tetra','Quad_Tetra','Pyramid','Quad_Pyramid','Hexa','Quad_Hexa','TriQuad_Hexa','Penta','Quad_Penta','Hexagonal_Prism','Polyhedra','Quad_Polyhedra',))
          #self.setTypeField(('int','str','int','int','str','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int'),('id','idMaillage','idVersion','Machine'))
          self.setField(('Groupe','Maillage','Version','Machine','Entite','NbEntite'))
          self.setTypeField(('str','int','int','str','str','int'),('nomGroupe','idMaillage','idVersion','Machine','Entite'))


      def createSqlTable(self):
          query=QtSql.QSqlQuery()

          #texteQuery ="create table Groupes( nomGroupe varchar(40),idMaillage int, idVersion int,"
          #texteQuery+="Machine varchar(10), Node int, "
          #texteQuery+="0D int, Edge int, Quad_Edge int, Triangle int, Quad_Triangle int, BiQuad_Triangle int,"
          #texteQuery+="Quadrangle int, Quad_Quadrangle int,  BiQuad_Quadrangle int,  Polygon int,"
          #texteQuery+="Quad_Polygon int, Tetra int, Quad_Tetra int, Pyramid int, Quad_Pyramid int,"
          #texteQuery+="Hexa int, Quad_Hexa int, TriQuad_Hexa int, Penta int, Quad_Penta int,"
          #texteQuery+="Hexagonal_Prism int, Polyhedra int, Quad_Polyhedra int, "

          texteQuery ="create table Groupes( nomGroupe varchar(40),idMaillage int, idVersion int,"
          texteQuery+="Machine varchar(10), Entite var(40), NbEntite int,"
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="foreign key (Machine) references Machines(nomMachine)," 
          texteQuery+="primary key (nomGroupe,idMaillage,idVersion,Machine,Entite));"

          print "Creation de TableGroupes : ", query.exec_(texteQuery)


      def getVal(self,nomGroupe,idMaillage,idVersion,Machine,typeMaille):
          val=0                          # Valeur si l enregistrement n existe pas
          query=QtSql.QSqlQuery()
          texteQuery ='select NbEntite from Groupes  where nomGroupe ="' + nomGroupe +'"'
          texteQuery +=' and idMaillage=' + str(idMaillage)
          texteQuery +=' and idVersion = ' + str(idVersion)
          texteQuery +=' and Machine ="'  + Machine + '"' 
          texteQuery +=' and Entite ="'  + str(typeMaille)   + '";' 
          query.exec_(texteQuery)
          while (query.next()) :
              val=query.value(0).toInt()[0]
          while (query.next()) :
              print "plusieurs enregistrements dans groupe pour ", nomGroupe," ",str(idMaillage)," ",str(idVersion)," ",Machine
          return val


      def getAllEntity(self):
            query=QtSql.QSqlQuery()
            texteQuery ="select distinct Entite from Groupes;"
            query.exec_(texteQuery)
            maListe=[]
            while (query.next()) :
                maListe.append(str(query.value(0).toString()))
            return maListe
