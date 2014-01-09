# -*- coding: utf-8 -*-

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
  if maillageFissureParams.has_key('nomRep'):
    nomRep = maillageFissureParams['nomRep']
  
  nomFicFissure     = maillageFissureParams['nomFicFissure']
  fichierStatMaillageFissure = nomRep + '/' + nomFicFissure + '.res'
  fichierNewRef = nomRep + '/' + nomFicFissure + '.new'
  logging.debug("fichierStatMaillageFissure=%s", fichierStatMaillageFissure)

  OK = False
  if maillage is not None:
    mesures = maillage.GetMeshInfo()
    d= {}
    for key, value in mesures.iteritems():
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
