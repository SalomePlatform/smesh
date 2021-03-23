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

class TableMailleurs (TableDeBase):
    def __init__(self):
        TableDeBase.__init__(self,"Mailleurs")
        self.cols=" (nomMailleur) "
        self.setField(("id","nomMailleur"))
        self.setTypeField(("int","str"),('id'))

    def createSqlTable(self):
        query=QSqlQuery()
        print("Creation de TableMailleurs", query.exec_("create table Mailleurs(id integer primary key autoincrement, nomMailleur varchar(40));"))

    def dejaRemplie(self):
        texteQuery="select * from  Mailleurs where nomMailleur='Blsurf+Ghs3D';"
        maQuery=QSqlQuery()
        maQuery.exec_(texteQuery)
        nb=0
        while(maQuery.next()): nb=nb+1
        return nb

    def remplit(self):
        if self.dejaRemplie() :
            print("Table Mailleurs deja initialisee")
            return
        self.insereLigneAutoId(('BLSURF',))
        self.insereLigneAutoId(('NETGEN1D2D',))
        self.insereLigneAutoId(('GHS3D+BLSURF',))
        self.insereLigneAutoId(('GHS3D+NETGEN1D2D',))
        self.insereLigneAutoId(('NETGEN1D2D3D',))

    def insereLigneAutoId(self,valeurs,debug=False):
        # difficulte a construire le texte avec une seule valeur
        texteQuery='insert into  Mailleurs (nomMailleur) values ("'+ str(valeurs[0])+ '");'
        maQuery=QSqlQuery()
        if debug  : print(texteQuery, " " , maQuery.exec_(texteQuery))
        else : maQuery.exec_(texteQuery)

    def getTous(self):
        l1=[]
        l2=[]
        texteQuery="select * from  Mailleurs;"
        maQuery=QSqlQuery()
        maQuery.exec_(texteQuery)
        while(maQuery.next()):
            l1.append( maQuery.value(0).toInt()[0])
            l2.append( maQuery.value(1).toString())
        return l1,l2

    def getName(self,mailleurId):
        texteQuery="select  nomMailleur from  Mailleurs where id = " + str(mailleurId) + " ;"
        maQuery=QSqlQuery()
        maQuery.exec_(texteQuery)
        while(maQuery.next()):
            mailleurName=maQuery.value(0).toString()
        return mailleurName
