#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys,os
import salome
from getStats import getGroupesRef
from Type_Maille import dicoDimENtite

def getCritere(dim,NomMesh,acritere,theStudy):
  import SMESH
  from salome.smesh import smeshBuilder
  smesh = smeshBuilder.New(theStudy)
  import numpy
#  print dim,NomMesh,acritere
  if dim == 2 :
    NumElt=NomMesh.GetElementsByType(SMESH.FACE)
  if dim == 3 :
    NumElt=NomMesh.GetElementsByType(SMESH.VOLUME)
  
  critere=[]
  if acritere =="Ratio" :
    for i in range(len(NumElt)):
      critere.append (NomMesh.GetAspectRatio(NumElt[i]))

  if acritere =="Length" :
    for i in range(len(NumElt)):
      critere.append (NomMesh.GetMaxElementLength(NumElt[i]))
      
  crit= numpy.array(critere)
  max=crit.max()
  min=crit.min()
  moyenne=crit.mean()
  M=numpy.median(crit)
  Q1=numpy.percentile(crit,25)
  Q3=numpy.percentile(crit,75)
  
  return [max,min,Q1,M,Q3,moyenne]

def getCritereGroupe(NomMesh,NomGr,acritere,theStudy):
  import SMESH
  from salome.smesh import smeshBuilder
  smesh = smeshBuilder.New(theStudy)
  import numpy
    
  # on ne traite que les mailles 2D et 3D
  NumElt = NomGr.GetListOfID()
  type_gr=NomMesh.GetElementGeomType(NumElt[0])

#  print "critere",acritere
#  print "type groupe",type_gr
#  print dicoDimENtite[str(type_gr)]

  if dicoDimENtite[str(type_gr)]==0 :
     return [0,0,0,0,0,0]

  critere=[]
  if acritere =="Ratio" :
    for i in range(len(NumElt)):
      critere.append (NomMesh.GetAspectRatio(NumElt[i]))

  if acritere =="Length" :
    for i in range(len(NumElt)):
      critere.append (NomMesh.GetMaxElementLength(NumElt[i]))
      
  crit= numpy.array(critere)
  max=crit.max()
  min=crit.min()
  moyenne=crit.mean()
  M=numpy.median(crit)
  Q1=numpy.percentile(crit,25)
  Q3=numpy.percentile(crit,75)
  
  return [max,min,Q1,M,Q3,moyenne]

def getObjectsGroupe(Mesh,liste,theStudy):
  import SMESH
  from salome.smesh import smeshBuilder
  dico={}
  lGroups=Mesh.GetGroups()
  for g in lGroups :
      name = g.GetName() 
      for n in liste :
          if name == n :  dico[name]=g
  return dico

def getStatsCritere(dim,Mesh,fichierMedResult,theStudy):
  fichierStatRatio=fichierMedResult.replace('.med','.ratio')
  max,min,Q1,M,Q3,moyenne = getCritere(dim,Mesh,"Ratio",theStudy)
  f = open(fichierStatRatio, 'w')
  f.write(str(max)+","+str(min)+","+str(Q1)+","+str(M)+","+str(Q3)+","+str(moyenne))
  f.close()

  fichierStatRatio=fichierMedResult.replace('.med','.taille')
  max,min,Q1,M,Q3,moyenne = getCritere(dim,Mesh,"Length",theStudy)
  f = open(fichierStatRatio, 'w')
  f.write(str(max)+","+str(min)+","+str(Q1)+","+str(M)+","+str(Q3)+","+str(moyenne))
  f.close()

  liste=getGroupesRef(fichierMedResult)
  dicoGroupe=getObjectsGroupe(Mesh,liste,theStudy)
  for groupe in liste :
      max,min,Q1,M,Q3,moyenne=getCritereGroupe(Mesh,dicoGroupe[groupe],"Ratio",theStudy)
      extension="_"+groupe+'_Ratio.res'
      fichier=fichierMedResult.replace('.med',extension)
      f = open(fichier, 'w')
      f.write(str(max)+","+str(min)+","+str(Q1)+","+str(M)+","+str(Q3)+","+str(moyenne))
      f.close()
      max,min,Q1,M,Q3,moyenne=getCritereGroupe(Mesh,dicoGroupe[groupe],"Length",theStudy)
      extension="_"+groupe+'_Taille.res'
      fichier=fichierMedResult.replace('.med',extension)
      f = open(fichier, 'w')
      f.write(str(max)+","+str(min)+","+str(Q1)+","+str(M)+","+str(Q3)+","+str(moyenne))
      f.close()

