from qtsalome import QSqlQuery
import datetime

class TableDeBase :
	def __init__(self,nom):
	  self.nom=nom

        def setField(self,FieldStringList):
          self.FieldStringList=FieldStringList
          self.idName=FieldStringList[0]
       
        def setTypeField(self,FieldTypeListe,clef):
          self.FieldTypeListe = FieldTypeListe
          self.clef=clef

        def getFields(self):
          return self.FieldStringList
    
        def insereLigne(self,valeurs,debug=False):
          if self.verifieExitenceId(valeurs[0])!=0 :
	     print "impossible d inserer " , valeurs, "dans ", self.nom
	     print "l id est deja existant"
             return False
          texteQuery='insert into ' + self.nom + " values "+ str(valeurs)+ ';'
          maQuery=QSqlQuery()
          if debug : print texteQuery, " " , maQuery.exec_(texteQuery)
          else     : maQuery.exec_(texteQuery)

        def insereLigneAutoId(self,valeurs,debug=False):
          texteQuery='insert into ' + self.nom + self.cols+ " values "+ str(valeurs)+ ';'
          maQuery=QSqlQuery()
          if debug : print texteQuery, " " , maQuery.exec_(texteQuery)
          else     : maQuery.exec_(texteQuery)

        def insereOuRemplaceLigne(self,valeurs,debug=False):
          texteQuery='insert or replace into ' + self.nom + " values "+ str(valeurs)+ ';'
          maQuery=QSqlQuery()
          if debug : print texteQuery, " " , maQuery.exec_(texteQuery)
          else     : maQuery.exec_(texteQuery)


        def verifieExitenceId(self,valeur):
# ne fonctionne pas correctement, il faudrait se servir de la clef
           texteQuery= "select * from " + self.nom + " where "+ self.idName+'='+str(valeur) +';'
           maQuery=QSqlQuery()
           maQuery.exec_(texteQuery)
           nb=0
           while(maQuery.next()): nb=nb+1
           return nb


        def remplit(self):
            print "Pas de remplissage sauf si cette methode est surchargee"

        def createSqlTable(self):
            print "Pas de creation par defaut : cette methode doit etre surchargee"

# On ne se sert pas du csv python entre autre parcequ'il ne gere pas les entetes
        def exportToCSV(self):
           aujourdhui=datetime.date.today()
           monFolder="ExportDB"+str(aujourdhui)
           monFichier=monFolder+"/Sauve_"+str(self.nom)+'.csv'
           texteQuery= "select * from " + self.nom +';'
           texteSauve=""
           for col in self.FieldStringList:
               texteSauve+=col+";"
           texteSauve=texteSauve[0:-1] # on enleve le dernier ";"
           texteSauve+="\n"
         
           maQuery=QSqlQuery()
           maQuery.exec_(texteQuery)
           asauver=0
           while(maQuery.next()):
              asauver=1
              for i in range(len(self.FieldStringList)):
                  texteSauve+=str(maQuery.value(i).toString())+";"
              texteSauve=texteSauve[0:-1] # on enleve le dernier ";"
              texteSauve+="\n"
           
           if asauver == 0 :
              print "pas de sauvegarde de : " , self.nom , " table vide"
              return

           from Stats.utiles import writeFile
           Bok=writeFile(monFichier,texteSauve)
           if Bok :
              print "sauvegarde de : " , self.nom , " effectuee "
           else :
              print "pas de sauvegarde de : " , self.nom , " IOerror"

        def importFromCSV(self,folder,force):
           monFichier=folder+"/Sauve_"+str(self.nom)+'.csv'
           try :
             f=open(monFichier,'r')
           except:
             print "Pas de chargement de la table ", self.nom
             print "Impossible d'ouvrir le fichier ", monFichier
             return 0
           lignes=f.readlines()
           enTete=tuple(lignes[0][0:-1].split(";"))
           if enTete!=self.FieldStringList:
             print "Pas de chargement de la table ", self.nom
             print "les entetes ne correspondent pas"
             return 0
           for StrVal in lignes[1:]:
               listeVal=tuple(StrVal[0:-1].split(";"))
               listeValTypee=[]
               for i in range(len(listeVal)):
                   if self.FieldTypeListe[i]=='int'  : listeValTypee.append(int(listeVal[i]))
                   if self.FieldTypeListe[i]=='float': listeValTypee.append(float(listeVal[i]))
                   if self.FieldTypeListe[i]=='str'  : listeValTypee.append(listeVal[i])
               if force==1 : self.insereOuRemplaceLigne(tuple(listeValTypee))
               if force==0 : self.insereLigne(tuple(listeValTypee))

          
