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
import subprocess
import time

class Job:
  def __init__(self,listeParamMaillage,salomePath,versionId,mesGroupesRef):    
      self.id=listeParamMaillage[0]
      self.script=str(listeParamMaillage[1])
      self.fichierMedResult=str(listeParamMaillage[2])
      self.fichierStatResult=str(listeParamMaillage[2]).replace('.med','.res')
      self.fichierGroupe=self.fichierMedResult.replace('.med','_groupesRef.res')
      self.salomePath=salomePath
      self.versionId=versionId
      self.mesGroupesRef=mesGroupesRef
      self.CPU=3 # Quand on n appelle pas execute
      self.fichiersADetruire=[self.fichierMedResult,self.fichierStatResult,self.fichierGroupe]

      
  def execute(self):
      home=os.path.expanduser("~")
      commande=os.path.abspath(os.path.join(home,self.salomePath,"salome"))
      script= os.path.abspath(os.path.join(os.path.abspath(__file__),'../..',self.script)) 

      debut=time.time()
      #a=os.system(commande+" -t "+script+" > /tmp/Exec 2>&1")
      a=os.system(commande+" -t "+script)
      fin=time.time()
      self.CPU=fin-debut
      print("  Temps d execution : ",  self.CPU)

      #stdout, stderr = p.communicate() 

     
  def getStatSurMailles(self):
      try:
         text=open(self.fichierStatResult).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(self.fichierStatResult))
         exit(1)
      liste=text.split()
      i=0
      listeColonnes=[]
      listeValues=[]
      while i < len(liste) :
         listeColonnes.append(liste[i])
         listeValues.append(liste[i+1])
         i=i+2
      return listeColonnes,listeValues


  def getStatSurRatiosGroupes(self,groupe):
      extension="_"+groupe+'_Ratio.res'
      fichier=self.fichierMedResult.replace('.med',extension)
      self.fichiersADetruire.append(fichier)
      try:
         text=open(fichier).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(fichier))
         exit(1)
      liste=text.split(",")
      return liste

  def getStatSurTaillesGroupes(self,groupe):
      extension="_"+groupe+'_Taille.res'
      fichier=self.fichierMedResult.replace('.med',extension)
      self.fichiersADetruire.append(fichier)
      try:
         text=open(fichier).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(fichier))
         exit(1)
      liste=text.split(",")
      return liste


  def getStatSurGroupes(self,groupe):
      extension="_"+groupe+'.res'
      fichier=self.fichierMedResult.replace('.med',extension)
      self.fichiersADetruire.append(fichier)
      try:
         text=open(fichier).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(fichier))
         exit(1)
      liste=text.split()
      i=0
      listeColonnes=[]
      listeValues=[]
      while i < len(liste) :
         listeColonnes.append(liste[i])
         listeValues.append(liste[i+1])
         i=i+2
      return listeColonnes,listeValues

  def getStatSurTailles(self):
      fichier=self.fichierMedResult.replace('.med','.taille')
      self.fichiersADetruire.append(fichier)
      try:
         text=open(fichier).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(fichier))
         exit(1)
      liste=text.split(",")
#      print "taille",liste
      return liste

  def getStatSurRatios(self):
      fichier=self.fichierMedResult.replace('.med','.ratio')
      self.fichiersADetruire.append(fichier)
      try:
         text=open(fichier).read()
      except:
         print("Impossible d'ouvrir le fichier: ", str(fichier))
         exit(1)
      liste=text.split(",")
      return liste

  def getCPU(self):
      return self.CPU


  def menage(self):
      for fichier in self.fichiersADetruire:
        try :
          os.remove(fichier)
        except :
          pass
 
