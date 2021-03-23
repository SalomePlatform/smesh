# Copyright (C) 2014-2021  EDF R&D
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
          print("Creation de TableGroupeRatios : " , query.exec_(texteQuery))

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
          if nb > 1 : print("Double valeur de Reference dans la table des mailles")
          return val

