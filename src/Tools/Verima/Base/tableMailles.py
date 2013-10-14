from PyQt4 import QtSql, QtCore
from tableDeBase import TableDeBase

class TableMailles (TableDeBase):
      def __init__(self):
         TableDeBase.__init__(self,"Mailles")
         # self.setField(('Maillage','Version','Machine','Node','N0D','Edge','Quad_Edge','Triangle','Quad_Triangle','BiQuad_Triangle','Quadrangle','Quad_Quadrangle','BiQuad_Quadrangle','Polygon','Quad_Polygon','Tetra','Quad_Tetra','Pyramid','Quad_Pyramid','Hexa','Quad_Hexa','TriQuad_Hexa','Penta','Quad_Penta','Hexagonal_Prism','Polyhedra','Quad_Polyhedra',))
         # self.setTypeField(('int','int','str','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int','int'),('idMaillage','idVersion','Machine'))
         self.setField(('Maillage','Version','Machine','Entite','NbEntite'))
         self.setTypeField(('int','int','str','str','int'),('idMaillage','idVersion','Machine','Entite'))


      def createSqlTable(self):
          query=QtSql.QSqlQuery()

          #texteQuery ="create table Mailles(idMaillage int, idVersion int,Machine var(10),  Node int, "
          #texteQuery+="N0D int, Edge int, Quad_Edge int, Triangle int, Quad_Triangle int, BiQuad_Triangle int,"
          #texteQuery+="Quadrangle int, Quad_Quadrangle int,  BiQuad_Quadrangle int,  Polygon int,"
          #texteQuery+="Quad_Polygon int, Tetra int, Quad_Tetra int, Pyramid int, Quad_Pyramid int,"
          #texteQuery+="Hexa int, Quad_Hexa int, TriQuad_Hexa int, Penta int, Quad_Penta int,"
          #texteQuery+="Hexagonal_Prism int, Polyhedra int, Quad_Polyhedra int, "

          texteQuery ="create table Mailles(idMaillage int, idVersion int, Machine var(40), Entite var(40), NbEntite int, "
          texteQuery+="foreign key (idMaillage) references Maillages(id)," 
          texteQuery+="foreign key (idVersion) references Versions(id)," 
          texteQuery+="foreign key (Machine) references Machines(nomMachine)," 
          texteQuery+="primary key (idMaillage,idVersion,Machine,Entite));"

          print "Creation de TableMailles : " , query.exec_(texteQuery)

 
      def getVal(self,idMaillage, idVersion, Machine, Entite):
            query=QtSql.QSqlQuery()
            texteQuery ='select NbEntite from Mailles where idMaillage='+str(idMaillage) 
            texteQuery+=' and  idVersion='+str(idVersion) 
            texteQuery+=" and Machine='" + str(Machine) +"'" 
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
            query=QtSql.QSqlQuery()
            texteQuery ="select distinct Entite from Mailles;"
            query.exec_(texteQuery)
            maListe=[]
            while (query.next()) :
                maListe.append(str(query.value(0).toString()))
            return maListe
