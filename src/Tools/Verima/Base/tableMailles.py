# Copyright (C) 2013-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

from qtsalome import QSqlQuery
from Base.tableDeBase import TableDeBase

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

          print("Creation de TableMailles : " , query.exec_(texteQuery))

 
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
            if nb > 1 : print("Double valeur de Reference dans la table des mailles")
            return val


      def getAllEntity(self):
            query=QSqlQuery()
            texteQuery ="select distinct Entite from Mailles;"
            query.exec_(texteQuery)
            maListe=[]
            while (query.next()) :
                maListe.append(str(query.value(0).toString()))
            return maListe
