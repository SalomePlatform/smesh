from PyQt4 import QtSql, QtCore
from tableDeBase import TableDeBase

class TableVersions (TableDeBase):
	def __init__(self):
            TableDeBase.__init__(self,"Versions")
            self.setField(("id","nomVersion","Reference"))
            self.setTypeField(('int','str','str'),("id"))
            self.cols=" (nomVersion, ref) "

        def createSqlTable(self):
            query=QtSql.QSqlQuery()
            texteQuery ="create table Versions(id integer primary key autoincrement, nomVersion varchar(10),"
            texteQuery+="ref varchar(3));"
            print "Creation de TableVersions : " , query.exec_(texteQuery)

        def dejaRemplie(self):
            texteQuery="select * from  Versions where nomVersion='Salome6.6';"
            maQuery=QtSql.QSqlQuery()
            maQuery.exec_(texteQuery)
            nb=0
            while(maQuery.next()): nb=nb+1
            return nb

        def remplit(self):
#PN         pour tester
            if self.dejaRemplie():
               print "Table des Versions deja initialisee"
               return
            self.insereLigneAutoId(('Salome6.6','non'))
            self.insereLigneAutoId(('Salome7.3','oui'))

        def creeVersion(self,version):
            self.insereLigneAutoId((version,'non'))

        
        def chercheVersion(self,version):
            query=QtSql.QSqlQuery()
            version=str(version)
            if QtCore.QString(version).toInt()[1]==True :
               texteQuery ="select id, nomVersion from Versions where id = " + str(version) +";"
            else:
               texteQuery ="select id, nomVersion from Versions where nomVersion ='" + version +"' ;"
            query.exec_(texteQuery)
            nb=0
            while(query.next()):
               nb=nb+1
               id=query.value(0).toInt()[0]
               nom=query.value(1).toString()
            if nb != 1 : return 0, 0, ""
            return  1, id, nom

        def remplaceVersion(self,version):
            bOk,versionId,versionName = self.chercheVersion(version)
            if bOk == 0 : return "La version choisie comme version de reference n est pas dans la base"
            query=QtSql.QSqlQuery()
            texteQuery ='update Versions set ref="non" where ref="oui";'
            query.exec_(texteQuery)
            texteQuery ='update Versions set ref="oui"where id='+str(versionId)+";"
            query.exec_(texteQuery)
            return "changement effectue"
           
        def getVersionRef(self):
            query=QtSql.QSqlQuery()
            texteQuery ='select id from Versions  where ref="oui";'
            query.exec_(texteQuery)
            while (query.next()) : 
               id=query.value(0).toInt()[0]
            while (query.next()) :
               print "Base corrompue : plusieurs versions de reference"
               exit()
            return id

