# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import GEOM
import math
import numpy as np

def mydot(a):
  return np.dot(a,a)

# -----------------------------------------------------------------------------
# --- groupe de quadrangles de face transformé en face géométrique par filling

def quadranglesToShapeNoCorner(meshQuad, shapeFissureParams, centreFondFiss):
  """
  groupe de quadrangles de face transformée en faces géométriques par filling
  on part de quadrangles définissant une zone a 4 cotés (convexe), et on reconstitue n lignes de p points.
  Ces n lignes de p points sont transformées en n courbes géométriques,
  à partir desquelles on reconstitue une surface géométrique.
  Il peut y avoir plusieurs faces géométriques reconstituées, si on fournit des groupes de quadrangles non connexes.
  On détecte les angles vifs, pour conserver des arêtes vives délimitant des faces connexes.
  @param meshQuad : maillages constitué de quadrangles constituant une ou plusieurs zones convexes
  @return (fillings, noeuds_Bords) : liste de geomObject, listes des bords (bord = liste ordonnée de noeuds (geomObject))
  """
  logging.info("start")

  isVecteurDefaut = False
  if shapeFissureParams.has_key('vecteurDefaut'):
    isVecteurDefaut = True
    vecteurDefaut = shapeFissureParams['vecteurDefaut']

  fillings = []       # les faces reconstituées, découpées selon les arêtes vives
  noeuds_bords = []   #
  bords_Partages = [] # contient a la fin les courbes correspondant aux arêtes vives
  fillconts = []      # les faces reconstituées, sans découpage selon les arêtes vives
  idFilToCont = []    # index face découpée vers face sans découpe
  iface = 0           # index face découpée
  icont = 0           # index face continue
  
  allNodeIds = meshQuad.GetNodesId()
  while len(allNodeIds):
    logging.debug("len(allNodeIds): %s ", len(allNodeIds))
    nodeIds = allNodeIds
    for idNode in nodeIds: # rechercher un coin
      elems = meshQuad.GetNodeInverseElements(idNode)
      if len(elems) == 1:
        # un coin: un noeud, un element quadrangle
        elem = elems[0]
        break;
    idStart = idNode # le noeud de coin
    elemStart = elem # l'élément quadrangle au coin
    xyz = meshQuad.GetNodeXYZ(idStart)
    logging.debug("idStart %s, coords %s", idStart, str(xyz))
  
    nodelines =[] # on va constituer une liste de lignes de points
    nextLine = True
    ligneFinale = False
    while nextLine:
      logging.debug("--- une ligne")
      idNode = idStart
      elem = elemStart
      if ligneFinale:
        agauche = False      # sens de parcours des 4 noeuds d'un quadrangle
        nextLine = False
      else:
        agauche = True
      ligneIncomplete = True # on commence une ligne de points
      debutLigne = True
      nodeline = []
      elemline = []
      while ligneIncomplete: # compléter la ligne de points
        nodeline.append(idNode)
        allNodeIds.remove(idNode)
        elemline.append(elem)
        nodes = meshQuad.GetElemNodes(elem)
        i = nodes.index(idNode) # repérer l'index du noeud courant (i) dans l'élément quadrangle (0 a 3)
        if agauche:             # déterminer le noeud suivant (j) et celui opposé (k) dans le quadrangle
          if i < 3:
            j = i+1
          else:
            j = 0
          if j < 3:
            k = j+1
          else:
            k = 0
        else:
          if i > 0:
            j = i -1
          else:
            j = 3
          if j > 0:
            k = j -1
          else:
            k = 3
        isuiv = nodes[j]   #noeud suivant
        iapres = nodes[k]  #noeud opposé
        if debutLigne:
          debutLigne = False
          # précédent a trouver, dernière ligne : précédent au lieu de suivant
          if agauche:
            if i > 0:
              iprec = nodes[i -1]
            else:
              iprec = nodes[3]
            idStart = iprec
            elems3 = meshQuad.GetNodeInverseElements(iprec)
            if len(elems3) == 1: # autre coin
              ligneFinale = True
            else:
              for elem3 in elems3:
                if elem3 != elem:
                  elemStart = elem3
                  break
        #print nodes, idNode, isuiv, iapres
        elems1 = meshQuad.GetNodeInverseElements(isuiv)
        elems2 = meshQuad.GetNodeInverseElements(iapres)
        ligneIncomplete = False
        for elem2 in elems2:
          if elems1.count(elem2) and elem2 != elem:
            ligneIncomplete = True
            idNode = isuiv
            elem = elem2
            break
        if not  ligneIncomplete:
          nodeline.append(isuiv)
          allNodeIds.remove(isuiv)
      logging.debug("nodeline %s", nodeline)
      logging.debug("elemline %s", elemline)
      nodelines.append(nodeline)
    logging.debug("nodelines = %s", nodelines)
    longueur = [len(val) for val in nodelines]
    logging.debug("longueur = %s", longueur)
    # on a constitué une liste de lignes de points connexes
    logging.debug("dimensions [%s, %s]", len(nodelines),  len(nodeline))   
    
    # stockage des coordonnées dans un tableau numpy
    mat = np.zeros((len(nodelines), len(nodeline), 3))
    for i, ligne in enumerate(nodelines):
      for j, nodeId in enumerate(ligne):
        mat[i,j] = meshQuad.GetNodeXYZ(nodeId)
    logging.debug("matrice de coordonnées: \n%s",mat)
    logging.debug("dimensions %s", mat.shape)
    
    # recherche d'angles supérieurs a un seuil sur une ligne : angle entre deux vecteurs successifs
    cosmin = math.cos(math.pi/4.)          # TODO: angle reference en paramètre
    vecx = mat[:, 1:,  :] - mat[:, :-1, :] # vecteurs selon direction "x"
    vx0 = vecx[:, :-1, :]                  # vecteurs amont
    vx1 = vecx[:, 1:,  :]                  # vecteurs aval
    e = np.einsum('ijk,ijk->ij', vx0, vx1) # produit scalaire des vecteurs
    f = np.apply_along_axis(mydot, 2, vx0) # normes carrées vecteurs amont
    g = np.apply_along_axis(mydot, 2, vx1) # normes carrées vecteurs aval
    h = e/(np.sqrt(f*g))                   # cosinus
    ruptureX = h < cosmin                  # True si angle > reference
    logging.debug("matrice de rupture X: \n%s",ruptureX)
    rupX = filter(lambda x: np.prod(ruptureX[:,x]), range(len(nodeline)-2))
    logging.debug("colonnes de rupture: %s",rupX)
    # recherche d'angles supérieurs a un seuil sur une colonne : angle entre deux vecteurs successifs
    vecy = mat[ 1:, :, :] - mat[:-1, :, :] # vecteurs selon direction "y"
    vy0 = vecy[:-1, :, :]                  # vecteurs amont
    vy1 = vecy[ 1:, :, :]                  # vecteurs aval
    e = np.einsum('ijk,ijk->ij', vy0, vy1) # produit scalaire des vecteurs
    f = np.apply_along_axis(mydot, 2, vy0) # normes carrées vecteurs amont
    g = np.apply_along_axis(mydot, 2, vy1) # normes carrées vecteurs aval
    h = e/(np.sqrt(f*g))                   # cosinus
    ruptureY = h < cosmin                  # True si angle > reference
    logging.debug("matrice de rupture Y: \n%s",ruptureY)
    rupY = filter(lambda x: np.prod(ruptureY[x, :]), range(len(nodelines)-2))
    logging.debug("lignes de rupture: %s",rupY)
    if (len(rupX)*len(rupY)) > 0:
      logging.critical("""Cas non traité: présence d'angles vifs dans 2 directions, 
      lors de la reconstitution des faces géométriques dans la zone remaillée""")
    
    mats = []
    bordsPartages = []
    if (len(rupX)> 0):
      rupX.append(mat.shape[1]-1)
      for i, index in enumerate(rupX):
        imax = index+2
        imin = 0
        if i > 0:
          imin = rupX[i-1] + 1
        mats.append(mat[:, imin:imax, :])
        if imax == mat.shape[1] + 1:
          ifin = 0
        else:
          ifin = imax
        bordsPartages.append([imin,ifin]) # les indices différents de 0 correspondent à des bords partagés
    elif (len(rupY)> 0):
      rupY.append(mat.shape[0]-1)
      for i, index in enumerate(rupY):
        imax = index+2
        imin = 0
        if i > 0:
          imin = rupY[i-1] + 1
        mats.append(mat[imin:imax, :, :])
        if imax == mat.shape[0] + 1:
          ifin = 0
        else:
          ifin = imax
        bordsPartages.append([imin,ifin]) # les indices différents de 0 correspondent à des bords partagés
    else:
      mats.append(mat)
      bordsPartages.append([0,0])         # les indices différents de 0 correspondent à des bords partagés
    
    curvconts = []
    for nmat, amat in enumerate(mats):
      logging.debug("dimensions matrice %s: %s", nmat, amat.shape)
      nbLignes = amat.shape[1] # pas de rupture, ou rupture selon des colonnes: on transpose
      nbCols = amat.shape[0]
      if len(rupY) > 0 :       # rupture selon des lignes: pas de transposition
        nbLignes = amat.shape[0]
        nbCols = amat.shape[1]
      curves = []
      noeudsBords = []
      for i in range(4):
        noeudsBords.append([])
      k = 0
      for i in range(nbLignes):
        nodeList = []
        for j in range(nbCols):
          #logging.debug("point[%s,%s] = (%s, %s, %s)",i,j,amat[i,j,0], amat[i,j,1], amat[i,j,2])
          if len(rupY) > 0 : # pas de transposition
            node = geompy.MakeVertex(amat[i,j,0], amat[i,j,1], amat[i,j,2])
          else:              # transposition
            node = geompy.MakeVertex(amat[j,i,0], amat[j,i,1], amat[j,i,2])
          nodeList.append(node)
          if i == 0:
            noeudsBords[0].append(node)
            #name = "bord0_%d"%k
            #geomPublish(initLog.debug,  node, name )
          if i == (nbLignes -1):
            noeudsBords[2].append(node)
            #name = "bord2_%d"%k
            #geomPublish(initLog.debug,  node, name )
          if j == 0:
            noeudsBords[1].append(node)
            #name = "bord1_%d"%k
            #geomPublish(initLog.debug,  node, name )
          if j == (nbCols -1):
            noeudsBords[3].append(node)
            #name = "bord3_%d"%k
            #geomPublish(initLog.debug,  node, name )
            k += 1
        curve = geompy.MakeInterpol(nodeList, False, False)
        #name = "curve_%d"%i
        #geomPublish(initLog.debug,  curve, name )
        if len(curvconts) == 0 or len(curves) > 0: # éliminer les doublons de la surface sans découpe 
          curvconts.append(nodeList)
        curves.append(curve)
      if bordsPartages[nmat][0] :
        bordsPartages[nmat][0] = curves[0]  # la première ligne est un bord partagé
      else:
        bordsPartages[nmat][0] = None
      if bordsPartages[nmat][1] :
        bordsPartages[nmat][1] = curves[-1] # la dernière ligne est un bord partagé
      else:
        bordsPartages[nmat][1] = None
      filling = geompy.MakeFilling(geompy.MakeCompound(curves), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
      # --- test orientation filling
      vertex = geompy.MakeVertexOnSurface(filling, 0.5, 0.5)
      normal = geompy.GetNormal(filling, vertex)

      if centreFondFiss is not None:
        logging.debug("orientation filling a l'aide du centre de fond de fissure")
        vecteurDefaut = geompy.MakeVector(centreFondFiss, vertex)
        
      if not isVecteurDefaut:
        pointIn_x = 0.0
        pointIn_y = 0.0
        pointIn_z = 0.0
        pointExplicite = False
        if shapeFissureParams.has_key('pointIn_x'):
          pointExplicite = True
          pointIn_x = shapeFissureParams['pointIn_x']
        if shapeFissureParams.has_key('pointIn_y'):
          pointExplicite = True
          pointIn_y = shapeFissureParams['pointIn_y']
        if shapeFissureParams.has_key('pointIn_z'):
          pointExplicite = True
          pointIn_z = shapeFissureParams['pointIn_z']
        if pointExplicite:
          cdg = geompy.MakeVertex(pointIn_x, pointIn_y, pointIn_z)
          logging.debug("orientation filling par point intérieur %s", (pointIn_x, pointIn_y, pointIn_z))
          vecteurDefaut = geompy.MakeVector(cdg, vertex)
        
      if shapeFissureParams.has_key('convexe'):
        isConvexe = shapeFissureParams['convexe']
        logging.debug("orientation filling par indication de convexité %s", isConvexe)
        cdg = geompy.MakeCDG(filling)
        if isConvexe:
          vecteurDefaut = geompy.MakeVector(cdg, vertex)
        else:
          vecteurDefaut = geompy.MakeVector(vertex, cdg)
     
      if vecteurDefaut is not None:
        geomPublish(initLog.debug, normal, "normFillOrig%d"%iface)
        geomPublish(initLog.debug, vecteurDefaut, "fromInterieur%d"%iface)
        if geompy.GetAngleRadians(vecteurDefaut, normal) > math.pi/2.0:
          filling = geompy.ChangeOrientation(filling)
      geomPublish(initLog.debug,  filling, "filling%d"%iface )
      #geompy.ExportBREP(filling, "filling.brep")
      iface = iface+1
      fillings.append(filling)
      noeuds_bords.append(noeudsBords)
      idFilToCont.append(icont)
      bords_Partages += bordsPartages
      logging.debug("bords_Partages = %s", bords_Partages)
      pass # --- loop on mats
    # --- reconstruction des faces continues à partir des listes de noeuds
    #     les courbes doivent suivre la courbure pour éviter les oscillations
    if icont == iface - 1: # pas de découpe, on garde la même face
      fillcont = fillings[-1]
    else:
      nbLignes = len(curvconts[0])
      curves = []
      for i in range(nbLignes):
        nodes = [curvconts[j][i] for j in range(len(curvconts))]
        curve = geompy.MakeInterpol(nodes, False, False)
        curves.append(curve)
      fillcont = geompy.MakeFilling(geompy.MakeCompound(curves), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
    geomPublish(initLog.debug,  fillcont, "filcont%d"%icont )
    fillconts.append(fillcont)
    icont = icont+1   
    pass   # --- loop while there are remaining nodes
  
  return fillings, noeuds_bords, bords_Partages, fillconts, idFilToCont
