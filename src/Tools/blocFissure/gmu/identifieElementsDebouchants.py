# -*- coding: utf-8 -*-

import logging
import math

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import traceback
from fissError import fissError

from produitMixte import produitMixte
from whichSide import whichSide
  
def identifieElementsDebouchants(ifil, facesDefaut, partitionPeauFissFond,
                                edgesFondIn, edgesFondFiss, wireFondFiss,
                                aretesVivesC, fillingFaceExterne,
                                edgesPipeIn, verticesPipePeau, rayonPipe,
                                facesInside, facesOnside):
  """
  elements débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)
  """
  
  logging.info('start')
 
  verticesEdgesFondIn = [] # les points du fond de fissure au débouché du pipe sur la peau (indice de edgesFondIn)
  pipexts = []             # les segments de pipe associés au points de fond de fissure débouchants (même indice)
  cercles = []             # les cercles de generation des pipes débouchant (même indice)
  facesFissExt = []        # les faces de la fissure externe associés au points de fond de fissure débouchants (même indice)
  edgesFissExtPeau = []    # edges des faces de fissure externe sur la peau (même indice)
  edgesFissExtPipe = []    # edges des faces de fissure externe sur le pipe (même indice)

  #logging.debug("edgesFondIn %s", edgesFondIn)
  for iedf, edge in enumerate(edgesFondIn):
    name = "edgeFondIn%d"%iedf
    geomPublishInFather(initLog.debug, partitionPeauFissFond, edge, name)
    dist = [ geompy.MinDistance(pt, edge) for pt in verticesPipePeau]
    ptPeau = verticesPipePeau[dist.index(min(dist))] # le point de verticesPipePeau a distance minimale de l'edge
    [u, PointOnEdge, EdgeInWireIndex]  = geompy.MakeProjectionOnWire(ptPeau, wireFondFiss)
    logging.debug("u:%s, EdgeInWireIndex: %s, len(edgesFondFiss): %s", u, EdgeInWireIndex, len(edgesFondFiss))
    localEdgeInFondFiss = edgesFondFiss[EdgeInWireIndex]
    centre = PointOnEdge
    centre2 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, u)
    geomPublishInFather(initLog.debug, partitionPeauFissFond, centre2, "centre2_%d"%iedf)
    verticesEdgesFondIn.append(centre)
    name = "verticeEdgesFondIn%d"%iedf
    geomPublishInFather(initLog.debug, partitionPeauFissFond, centre, name)
    norm = geompy.MakeTangentOnCurve(localEdgeInFondFiss, u)
    geomPublishInFather(initLog.debug, partitionPeauFissFond, centre, "norm%d"%iedf)
    cercle = geompy.MakeCircle(centre, norm, rayonPipe)
    geomPublishInFather(initLog.debug, partitionPeauFissFond, cercle, "cerclorig%d"%iedf)
    [vertex] = geompy.ExtractShapes(cercle, geompy.ShapeType["VERTEX"], False)
    vec1 = geompy.MakeVector(centre, vertex)
    vec2 = geompy.MakeVector(centre, ptPeau)
    angle = geompy.GetAngleRadians(vec1, vec2)
    # cas général : on reconstitue une portion de pipe, avec l'arête de couture qui coincide
    #   avec la face de fissure, au niveau du débouché sur la face externe
    # cas dégénéré : le pipe débouche perpendiculairement à une surface plane à l'origine.
    #   La partition filling / pipe reconstruit échoue.
    #   - Si on partitionne le filling avec un simple pipe obtenu par extrusion droite du cercle,
    #     cela donne un point en trop sur le cercle.
    #   - Si on prend une vraie surface plane (pas un filling), on peut faire la partition avec 
    #     les pipes reconstruits              
    logging.debug("angle=%s", angle)
    #if abs(angle) > 1.e-7:
    sommetAxe = geompy.MakeTranslationVector(centre, norm)
    pm = produitMixte(centre, vertex, ptPeau, sommetAxe)
    if pm > 0:  # ajout de pi a (-)angle pour éviter des points confondus (partition échoue) dans les cas dégénérés
      cercle = geompy.MakeRotation(cercle, norm, angle + math.pi)
    else:
      cercle = geompy.MakeRotation(cercle, norm, -angle + math.pi)
    name = "cercle%d"%iedf
    geomPublishInFather(initLog.debug,partitionPeauFissFond, cercle, name)
    cercles.append(cercle)

    # --- estimation de la longueur du pipe necessaire de part et d'autre du point de sortie
    if aretesVivesC is None:
      faceTestPeau = fillingFaceExterne
    else:
      faceTestPeau = facesDefaut[ifil]
    sideCentre = whichSide(faceTestPeau, centre)
    locPt0 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, 0.0)
    locPt1 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, 1.0)
    sidePt0 = whichSide(faceTestPeau, locPt0)
    sidePt1 = whichSide(faceTestPeau, locPt1)
    logging.debug("position centre cercle: %s, extremité edge u0: %s, u1: %s", sideCentre, sidePt0, sidePt1) 
    normFace = geompy.GetNormal(faceTestPeau, ptPeau)
    inclPipe = abs(geompy.GetAngleRadians(norm, normFace))
    lgp = max(rayonPipe/2., abs(3*rayonPipe*math.tan(inclPipe)))
    logging.debug("angle inclinaison Pipe en sortie: %s degres, lgp: %s", inclPipe*180/math.pi, lgp)
    
    # --- position des points extremite du pipe sur l'edge debouchante
    #     il faut la distance curviligne ofp du point central par rapport à une extrémité de l'edge débouchante
    locEdgePart = geompy.MakePartition([localEdgeInFondFiss],[centre], [], [], geompy.ShapeType["EDGE"], 0, [], 0)
    edgesLoc = geompy.ExtractShapes(locEdgePart, geompy.ShapeType["EDGE"], False)
    edgesLocSorted =[(geompy.MinDistance(edge, locPt0), kk, edge) for kk, edge in enumerate(edgesLoc)]
    edgesLocSorted.sort()
    try:
      ofp = geompy.BasicProperties(edgesLocSorted[0][2])[0] # distance curviligne centre locPt0
    except:
      texte = "Identification des éléments au débouché du pipe sur la face externe impossible. "
      texte += "Cause possible : la ligne de fond de fissure comprend un point géométrique coincidant avec la face externe. "
      texte += "La ligne de fond de fissure doit déboucher franchement de la face externe, et ne doit pas être coupée au niveau de la face."
      raise fissError(traceback.extract_stack(),texte)
    logging.debug("distance curviligne centre extremite0: %s", ofp)
    p1 = geompy.MakeVertexOnCurveByLength(localEdgeInFondFiss, ofp +lgp, locPt0)
    p2 = geompy.MakeVertexOnCurveByLength(localEdgeInFondFiss, ofp -lgp, locPt0)
    geomPublishInFather(initLog.debug, wireFondFiss, p1, "p1_%d"%iedf)
    geomPublishInFather(initLog.debug, wireFondFiss, p2, "p2_%d"%iedf)

    edgePart = geompy.MakePartition([localEdgeInFondFiss], [p1,p2], [], [], geompy.ShapeType["EDGE"], 0, [], 0)
    edps = geompy.ExtractShapes(edgePart, geompy.ShapeType["EDGE"], True)
    for edp in edps:
      if geompy.MinDistance(centre, edp) < 1.e-3:
        pipext = geompy.MakePipe(cercle, edp)
        name = "pipeExt%d"%iedf
        geomPublishInFather(initLog.debug, partitionPeauFissFond, pipext, name)
        pipexts.append(pipext)

    for ifa, face in enumerate(facesInside):
      logging.debug("recherche edges communes entre une face inside et (faces onside, edges pipe et fond débouchante)")
      edgesPeauFis = []
      edgesPipeFis = []
      edgesPipeFnd = []
      try:
        edgesPeauFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(facesOnside), face], geompy.ShapeType["EDGE"])
        logging.debug("    faces onside %s",edgesPeauFis)
        edgesPipeFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesPipeIn), face], geompy.ShapeType["EDGE"])
        logging.debug("    edgesPipeIn %s", edgesPipeFis)
        edgesPipeFnd = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesFondIn), face], geompy.ShapeType["EDGE"])
        logging.debug("    edgesFondIn %s ", edgesPipeFnd)
      except:
        logging.debug("  pb edges communes %s %s %s",edgesPeauFis, edgesPipeFis, edgesPipeFnd)
        pass
      if (len(edgesPeauFis) > 0) and (len(edgesPipeFis) > 0) and (len(edgesPipeFnd) == 0):
        dist = geompy.MinDistance(geompy.MakeCompound(edgesPeauFis), ptPeau)
        logging.debug("    test distance extrémité reference %s", dist)
        if dist < 1.e-3: # c'est la face de fissure externe associée
          logging.debug("    face %s inside ajoutée", ifa)
          facesFissExt.append(face)
          name="faceFissExt%d"%iedf
          geomPublishInFather(initLog.debug, partitionPeauFissFond, face, name)
          dist = 1.
          for ipe, edpe in enumerate(edgesPeauFis):
            for ipi, edpi in enumerate(edgesPipeFis):
              dist = geompy.MinDistance(edpe, edpi)
              if dist < 1.e-3:
                edgesFissExtPeau.append(edpe)
                name="edgesFissExtPeau%d"%iedf
                geomPublishInFather(initLog.debug, partitionPeauFissFond, edpe, name)
                edgesFissExtPipe.append(edpi)
                name="edgesFissExtPipe%d"%iedf
                geomPublishInFather(initLog.debug, partitionPeauFissFond, edpi, name)
                break
            if dist < 1.e-3:
              break

  return (verticesEdgesFondIn, pipexts, cercles, facesFissExt, edgesFissExtPeau, edgesFissExtPipe)