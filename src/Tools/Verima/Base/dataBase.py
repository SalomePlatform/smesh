# -*- coding: utf-8 -*-

import os
import datetime
import sys

from PyQt4 import QtSql, QtCore
from tableMaillages  import TableMaillages
from tableMailleurs  import TableMailleurs
from tableMachines   import TableMachines
from tableVersions   import TableVersions
from tableGroupesRef import TableGroupesRef
from tableGroupes    import TableGroupes
from tableMailles    import TableMailles
from tableTailles    import TableTailles
from tableRatios     import TableRatios
from tablePerfs      import TablePerfs
from Stats.job       import Job
from Stats.ref       import Ref
from CreeDocuments.jobHtml       import Document


class Base:
  def __init__(self,file):    
       self.db  = QtSql.QSqlDatabase.addDatabase("QSQLITE")
       self.db.setDatabaseName(file)
       self.db.setHostName("localhost");
       self.db.setUserName("");
       self.db.setPassword("")
       if not self.db.open():
         print(self.db.lastError().text())
       else:
         print "dataBase Open"
       self.file=file
        
  def create(self):
      self.maTableMailleurs=TableMailleurs()
      self.maTableMailleurs.createSqlTable()
      self.maTableMachines=TableMachines()
      self.maTableMachines.createSqlTable()
      self.maTableVersions=TableVersions()
      self.maTableVersions.createSqlTable()
      self.maTableMaillages=TableMaillages()
      self.maTableMaillages.createSqlTable()
      self.maTableGroupesRef=TableGroupesRef()
      self.maTableGroupesRef.createSqlTable()
      self.maTablePerfs=TablePerfs()
      self.maTablePerfs.createSqlTable()
      self.maTableMailles=TableMailles()
      self.maTableMailles.createSqlTable()
      self.maTableTailles=TableTailles()
      self.maTableTailles.createSqlTable()
      self.maTableGroupes=TableGroupes()
      self.maTableGroupes.createSqlTable()
      self.maTableRatios=TableRatios()
      self.maTableRatios.createSqlTable()
    
  def initialise(self):
      self.maTableMaillages=TableMaillages()
      self.maTableMailleurs=TableMailleurs()
      self.maTableVersions=TableVersions()
      self.maTableMachines=TableMachines()
      self.maTableMailles=TableMailles()
      self.maTableTailles=TableTailles()
      self.maTableGroupes=TableGroupes()
      self.maTableGroupesRef=TableGroupesRef()
      self.maTablePerfs=TablePerfs()
      self.maTableRatios=TableRatios()
 
  def remplit(self):
      self.maTableMailleurs.remplit()
      self.maTableMaillages.remplit()
      self.maTableVersions.remplit()
      #self.maTableVersions.creeVersion()
      self.maTableMachines.creeMachine()
      self.maTableGroupesRef.remplit()

  def close(self):
       self.db.close()
       
  def exportToCSV(self,partiel):
      aujourdhui=datetime.date.today()
      monFolder="ExportDB"+str(aujourdhui)
      try :
        os.mkdir(monFolder)
      except :
        pass
      self.maTableMachines.exportToCSV()
      self.maTableMailles.exportToCSV()
      self.maTableTailles.exportToCSV()
      self.maTableGroupes.exportToCSV()
      self.maTablePerfs.exportToCSV()
      self.maTableRatios.exportToCSV()
      if partiel==True: return
      self.maTableMailleurs.exportToCSV()
      self.maTableVersions.exportToCSV()
      self.maTableMaillages.exportToCSV()
      self.maTableMachines.exportToCSV()
      self.maTableGroupesRef.exportToCSV()

  def importFromCSV(self,folder,partiel,force):
      if partiel==False: 
         self.maTableMailleurs.importFromCSV(folder,force)
         self.maTableVersions.importFromCSV(folder,force)
         self.maTableMaillages.importFromCSV(folder,force)
         self.maTableGroupesRef.importFromCSV(folder,force)
         self.maTableMachines.importFromCSV(folder,force)
      self.maTableMailles.importFromCSV(folder,force)
      self.maTableGroupes.importFromCSV(folder,force)
      self.maTablePerfs.importFromCSV(folder,force)
      self.maTableRatios.importFromCSV(folder,force)
      self.maTableGroupesRef.importFromCSV(folder,force)

  def Structure(self):
      # jamais appelee. juste pour memoire
      texteQuery='select * from sqlite_master;'
      maQuery=QSqlQuery()
      return maQuery.exec_(texteQuery)

  def passeJobs(self,all,salomePath,version,force,listeMaillage):
      if all==True: paramMaillage=self.maTableMaillages.construitListeMaillages()
      else        : paramMaillage=self.maTableMaillages.verifieListeMaillages(listeMaillage)
      # paramMaillage=(( id, script, fichier med),)

      #version="Salome7.3"
      bOk,versionId,versionName = self.maTableVersions.chercheVersion(version)
      if bOk==False:
         self.maTableVersions.creeVersion(version)
         print "nouvelle Version enregistree dans la base"
         bOk,versionId,versionName = self.maTableVersions.chercheVersion(version)
         if bOk==False:
            print "Impossible de creer la version"
            return

      bOk,nomMachine = self.maTableMachines.chercheMachine()
      if bOk==False:
         self.maTableMachines.creeMachine()
         print "enregistrement de la machine dans la table des machines"
         bOk,nomMachine = self.maTableMachines.chercheMachine()
         if bOk==False:
            print "Impossible de creer la version"
            return


      for params in paramMaillage:
        
         print "___________________________________________"
         print ""
         print " Job : ", params[1]
         print " Version de salome : ", versionName 

         #recherche si le Job a deja ete passe
         if self.maTablePerfs.getVal(params[0],versionId,nomMachine) !=None and force==False:
            print "job numero: " , params[0], "deja passe"
            print ""
            print "___________________________________________"
            continue

         # recherche de la reference du Job
         maRef=Ref(self,params[0],versionId,nomMachine)
         idVersionRef=self.maTableVersions.getVersionRef()
         idJob=params[0]
         if maRef.existe==False and idVersionRef != versionId :
            print "pas de reference pour le job : ", params[0]
            print "pas de passage du job", 
            continue


         mesGroupesRef=self.maTableGroupesRef.getVals(idJob)
         fichierGroupesRef=str(params[2]).replace('.med','_groupesRef.res')
         from Stats.utiles import writeFile
         writeFile(fichierGroupesRef,",".join(mesGroupesRef))
         monjob=Job(params,salomePath,versionId,nomMachine,mesGroupesRef)
         print ""
         print "  Debut d execution"
         monjob.execute()
      
         # remplit Perfs
         self.maTablePerfs.insereOuRemplaceLigne((idJob,versionId,nomMachine,int(monjob.getCPU()),0),False)
         if idVersionRef != versionId :
            stop=maRef.verifieCpu(monjob.getCPU())
            if stop :
               print "                   Arret                "
               print " Pb CPU: Seuil Atteint sur job : ", params[0]
               print "________________________________________"

         # remplit la table Mailles
         listeColonnes,listeValues=monjob.getStatSurMailles()
         print listeColonnes,listeValues
         i=0
         for col in listeColonnes :
            lVal=[idJob,versionId,nomMachine,col]+[listeValues[i]]
            lVal=tuple(lVal)
            if str(listeValues[i]) != str(0): self.maTableMailles.insereOuRemplaceLigne(lVal,False)
            i+=1
         if idVersionRef != versionId :
            stop=maRef.verifieMailles(listeValues,listeColonnes)
            if stop :
               print "                   Arret                "
               print " Pb sur le nombre de Maille : Seuil Atteint sur job : ", params[0]
               print "_________________________________________________________________"
               #exit()
            else :
               print "verification du Nb de Mailles effectuee"

         # remplit la table Tailles
         listeValues=monjob.getStatSurTailles()
         lVal=[params[0],versionId,nomMachine]+listeValues
         self.maTableTailles.insereOuRemplaceLigne(tuple(lVal),False)
         if idVersionRef != versionId :
            stop=maRef.verifieTailles(lVal)
            if stop :
               print "                   Arret                "
               print " Pb sur la taille des mailles : Seuil Atteint sur job : ", params[0]
               print "_________________________________________________________________"
               #exit()
            else :
               print "verification du Nb de Mailles effectuee"


         # remplit la table Ratio
         maDim=self.maTableMaillages.getVal(params[0],'dimension')
         if maDim == 3 :
            listeValues=monjob.getStatSurRatios()
            lVal=[params[0],versionId,nomMachine]+listeValues
            self.maTableRatios.insereOuRemplaceLigne(tuple(lVal),False)
            if idVersionRef != versionId :
               stop=maRef.verifieRatios(lVal)
               if stop :
                  print "                   Arret                "
                  print " Pb sur la taille des mailles : Seuil Atteint sur job : ", params[0]
                  print "_________________________________________________________________"
                  #exit()
               else :
                  print "verification du Nb de Mailles effectuee"

         # remplit les Groupes
         for groupe in mesGroupesRef:
             listeColonnes,listeValues=monjob.getStatSurGroupes(groupe)
             i=0
             for col in listeColonnes :
                lVal=[groupe,params[0],versionId,nomMachine,col]+[listeValues[i]]
                lVal=tuple(lVal)
                if str(listeValues[i]) != str(0): self.maTableGroupes.insereOuRemplaceLigne(lVal,False)
                i=i+1
             if idVersionRef != versionId :
                stop=maRef.verifieMaillesPourGroupes(groupe,listeValues,listeColonnes)
                stop=0
                if stop :
                   print "                   Arret                "
                   print " Pb sur le nombre de Maille : Seuil Atteint sur job : ", params[0]
                   print "_________________________________________________________________"
                   #exit()
         print "verification du Nb de Mailles sur les groupes reference effectuee"
         print "_________________________________________________________________"

  
  def compare(self,version,versionRef,fichier):    
      print "_________________________________________________________________"
      print "Generation du rapport de comparaison" 
      bOk,versionId,versionName = self.maTableVersions.chercheVersion(version)
      if bOk==False :
         print "version ", version , " inconnue dans la base"
         exit()
      print "Version a comparer : ", versionName
      versionCompName=versionName
      versionCompId=versionId

      if versionRef==None:
         idVersionRef=self.maTableVersions.getVersionRef()
      bOk,versionId,versionName = self.maTableVersions.chercheVersion(idVersionRef)
      if bOk==False :
         print "version de référence ", versionRef , " inconnue dans la base"
      versionRefName=versionName
      versionRefId=versionId
      print "Version de comparaison : ", versionRefName

      bOk,nomMachine = self.maTableMachines.chercheMachine()
      monDocument=Document()
      monDocument.initEntete(versionName, versionRefName,nomMachine)
      mailleursIdListe,mailleursNameListe=self.maTableMailleurs.getTous()
      
      allEntitySurMaille=self.maTableMailles.getAllEntity()
      allEntitySurGroupe=self.maTableGroupes.getAllEntity()
      # Boucle sur les mailleurs
      for indexMailleur in range(len(mailleursNameListe)):
          monDocument.initMailleur(mailleursNameListe[indexMailleur]) 
          l1,l2,l3,l4,l5,l6=self.maTableMaillages.getTousPourMaillage(mailleursIdListe[indexMailleur])
          maillagesIdListe=l1
          maillagesNameListe=l2
          maillagesSeuilCPUListe=l3
          maillagesSeuilRatioListe=l4
          maillagesSeuilTailleListe=l5
          maillagesSeuilNbMailleListe=l6
          dicoMaillage={}
      #   Boucle sur les maillages 
          for indexMaillage in range(len(maillagesNameListe)):
             idMaillage=maillagesIdListe[indexMaillage]
             dicoMaillage["NOM"]=maillagesNameListe[indexMaillage]
             dicoMaillage["NBCPU"]=self.maTablePerfs.getVal(idMaillage,versionCompId,nomMachine)
             dicoMaillage["REFCPU"]=self.maTablePerfs.getVal(idMaillage,versionRefId,nomMachine)
             dicoMaillage["DIFCPU"],dicoMaillage["DIFREL"],dicoMaillage["WARNING"]=self.calculDiffCPU(dicoMaillage["NBCPU"],dicoMaillage["REFCPU"],maillagesSeuilCPUListe[indexMaillage])
  
             dicoMaillage["RMAX"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'RatioMax')
             dicoMaillage["RMAXREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'RatioMax')
             dicoMaillage["RMIN"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'RatioMin')
             dicoMaillage["RMINREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'RatioMin')
             dicoMaillage["RMOY"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'Moyenne')
             dicoMaillage["RMOYREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'Moyenne')
             dicoMaillage["R1Q"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'Q1')
             dicoMaillage["R1QREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'Q1')
             dicoMaillage["RMED"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'Mediane')
             dicoMaillage["RMEDREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'Mediane')
             dicoMaillage["R3Q"]=self.maTableRatios.getVal(idMaillage,versionCompId,nomMachine,'Q3')
             dicoMaillage["R3QREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,nomMachine,'Q3')
  
             dicoMaillage["TMAX"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'TailleMax')
             dicoMaillage["TMAXREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'TailleMax')
             dicoMaillage["TMIN"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'TailleMin')
             dicoMaillage["TMINREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'TailleMin')
             dicoMaillage["TMOY"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'Moyenne')
             dicoMaillage["TMOYREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'Moyenne')
             dicoMaillage["T1Q"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'Q1')
             dicoMaillage["T1QREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'Q1')
             dicoMaillage["TMED"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'Mediane')
             dicoMaillage["TMEDREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'Mediane')
             dicoMaillage["T3Q"]=self.maTableTailles.getVal(idMaillage,versionCompId,nomMachine,'Q3')
             dicoMaillage["T3QREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,nomMachine,'Q3')

             monDocument.initJob(dicoMaillage)

             i=0
             dico={}
             dico["TITRE"]="Nombre de Mailles dans le maillage entier"
             for nomColonne in allEntitySurMaille:
                 val=self.maTableMailles.getVal(idMaillage,versionCompId,nomMachine,nomColonne)
                 valRef=self.maTableMailles.getVal(idMaillage,versionRefId,nomMachine,nomColonne)
                 if val==0 and valRef==0 : continue
                 i=i+1
                 #print nomColonne;print val; print valRef
                 cMAIL="MAIL"+str(i); cREF="REF"+str(i)
                 cNB="NB"+str(i); cDIF="DIF"+str(i); cREL="REL"+str(i)
                 cWARN="WARN"+str(i);
                 dico[cMAIL]=nomColonne
                 dico[cREF]=valRef
                 dico[cNB]=val
                 dico[cDIF],dico[cREL],dico[cWARN]=self.calculDiff(val,valRef,maillagesSeuilNbMailleListe[indexMaillage])
             monDocument.initMaille(dico,i)

             # Boucle sur les groupes du maillage
             mesGroupesRef=self.maTableGroupesRef.getVals(idMaillage)
             if mesGroupesRef==[] : monDocument.addNoGroup()
             for groupeId in mesGroupesRef:
                 i=0
                 dico={}
                 dico["TITRE"]="Nombre de Mailles dans le groupe "+groupeId
                 for nomColonne in allEntitySurGroupe:
                     val=self.maTableGroupes.getVal(groupeId,idMaillage,versionCompId,nomMachine,nomColonne)
                     valRef=self.maTableGroupes.getVal(groupeId,idMaillage,versionRefId,nomMachine,nomColonne)
                     if val==0 and valRef==0 : continue
                     i=i+1
                     cMAIL="MAIL"+str(i); cREF="REF"+str(i)
                     cNB="NB"+str(i); cDIF="DIF"+str(i); cREL="REL"+str(i)
                     cWARN="WARN"+str(i);
                     #print nomColonne," ",val," ",valRef
                     dico[cMAIL]=nomColonne
                     dico[cREF]=valRef
                     dico[cNB]=val
                     dico[cDIF],dico[cREL],dico[cWARN]=self.calculDiff(val,valRef,maillagesSeuilNbMailleListe[indexMaillage])
                 monDocument.initMaille(dico,i)

      monDocument.creeDocument(fichier)

  def calculDiffCPU(self,nb,nbRef,seuil):    
         # different de calculDiff : si on est inferieur a la reference
         # on n a pas de warning
         diff=nb-nbRef
         diffRel=((nb-nbRef)*100)/(nbRef*1.00)
         if diffRel > seuil :
            warning="plus de "+ str(seuil) + "% de difference CPU"
         else :
            warning=""
         diffRelStr=str(diffRel)+"%"
         return diff,diffRelStr,warning
         
  def calculDiff(self,nb,nbRef,seuil):    
         diff=nb-nbRef
         diffRel=((nb-nbRef)*100)/(nbRef*1.00)
         if diffRel > seuil or (-1*diffRel) > seuil  :
            warning="difference superieure a "+ str(seuil) + "%"
         else :
            warning=""
         diffRelStr=str(diffRel)+"%"
         return diff,diffRelStr,warning
         
