# -*- coding: utf-8 -*-
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

    from .utiles import writeFile
    writeFile(fichier,txt)


def getStatsGroupes(maillage,fichierMedResult):
  """
  """
  logging.info('start')
  fichierGroupe=fichierMedResult.replace('.med','_groupesRef.res')
  lGroups=getGroupesRef(fichierGroupe)
  if len(lGroups)==0: 
    print("pas de Groupe de Reference ")
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
  from .utiles import writeFile
  writeFile(fichierStatGroupe,txt)



