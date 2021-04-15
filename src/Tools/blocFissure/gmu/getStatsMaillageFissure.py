# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
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
"""Statistiques maillage"""

import os
import logging

def getStatsMaillageFissure(maillage, referencesMaillageFissure, maillageFissureParams):
  """"Statistiques maillage"""

  logging.debug('start')

  if 'nomRep' in maillageFissureParams:
    nomRep = maillageFissureParams['nomRep']
  else:
    nomRep = os.path.curdir

  nomFicFissure     = maillageFissureParams['nomFicFissure']
  fichierStatMaillageFissure = os.path.join(nomRep, "{}.res".format(nomFicFissure))
  fichierNewRef = os.path.join(nomRep, "{}.new".format(nomFicFissure))
  logging.debug("fichierStatMaillageFissure=%s", fichierStatMaillageFissure)

  ok_maillage = False
  if maillage is not None:
    mesures = maillage.GetMeshInfo()
    d_resu = dict()
    for key, value in mesures.items():
      logging.debug( "key: %s value: %s", key, value)
      d_resu[str(key)] = value
    logging.debug("dico mesures %s", d_resu)

    text_2 = ""
    ok_maillage = True
    with open(fichierStatMaillageFissure, "w") as fic_stat :

      # Le nombre d'arêtes, de quadrangles ou d'hexaèdres doit être rigoureusement identique
      for key in ('Entity_Quad_Edge', 'Entity_Quad_Quadrangle', 'Entity_Quad_Hexa'):
        if d_resu[key] != referencesMaillageFissure[key]:
          text = "Ecart"
          ok_maillage = False
        else:
          text = "Valeur_OK"
        text += ": {} reference: {} calcul: {}".format(key,referencesMaillageFissure[key],d_resu[key])
        logging.info(text)
        fic_stat.write(text+"\n")
        text_2 += "                                          {} = {}, \\\n".format(key,d_resu[key])

      # Le nombre de noeuds, de triangles, de tétraèdres ou de pyramides peut varier du fait des algorithmes. On tolère 5% d'écart.
      tolerance = 0.05
      for key in ('Entity_Node', 'Entity_Quad_Triangle', 'Entity_Quad_Tetra', 'Entity_Quad_Pyramid', 'Entity_Quad_Penta'):
        if d_resu[key] == referencesMaillageFissure[key]:
          text = "Valeur_OK"
        elif (d_resu[key] < (1.0 - tolerance)*referencesMaillageFissure[key]) \
          or (d_resu[key] > (1.0 + tolerance)*referencesMaillageFissure[key]):
          text = "Ecart de plus de {}%".format(tolerance*100.)
          ok_maillage = False
        else:
          text = "Valeur_OK à moins de {}%".format(tolerance*100.)
        text += ": {} reference: {} calcul: {}".format(key,referencesMaillageFissure[key],d_resu[key])
        logging.info(text)
        fic_stat.write(text+"\n")
        text_2 += "                                          {} = {}, \\\n".format(key,d_resu[key])

    if ok_maillage:
      text = "Calcul cohérent avec la référence."
    else:
      text = "Calcul différent de la référence.\n"
      text += "Voir le fichier {}\n".format(fichierStatMaillageFissure)
      text += "La nouvelle référence est disponible dans le fichier :\n{}\n".format(fichierNewRef)
      text += "Il faut l'insérer pour créer le dictionnaire 'referencesMaillageFissure' dans le paramétrage du cas."
#     Résultats de référence pour intégration dans le python du cas pour une mise à jour
      with open(fichierNewRef, "w") as fic_info :
        fic_info.write(text_2[:-4]+" \\")

    print (text)

  return ok_maillage
