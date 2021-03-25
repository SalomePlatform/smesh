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

          print("Creation de TablePerfs : " , query.exec_(texteQuery))

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
              print("plusieurs enregistrements dans perf pour ",str(idMaillage)," ",str(idVersion)," ",Machine)
          if cpu==None :
              print("pas d enregistrement dans perf pour ",str(idMaillage)," ",str(idVersion)," ",Machine)
          return cpu
