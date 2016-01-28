# -*- coding: utf-8 -*-

import os
import datetime
import sys

from qtsalome import QSqlQuery
from tableMaillages  import TableMaillages
from tableMailleurs  import TableMailleurs
from tableMachines   import TableMachines
from tableVersions   import TableVersions
from tableGroupesRef import TableGroupesRef
from tableGroupes    import TableGroupes
from tableMailles    import TableMailles
from tableTailles    import TableTailles
from tableRatios     import TableRatios
from tableGroupeRatios     import TableGroupeRatios
from tableGroupeTailles    import TableGroupeTailles
from tablePerfs      import TablePerfs
from Stats.job       import Job
from CreeDocuments.jobHtml       import Document


class Base:
  def __init__(self,file):    
       self.db  = QSqlDatabase.addDatabase("QSQLITE")
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
      self.maTableGroupeRatios=TableGroupeRatios()
      self.maTableGroupeRatios.createSqlTable()
      self.maTableGroupeTailles=TableGroupeTailles()
      self.maTableGroupeTailles.createSqlTable()
    
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
      self.maTableGroupeRatios=TableGroupeRatios()
      self.maTableGroupeTailles=TableGroupeTailles()
 
  def remplit(self):
      self.maTableMailleurs.remplit()
      self.maTableMaillages.remplit()
      self.maTableVersions.remplit()
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
      self.maTableGroupeRatios.exportToCSV()
      self.maTableGroupeTailles.exportToCSV()
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
      self.maTableGroupeRatios.importFromCSV(folder,force)
      self.maTableGroupeTailles.importFromCSV(folder,force)

  def Structure(self):
      # jamais appelee. juste pour memoire
      texteQuery='select * from sqlite_master;'
      maQuery=QSqlQuery()
      return maQuery.exec_(texteQuery)

  def passeJobs(self,all,salomePath,version,force,listeMaillage):
      if all==True: paramMaillage=self.maTableMaillages.construitListeMaillages()
      else        : paramMaillage=self.maTableMaillages.verifieListeMaillages(listeMaillage)
      # paramMaillage=(( id, script, fichier med),)

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


         idJob=params[0]
         mesGroupesRef=self.maTableGroupesRef.getVals(idJob)
         fichierGroupesRef=str(params[2]).replace('.med','_groupesRef.res')
         from Stats.utiles import writeFile
         if mesGroupesRef != [] :
           writeFile(fichierGroupesRef,",".join(mesGroupesRef))
         monjob=Job(params,salomePath,versionId,mesGroupesRef)
         print ""
         print "  Debut d execution"
         monjob.execute()
      
         # remplit Perfs
         self.maTablePerfs.insereOuRemplaceLigne((idJob,versionId,nomMachine,int(monjob.getCPU()),0),False)

         # remplit la table Mailles
         listeColonnes,listeValues=monjob.getStatSurMailles()
         i=0
         for col in listeColonnes :
            lVal=[idJob,versionId,col]+[listeValues[i]]
            lVal=tuple(lVal)
            if str(listeValues[i]) != str(0): self.maTableMailles.insereOuRemplaceLigne(lVal,False)
            i+=1

         # remplit la table Tailles
         listeValues=monjob.getStatSurTailles()
         lVal=[params[0],versionId]+listeValues
         self.maTableTailles.insereOuRemplaceLigne(tuple(lVal),False)

         # remplit la table Ratio
         maDim=self.maTableMaillages.getVal(params[0],'dimension')
         listeValues=monjob.getStatSurRatios()
         lVal=[params[0],versionId]+listeValues
         self.maTableRatios.insereOuRemplaceLigne(tuple(lVal),False)

         # remplit les Groupes
         for groupe in mesGroupesRef:
             listeColonnes,listeValues=monjob.getStatSurGroupes(groupe)
             i=0
             for col in listeColonnes :
                lVal=[groupe,params[0],versionId,col]+[listeValues[i]]
                lVal=tuple(lVal)
                if str(listeValues[i]) != str(0): self.maTableGroupes.insereOuRemplaceLigne(lVal,False)
                i=i+1

         # remplit les Ratio Groupes
         for groupe in mesGroupesRef:
             listeValues=monjob.getStatSurRatiosGroupes(groupe)
             if listeValues==['0','0','0','0','0','0'] : continue
             lVal=[params[0],versionId,groupe]+listeValues
             self.maTableGroupeRatios.insereOuRemplaceLigne(tuple(lVal),False)
             listeValues=monjob.getStatSurTaillesGroupes(groupe)
             lVal=[params[0],versionId,groupe]+listeValues
             self.maTableGroupeTailles.insereOuRemplaceLigne(tuple(lVal),False)
         monjob.menage()


  
  def compare(self,version,ListeVersionRefString,fichier):    
      print "_________________________________________________________________"
      print "Generation du rapport de comparaison" 
      print version
      bOk,versionId,versionName = self.maTableVersions.chercheVersion(version)
      if bOk==False :
         print "version ", version , " inconnue dans la base"
         exit()
      print "Version a comparer : ", versionName
      versionCompName=versionName
      versionCompId=versionId

      listeVersionRefId=[]
      listeVersionRefName=[]
      ListeVersionRef=ListeVersionRefString.split(",")
      for id in ListeVersionRef:
          bOk,versionId,versionName = self.maTableVersions.chercheVersion(id)
          if bOk==False :
             print "version ", id , " inconnue dans la base"
             exit()
          listeVersionRefId.append(versionId)
          listeVersionRefName.append(versionName)

      bOk,nomMachine = self.maTableMachines.chercheMachine()
      monDocument=Document()
      monDocument.initEntete(versionCompName, nomMachine)

      maillagesIdListe, maillagesNameListe=self.maTableMaillages.getTous()
      if len(maillagesIdListe) != len (listeVersionRefId):
         print "Pas assez de version de reference"
         exit()
      
      allEntitySurMaille=self.maTableMailles.getAllEntity()
      allEntitySurGroupe=self.maTableGroupes.getAllEntity()

      # Boucle sur les maillages
      for idMaillage in maillagesIdListe :
          print idMaillage
          versionRefId=listeVersionRefId[idMaillage - 1]
          versionRefName=listeVersionRefName[idMaillage - 1]
          mailleurId=self.maTableMaillages.getMailleurId(idMaillage)
          mailleurName=self.maTableMailleurs.getName(mailleurId)

          # Recuperation des seuils
          l1,l2,l3,l4,l5,l6=self.maTableMaillages.getSeuilsPourMaillage(idMaillage)
          monSeuilCPU=l3
          monSeuilRatio=l4
          monSeuilTaille=l5
          monSeuilNbMaille=l6


          # Et du temps CPU
          dicoMaillage={}
          dicoMaillage["NBCPU"]=self.maTablePerfs.getVal(idMaillage,versionCompId,nomMachine)
          dicoMaillage["REFCPU"]=self.maTablePerfs.getVal(idMaillage,versionRefId,nomMachine)
          dicoMaillage["DIFCPU"],dicoMaillage["DIFREL"],dicoMaillage["WARNING"]=self.calculDiffCPU(dicoMaillage["NBCPU"],dicoMaillage["REFCPU"],monSeuilCPU)
          monDocument.initMaillage(maillagesNameListe[idMaillage-1],mailleurName,versionRefName,dicoMaillage) 

          # Boucle sur les Mailles

          dico={}
          dico["TITRE"]="Nombre de Mailles dans le maillage entier"
          i=0
          for nomColonne in allEntitySurMaille:
                 val=self.maTableMailles.getVal(idMaillage,versionCompId,nomColonne)
                 valRef=self.maTableMailles.getVal(idMaillage,versionRefId,nomColonne)
                 if val==0 and valRef==0 : continue
                 i=i+1
                 #print nomColonne;print val; print valRef
                 cMAIL="MAIL"+str(i); cREF="REF"+str(i)
                 cNB="NB"+str(i); cDIF="DIF"+str(i); cREL="REL"+str(i)
                 cWARN="WARN"+str(i);
                 dico[cMAIL]=nomColonne
                 dico[cREF]=valRef
                 dico[cNB]=val
                 dico[cDIF],dico[cREL],dico[cWARN]=self.calculDiff(val,valRef,monSeuilNbMaille)
          monDocument.initMaille(dico,i)



          #
          dicoMaillage={}

          dicoMaillage["RMAX"]=self.maTableRatios.getVal(idMaillage,versionCompId,'RatioMax')
          dicoMaillage["RMAXREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'RatioMax')
          dicoMaillage["DIFARMAX"],dicoMaillage["DIFRRMAX"],dicoMaillage["WRRMAX"]=self.calculDiff(dicoMaillage["RMAX"],dicoMaillage["RMAXREF"],monSeuilRatio)
          dicoMaillage["RMIN"]=self.maTableRatios.getVal(idMaillage,versionCompId,'RatioMin')
          dicoMaillage["RMINREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'RatioMin')
          dicoMaillage["DIFARMIN"],dicoMaillage["DIFRRMIN"],dicoMaillage["WRRMIN"]=self.calculDiff(dicoMaillage["RMIN"],dicoMaillage["RMINREF"],monSeuilRatio)
          dicoMaillage["RMOY"]=self.maTableRatios.getVal(idMaillage,versionCompId,'Moyenne')
          dicoMaillage["RMOYREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'Moyenne')
          dicoMaillage["DIFARMOY"],dicoMaillage["DIFRRMOY"],dicoMaillage["WRRMOY"]=self.calculDiff(dicoMaillage["RMOY"],dicoMaillage["RMOYREF"],monSeuilRatio)
          dicoMaillage["R1Q"]=self.maTableRatios.getVal(idMaillage,versionCompId,'Q1')
          dicoMaillage["R1QREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'Q1')
          dicoMaillage["DIFAR1Q"],dicoMaillage["DIFRR1Q"],dicoMaillage["WRR1Q"]=self.calculDiff(dicoMaillage["R1Q"],dicoMaillage["R1QREF"],monSeuilRatio)
          dicoMaillage["RMED"]=self.maTableRatios.getVal(idMaillage,versionCompId,'Mediane')
          dicoMaillage["RMEDREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'Mediane')
          dicoMaillage["DIFARMED"],dicoMaillage["DIFRRMED"],dicoMaillage["WRRMED"]=self.calculDiff(dicoMaillage["RMED"],dicoMaillage["RMEDREF"],monSeuilRatio)
          dicoMaillage["R3Q"]=self.maTableRatios.getVal(idMaillage,versionCompId,'Q3')
          dicoMaillage["R3QREF"]=self.maTableRatios.getVal(idMaillage,versionRefId,'Q3')
          dicoMaillage["DIFAR3Q"],dicoMaillage["DIFRR3Q"],dicoMaillage["WRR3Q"]=self.calculDiff(dicoMaillage["R3Q"],dicoMaillage["R3QREF"],monSeuilRatio)
 
          dicoMaillage["TMAX"]=self.maTableTailles.getVal(idMaillage,versionCompId,'TailleMax')
          dicoMaillage["TMAXREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'TailleMax')
          dicoMaillage["DIFATMAX"],dicoMaillage["DIFRTMAX"],dicoMaillage["WTMAX"]=self.calculDiff(dicoMaillage["TMAX"],dicoMaillage["TMAXREF"],monSeuilTaille)
          dicoMaillage["TMIN"]=self.maTableTailles.getVal(idMaillage,versionCompId,'TailleMin')
          dicoMaillage["TMINREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'TailleMin')
          dicoMaillage["DIFATMIN"],dicoMaillage["DIFRTMIN"],dicoMaillage["WTMIN"]=self.calculDiff(dicoMaillage["TMIN"],dicoMaillage["TMINREF"],monSeuilTaille)
          dicoMaillage["TMOY"]=self.maTableTailles.getVal(idMaillage,versionCompId,'Moyenne')
          dicoMaillage["TMOYREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'Moyenne')
          dicoMaillage["DIFATMOY"],dicoMaillage["DIFRTMOY"],dicoMaillage["WTMOY"]=self.calculDiff(dicoMaillage["TMOY"],dicoMaillage["TMOYREF"],monSeuilTaille)
          dicoMaillage["T1Q"]=self.maTableTailles.getVal(idMaillage,versionCompId,'Q1')
          dicoMaillage["T1QREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'Q1')
          dicoMaillage["DIFAT1Q"],dicoMaillage["DIFRT1Q"],dicoMaillage["WT1Q"]=self.calculDiff(dicoMaillage["T1Q"],dicoMaillage["T1QREF"],monSeuilTaille)
          dicoMaillage["TMED"]=self.maTableTailles.getVal(idMaillage,versionCompId,'Mediane')
          dicoMaillage["TMEDREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'Mediane')
          dicoMaillage["DIFATMED"],dicoMaillage["DIFRTMED"],dicoMaillage["WTMED"]=self.calculDiffCPU(dicoMaillage["TMED"],dicoMaillage["TMEDREF"],monSeuilTaille)
          dicoMaillage["T3Q"]=self.maTableTailles.getVal(idMaillage,versionCompId,'Q3')
          dicoMaillage["T3QREF"]=self.maTableTailles.getVal(idMaillage,versionRefId,'Q3')
          dicoMaillage["DIFAT3Q"],dicoMaillage["DIFRT3Q"],dicoMaillage["WT3Q"]=self.calculDiffCPU(dicoMaillage["T3Q"],dicoMaillage["T3QREF"],monSeuilTaille)

          #PNPNPN a revoir le initJob
          monDocument.initJob(dicoMaillage)


          # Boucle sur les groupes du maillage
          mesGroupesRef=self.maTableGroupesRef.getVals(idMaillage)
          if mesGroupesRef==[] : monDocument.addNoGroup()
          for groupeId in mesGroupesRef:
              i=0
              dico={}
              dico["TITRE"]="Nombre de Mailles dans le groupe "+groupeId
              for nomColonne in allEntitySurGroupe:
                  val=self.maTableGroupes.getVal(groupeId,idMaillage,versionCompId,nomColonne)
                  valRef=self.maTableGroupes.getVal(groupeId,idMaillage,versionRefId,nomColonne)
                  if val==0 and valRef==0 : continue
                  i=i+1
                  cMAIL="MAIL"+str(i); cREF="REF"+str(i)
                  cNB="NB"+str(i); cDIF="DIF"+str(i); cREL="REL"+str(i)
                  cWARN="WARN"+str(i);
                  dico[cMAIL]=nomColonne
                  dico[cREF]=valRef
                  dico[cNB]=val
                  dico[cDIF],dico[cREL],dico[cWARN]=self.calculDiff(val,valRef,monSeuilNbMaille)
                  monDocument.initMaille(dico,i)


              #
              dico={}
    
              if self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'RatioMax') != 0 :
                 dico["RMAX"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'RatioMax')
                 dico["RMAXREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'RatioMax')
                 dico["DIFARMAX"],dico["DIFRRMAX"],dico["WRRMAX"]=self.calculDiff(dico["RMAX"],dico["RMAXREF"],monSeuilRatio)
                 dico["RMIN"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'RatioMin')
                 dico["RMINREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'RatioMin')
                 dico["DIFARMIN"],dico["DIFRRMIN"],dico["WRRMIN"]=self.calculDiff(dico["RMIN"],dico["RMINREF"],monSeuilRatio)
                 dico["RMOY"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'Moyenne')
                 dico["RMOYREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'Moyenne')
                 dico["DIFARMOY"],dico["DIFRRMOY"],dico["WRRMOY"]=self.calculDiff(dico["RMOY"],dico["RMOYREF"],monSeuilRatio)
                 dico["R1Q"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'Q1')
                 dico["R1QREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'Q1')
                 dico["DIFAR1Q"],dico["DIFRR1Q"],dico["WRR1Q"]=self.calculDiff(dico["R1Q"],dico["R1QREF"],monSeuilRatio)
                 dico["RMED"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'Mediane')
                 dico["RMEDREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'Mediane')
                 dico["DIFARMED"],dico["DIFRRMED"],dico["WRRMED"]=self.calculDiff(dico["RMED"],dico["RMEDREF"],monSeuilRatio)
                 dico["R3Q"]=self.maTableGroupeRatios.getVal(idMaillage,versionCompId,groupeId,'Q3')
                 dico["R3QREF"]=self.maTableGroupeRatios.getVal(idMaillage,versionRefId,groupeId,'Q3')
                 dico["DIFAR3Q"],dico["DIFRR3Q"],dico["WRR3Q"]=self.calculDiff(dico["R3Q"],dico["R3QREF"],monSeuilRatio)
                 monDocument.CreeGroupeRatios(dico)
     
              dico={}
              if self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'TailleMax') != 0:
                 dico["TMAX"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'TailleMax')
                 dico["TMAXREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'TailleMax')
                 dico["DIFATMAX"],dico["DIFRTMAX"],dico["WTMAX"]=self.calculDiff(dico["TMAX"],dico["TMAXREF"],monSeuilTaille)
                 dico["TMIN"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'TailleMin')
                 dico["TMINREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'TailleMin')
                 dico["DIFATMIN"],dico["DIFRTMIN"],dico["WTMIN"]=self.calculDiff(dico["TMIN"],dico["TMINREF"],monSeuilTaille)
                 dico["TMOY"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'Moyenne')
                 dico["TMOYREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'Moyenne')
                 dico["DIFATMOY"],dico["DIFRTMOY"],dico["WTMOY"]=self.calculDiff(dico["TMOY"],dico["TMOYREF"],monSeuilTaille)
                 dico["T1Q"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'Q1')
                 dico["T1QREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'Q1')
                 dico["DIFAT1Q"],dico["DIFRT1Q"],dico["WT1Q"]=self.calculDiff(dico["T1Q"],dico["T1QREF"],monSeuilTaille)
                 dico["TMED"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'Mediane')
                 dico["TMEDREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'Mediane')
                 dico["DIFATMED"],dico["DIFRTMED"],dico["WTMED"]=self.calculDiffCPU(dico["TMED"],dico["TMEDREF"],monSeuilTaille)
                 dico["T3Q"]=self.maTableGroupeTailles.getVal(idMaillage,versionCompId,groupeId,'Q3')
                 dico["T3QREF"]=self.maTableGroupeTailles.getVal(idMaillage,versionRefId,groupeId,'Q3')
                 dico["DIFAT3Q"],dico["DIFRT3Q"],dico["WT3Q"]=self.calculDiffCPU(dico["T3Q"],dico["T3QREF"],monSeuilTaille)
                 monDocument.CreeGroupeTaille(dico)
    
    
    


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
         #print nb,nbRef,seuil
         diff=nb-nbRef
         diffRel=((nb-nbRef)*100)/(nbRef*1.00)
         if diffRel > seuil or (-1*diffRel) > seuil  :
            warning="difference superieure a "+ str(seuil) + "%"
         else :
            warning=""
         diffRelStr=str(diffRel)+"%"
         return diff,diffRelStr,warning
         
