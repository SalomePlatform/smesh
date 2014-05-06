# -*- coding: utf-8 -*-

import os
import logging

# -----------------------------------------------------------------------------
# --- satistiques maillage

def getGroupesRef(fichierMed):
      fichier=fichierMed.replace('.med','_groupesRef.res')
      try:
         text=open(fichier).read()
      except:
         return []
      liste=text.split(",")
      return liste


def getStatsMaillage(maillage,fichierMed):
  """
  """
  logging.info('start')

  OK = False
  fichier=fichierMed.replace('.med','.res')
  if maillage is not None:
    mesures = maillage.GetMeshInfo()
    txt=""
    import SMESH
    for i in range(len(mesures)):
      txt += str(SMESH.EntityType._item(i))+ " " +str(mesures[SMESH.EntityType._item(i)]) + "\n"

    from utiles import writeFile
    writeFile(fichier,txt)


def getStatsGroupes(maillage,fichierMedResult):
  """
  """
  logging.info('start')
  fichierGroupe=fichierMedResult.replace('.med','_groupesRef.res')
  lGroups=getGroupesRef(fichierGroupe)
  if len(lGroups)==0: 
    print "pas de Groupe de Reference "
    try :
      os.remove(fichierGroupe)
      return
    except :
      return
  lGroupsSMESH=maillage.GetGroups()
  for groupe in lGroupsSMESH :
     groupeName= groupe.GetName()
     if groupeName not in lGroups : continue
     extension='_'+groupeName+'.res'
     fichierStatGroupe=fichierGroupe.replace('_groupesRef.res',extension)
     getStatsStatSurGroupes(maillage,groupe,fichierStatGroupe)


def getStatsStatSurGroupes(maillage,groupe,fichierStatGroupe):
  mesures = maillage.GetMeshInfo(groupe)
  txt=""
  import SMESH
  for i in range(len(mesures)):
      txt += str(SMESH.EntityType._item(i))+ " " +str(mesures[SMESH.EntityType._item(i)]) + "\n"
  from utiles import writeFile
  writeFile(fichierStatGroupe,txt)



