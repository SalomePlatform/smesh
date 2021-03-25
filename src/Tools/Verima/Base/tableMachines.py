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

import os
from qtsalome import QSqlQuery
from Base.tableDeBase import TableDeBase

class TableMachines (TableDeBase):
    def __init__(self):
        TableDeBase.__init__(self,"Machines")
        self.setField(("nomMachine","Os"))
        self.setTypeField(('str','str'),('nomMachine'))

    def createSqlTable(self):
        query=QSqlQuery()
        print("creation de TableMachine : ", query.exec_("create table Machines(  nomMachine varchar(10) primary key, os varchar(10));"))

    def creeMachine(self):
        nomMachine=os.uname()[1]
        nomOs=os.uname()[2]
        self.insereLigne((nomMachine,nomOs))

    def chercheMachine(self):
        query=QSqlQuery()
        machine=os.uname()[1]
        texteQuery ="select nomMachine from Machines where nomMachine ='" + machine +"' ;"
        query.exec_(texteQuery)
        nb=0
        while(query.next()):
            nb=nb+1
            nom=str(query.value(0).toString())
        if nb != 1 : return 0,  ""
        return  1,  nom
