# -*- coding: utf-8 -*-
# Copyright (C) 2014-2019  CEA/DEN, EDF R&D
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

import logging
import SMESH

# -----------------------------------------------------------------------------
# --- statistiques maillage

def getStatsMaillageFissure(maillage, referencesMaillageFissure, maillageFissureParams):
  """
  TODO: a completer
  """
  logging.debug('start')

  nomRep = '.'
  if 'nomRep' in maillageFissureParams:
    nomRep = maillageFissureParams['nomRep']
  
  nomFicFissure     = maillageFissureParams['nomFicFissure']
  fichierStatMaillageFissure = nomRep + '/' + nomFicFissure + '.res'
  fichierNewRef = nomRep + '/' + nomFicFissure + '.new'
  logging.debug("fichierStatMaillageFissure=%s", fichierStatMaillageFissure)

  OK = False
  if maillage is not None:
    mesures = maillage.GetMeshInfo()
    d= {}
    for key, value in mesures.items():
      logging.debug( "key: %s value: %s", key, value)
      d[str(key)] = value
    logging.debug("dico mesures %s", d)      

    f = open(fichierStatMaillageFissure, 'w')
    f2 = open(fichierNewRef, 'w')
    OK = True
    for key in ('Entity_Quad_Pyramid', 'Entity_Quad_Hexa', 'Entity_Quad_Quadrangle'):
      if d[key] != referencesMaillageFissure[key]:
        logging.info("Ecart: %s reference: %s calcul: %s", key, referencesMaillageFissure[key], d[key])
        f.write("Ecart: " + key + " reference: " + str(referencesMaillageFissure[key]) + " calcul: " + str(d[key]) + '\n')
        OK = False
      else:
        logging.info("Valeur_OK: %s reference: %s calcul: %s", key, referencesMaillageFissure[key], d[key])
        f.write("Valeur_OK: " + key + " reference: " + str(referencesMaillageFissure[key]) + " calcul: " + str(d[key]) + '\n')
      f2.write(key + " = " + str(d[key]) + ",\n")
    tolerance = 0.05
    for key in ('Entity_Quad_Penta', 'Entity_Quad_Tetra', 'Entity_Quad_Triangle', 'Entity_Quad_Edge', 'Entity_Node'):
      if (d[key] < (1.0 - tolerance)*referencesMaillageFissure[key]) \
      or (d[key] > (1.0 + tolerance)*referencesMaillageFissure[key]):
        logging.info("Ecart: %s reference: %s calcul: %s", key, referencesMaillageFissure[key], d[key])
        f.write("Ecart: " + key + " reference: " + str(referencesMaillageFissure[key]) + " calcul: " + str(d[key]) + '\n')
        OK = False
      else:
        logging.info("Valeur_OK: %s reference: %s calcul: %s", key, referencesMaillageFissure[key], d[key])
        f.write("Valeur_OK: " + key + " reference: " + str(referencesMaillageFissure[key]) + " calcul: " + str(d[key]) + '\n')
      f2.write(key + " = " + str(d[key]) + ",\n")
    f.close()
    f2.close()
  return OK
