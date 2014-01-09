# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- recherche et classement des edges du tore par propagate

def propagateTore(tore):
  """
  Classement des edges du tore par une operation 'propagate'
  @param tore partionné et coupé
  @return (diams, circles, geners) edges dans le plan de fissure, edges demi circulaires,
  edges selon la generatrice (liste de compounds)
  """
  logging.info("start")

  lencomp = []
  compounds = geompy.Propagate(tore)
  for i in range(len(compounds)):
    #geompy.addToStudyInFather( tore, compounds[i], 'edges' )
    props = geompy.BasicProperties(compounds[i])
    lencomp.append(props[0])
    pass
  minlen = min(lencomp)
  maxlen = max(lencomp)
  diams = []
  geners = []
  circles = []
  for i in range(len(lencomp)):
    if (lencomp[i]- minlen)/minlen < 0.01 :
      diams.append(compounds[i])
    elif (maxlen - lencomp[i])/lencomp[i] < 0.2 :
      geners.append(compounds[i])
    else:
      circles.append(compounds[i])

  geompy.addToStudyInFather( tore, diams[0], 'diams0' )
  geompy.addToStudyInFather( tore, diams[1], 'diams1' )
  geompy.addToStudyInFather( tore, circles[0], 'circles0' )
  geompy.addToStudyInFather( tore, circles[1], 'circles1' )
  geompy.addToStudyInFather( tore, geners[0], 'geners' )

  return diams, circles, geners
