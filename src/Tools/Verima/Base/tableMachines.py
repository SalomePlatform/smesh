from qtsalome import QSqlQuery
from tableDeBase import TableDeBase
import os

class TableMachines (TableDeBase):
	def __init__(self):
            TableDeBase.__init__(self,"Machines")
            self.setField(("nomMachine","Os"))
            self.setTypeField(('str','str'),('nomMachine'))

        def createSqlTable(self):
            query=QSqlQuery()
            print "creation de TableMachine : ", query.exec_("create table Machines(  nomMachine varchar(10) primary key, os varchar(10));")

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

