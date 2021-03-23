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

class TableVersions (TableDeBase):
    def __init__(self):
        TableDeBase.__init__(self,"Versions")
        self.setField(("id","nomVersion","commentaire"))
        self.setTypeField(('int','str','str'),('id',))
        self.cols=" (nomVersion, commentaire) "

    def createSqlTable(self):
        query=QSqlQuery()
        texteQuery ="create table Versions(id integer primary key autoincrement, nomVersion varchar(10),"
        texteQuery+="commentaire varchar(30));"
        print("Creation de TableVersions : " , query.exec_(texteQuery))


    def remplit(self):
        self.insereLigneAutoId(('Salome7.2.0',''))
        self.insereLigneAutoId(('Salome7.3.0',''))
        self.insereLigneAutoId(('Salome7.4.0',''))

    def creeVersion(self,version,commentaire=""):
        self.insereLigneAutoId((version,commentaire))


    def chercheVersion(self,version):
        query=QSqlQuery()
        version=str(version)
        if bool(version) == True :
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
