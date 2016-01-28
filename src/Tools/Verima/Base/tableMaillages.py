from qtsalome import QSqlQuery
from tableDeBase import TableDeBase

class TableMaillages (TableDeBase):
	def __init__(self):
            TableDeBase.__init__(self,"Maillages")
            self.setField(("id","nomMaillage","Script","fichier","idMailleur","Dimension","Seuil CPU","Seuil Ratio","Seuil Taille","Seuil Nb Maille","Commentaire"))
            self.cols="(nomMaillage,nomScript,medResultat,idMailleur,dimension,seuilCPU,seuilRatio,seuilTaille,seuilNbMaille,commentaire)"
            self.setTypeField(('int','str','str','str','int','int','int','int','int','int','str'),('id'))

        def createSqlTable(self):
            query=QSqlQuery()
            texteQuery ="create table Maillages(id integer primary key autoincrement, nomMaillage varchar(10), "
            texteQuery+="nomScript varchar(40), medResultat varchar(15), idMailleur int, dimension int,"
            texteQuery+="seuilCPU int, seuilRatio int, seuilTaille int, seuilNbMaille int, commentaire varchar(60), "
            texteQuery+="foreign key (idMailleur) references Mailleur(id));"
            print "creation de TableMaillages : " , query.exec_(texteQuery)

        def getVal(self,idMaillage, nomChamp):
            query=QSqlQuery()
            valeur=None
            texteQuery ='select '+ nomChamp + ' from Maillages  where id=' + str(idMaillage) + ";"
            query.exec_(texteQuery)
            while (query.next()) :
              valeur=query.value(0).toInt()[0]
            while (query.next()) :
              print "plusieurs enregistrements dans Maillages pour ",str(idMaillage)
              exit()
            return valeur

        def dejaRemplie(self):
            texteQuery="select * from  Maillages where medResultat='/tmp/tetra.med';"
            maQuery=QSqlQuery()
            maQuery.exec_(texteQuery)
            nb=0
            while(maQuery.next()): nb=nb+1
            return nb

        def remplit(self):
            if self.dejaRemplie():
               print "table Maillage deja initialisee"
               return
#            self.insereLigneAutoId(('Fiche_7566_TUNNEL', '/home/H77945/CAS_TEST/MAILLEUR/FICHE_7566_TUNNEL/Fiche_7566_TUNNEL.py', '/tmp/Fiche_7566_TUNNEL.med', 3,3,10,10,10,10, 'Maillage d un tunnel'))
#            self.insereLigneAutoId(('Fiche_7957_AILETTE', '/home/H77945/CAS_TEST/MAILLEUR/FICHE_7957_AILETTE/Fiche_7957_AILETTE.py', '/tmp/Fiche_7957_AILETTE.med', 1,2,10,10,10,10, 'Maillage d une attache d aillette'))
        
        def construitListeMaillages(self):
            maQuery=QSqlQuery()
            texteQuery="select id, nomScript,medResultat from Maillages;"
            maQuery.exec_(texteQuery)
            listeMaillages=[]
            while(maQuery.next()):
                listeMaillages.append((maQuery.value(0).toInt()[0], maQuery.value(1).toString(), maQuery.value(2).toString()))
            return listeMaillages
           
        def verifieListeMaillages(self,listeMaillage):
            newListeMaillages=[]
            maQuery=QSqlQuery()
            for idM in listeMaillage:
                texteQuery="select id, nomScript,medResultat from Maillages where id = " + str(idM) +';'
                maQuery.exec_(texteQuery)
                maSize=0
                while(maQuery.next()):
                    maSize+=1
                    newListeMaillages.append((maQuery.value(0).toInt()[0], maQuery.value(1).toString(), maQuery.value(2).toString()))
                if maSize != 1 :
                   print "impossible de traiter le maillage : ", idM
            return  newListeMaillages

        def getSeuilsPourMaillage(self,idMaillage):
            texteQuery="select id,nomMaillage,seuilCPU,seuilRatio,seuilTaille,seuilNbMaille from  Maillages where id = "+ str(idMaillage) +"  ;"
            maQuery=QSqlQuery()
            maQuery.exec_(texteQuery)
            while(maQuery.next()): 
                 l1 = maQuery.value(0).toInt()[0]
                 l2 = maQuery.value(1).toString()
                 l3 = maQuery.value(2).toInt()[0]
                 l4 = maQuery.value(3).toInt()[0]
                 l5 = maQuery.value(4).toInt()[0]
                 l6 = maQuery.value(5).toInt()[0]
            return l1,l2,l3,l4,l5,l6

        def getTous(self):
            maillagesIdListe=[]; maillagesNomListe=[]
            texteQuery="select id,nomMaillage from  Maillages order by id;"
            maQuery=QSqlQuery()
            maQuery.exec_(texteQuery)
            while(maQuery.next()):
                 maillagesIdListe.append( maQuery.value(0).toInt()[0])
                 maillagesNomListe.append( maQuery.value(1).toString())
            return maillagesIdListe, maillagesNomListe

        def getMailleurId(self,idMaillage):
            texteQuery="select idMailleur from  Maillages where id = "+ str(idMaillage) +"  ;"
            maQuery=QSqlQuery()
            print texteQuery
            print maQuery.exec_(texteQuery)
            maQuery.exec_(texteQuery)
            while(maQuery.next()): 
                 idMailleur = maQuery.value(0).toInt()[0]
            return idMailleur
