import os
import subprocess
import time

class Ref:
  def __init__(self,maBase,idMaillage,idVersion,machine):    
      self.maBase=maBase
      self.idMaillage=idMaillage
      self.idVersion=idVersion
      self.machine=machine
      self.idVersionRef=self.maBase.maTableVersions.getVersionRef()

      self.existe=True
      if self.maBase.maTablePerfs.getVal(self.idMaillage,self.idVersionRef,self.machine) == None:
         self.existe=False



  def verifieCpu(self,NbSec):
      cpuAvant=self.maBase.maTablePerfs.getVal(self.idMaillage,self.idVersionRef,self.machine)
      seuil=self.maBase.maTableMaillages.getVal(self.idMaillage,"seuilCPU")
      seuilHaut=cpuAvant*(100+seuil)/100.
      if NbSec > seuilHaut  :
         print "Probleme consommation CPU : "
         print "         cpu reference : ", cpuAvant
         print "         seuil         : ", seuil
         print "         CPU           : ", NbSec
         return True
      return False
      
  def verifieTailles(self,listeValeurs):
      i=0
      seuil=self.maBase.maTableMaillages.getVal(self.idMaillage,"seuilTaille")
      for nomColonne in ('TailleMax','TailleMin','Quartile1','Mediane','Quartile3','Moyenne'):
          valTrouvee=float(listeValeurs[i])
          valAvant=float(self.maBase.maTableTailles.getVal(self.idMaillage,self.idVersionRef,self.machine,nomColonne))
          seuilHaut=valAvant*(100+seuil)/100.
          seuilBas=valAvant*(100-seuil)/100.
          if (valTrouvee < seuilBas) or (valTrouvee > seuilHaut) :
             print "Probleme sur le nombre de Mailles de type : ", nomColonne
             print "         nb reference : ", valAvant
             print "         seuil        : ", seuil
             print "         nb           : ", valTrouvee
             return True
          i=i+1
      return False

  def verifieRatios(self,listeValeurs):
      i=0
      seuil=self.maBase.maTableMaillages.getVal(self.idMaillage,"seuilRatio")
      for nomColonne in ('TailleMax','TailleMin','Quartile1','Mediane','Quartile3','Moyenne'):
          valTrouvee=float(listeValeurs[i])
          valAvant=float(self.maBase.maTableRatios.getVal(self.idMaillage,self.idVersionRef,self.machine,nomColonne))
          seuilHaut=valAvant*(100+seuil)/100.
          seuilBas=valAvant*(100-seuil)/100.
          if (valTrouvee < seuilBas) or (valTrouvee > seuilHaut) :
             print "Probleme sur le nombre de Mailles de type : ", nomColonne
             print "         nb reference : ", valAvant
             print "         seuil        : ", seuil
             print "         nb           : ", valTrouvee
             return True
          i=i+1
      return False

      

  def verifieMailles(self,listeValeurs,listeEntity):
      seuil=self.maBase.maTableMaillages.getVal(self.idMaillage,"seuilNbMaille")
      i=0
      for nomColonne in listeEntity :
          valTrouvee=int(listeValeurs[i])
          i=i+1
          valAvant=self.maBase.maTableMailles.getVal(self.idMaillage,self.idVersionRef,self.machine,nomColonne)
          #print nomColonne, " ",valTrouvee, " ",valAvant 
          seuilHaut=valAvant*(100+seuil)/100.
          seuilBas=valAvant*(100-seuil)/100.
          if (valTrouvee < seuilBas) or (valTrouvee > seuilHaut) :
             print "Probleme sur le nombre de Mailles de type : ", nomColonne
             print "         nb reference : ", valAvant
             print "         seuil        : ", seuil
             print "         nb           : ", valTrouvee
             return True
      return False

  def verifieMaillesPourGroupes(self,nomGroupe,listeValeurs,listeEntity):
      seuil=self.maBase.maTableMaillages.getVal(self.idMaillage,"seuilNbMaille")
      i=0
      #print nomGroupe,self.idMaillage,self.idVersion,self.machine
      for nomColonne in listeEntity:
          valTrouvee=int(listeValeurs[i])
          i=i+1
          valAvant=self.maBase.maTableGroupes.getVal(nomGroupe,self.idMaillage,self.idVersionRef,self.machine,nomColonne)
          #print nomColonne, " ",valTrouvee, " ",valAvant 
          seuilHaut=valAvant*(100+seuil)/100
          seuilBas=valAvant*(100-seuil)/100
          if (valTrouvee < seuilBas) or (valTrouvee > seuilHaut) :
             print "Probleme sur le nombre de Mailles de type : ", nomColonne, "pour le groupe ", nomGroupe
             print "         nb reference : ", valAvant
             print "         seuil        : ", seuil
             print "         nb           : ", valTrouvee
             return True
      return False

