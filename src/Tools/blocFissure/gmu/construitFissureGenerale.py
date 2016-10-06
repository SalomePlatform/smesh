# -*- coding: utf-8 -*-

import logging
import salome
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import GEOM
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH
import math
import bisect
import traceback

# from extractionOrientee import extractionOrientee
# from extractionOrienteeMulti import extractionOrienteeMulti
# from sortFaces import sortFaces
#from sortEdges import sortEdges
# from eliminateDoubles import eliminateDoubles
# from substractSubShapes import substractSubShapes
# from produitMixte import produitMixte
# from findWireEndVertices import findWireEndVertices
#from findWireIntermediateVertices import findWireIntermediateVertices
from orderEdgesFromWire import orderEdgesFromWire
# from getSubshapeIds import getSubshapeIds
from putName import putName
# from distance2 import distance2
from enleveDefaut import enleveDefaut
from shapeSurFissure import shapeSurFissure
from regroupeSainEtDefaut import RegroupeSainEtDefaut
from triedreBase import triedreBase
# from checkDecoupePartition import checkDecoupePartition
# from whichSide import whichSide
# from whichSideMulti import whichSideMulti
#from whichSideVertex import whichSideVertex
#from projettePointSurCourbe import projettePointSurCourbe
# from prolongeWire import prolongeWire
from restreintFaceFissure import restreintFaceFissure
from partitionneFissureParPipe import partitionneFissureParPipe
from construitPartitionsPeauFissure import construitPartitionsPeauFissure
from compoundFromList import compoundFromList
from identifieElementsGeometriquesPeau import identifieElementsGeometriquesPeau
from identifieFacesEdgesFissureExterne import identifieFacesEdgesFissureExterne
from calculePointsAxiauxPipe import calculePointsAxiauxPipe
from elimineExtremitesPipe import elimineExtremitesPipe
from construitEdgesRadialesDebouchantes import construitEdgesRadialesDebouchantes
from creePointsPipePeau import creePointsPipePeau
from ajustePointsEdgePipeFissure import ajustePointsEdgePipeFissure
from construitMaillagePipe import construitMaillagePipe
from mailleAretesEtJonction import mailleAretesEtJonction
from mailleFacesFissure import mailleFacesFissure
from mailleFacesPeau import mailleFacesPeau
from fissError import fissError

# -----------------------------------------------------------------------------
# --- procédure complète fissure générale

def construitFissureGenerale(maillagesSains,
                             shapesFissure, shapeFissureParams,
                             maillageFissureParams, elementsDefaut, step=-1):
  """
  TODO: a completer
  """
  logging.info('start')
  
  shapeDefaut       = shapesFissure[0] # faces de fissure, débordant
  fondFiss          = shapesFissure[4] # groupe d'edges de fond de fissure

  rayonPipe = shapeFissureParams['rayonPipe']
  if shapeFissureParams.has_key('lenSegPipe'):
    lenSegPipe = shapeFissureParams['lenSegPipe']
  else:
    lenSegPipe = rayonPipe

  nomRep            = maillageFissureParams['nomRep']
  nomFicSain        = maillageFissureParams['nomFicSain']
  nomFicFissure     = maillageFissureParams['nomFicFissure']

  nbsegRad          = maillageFissureParams['nbsegRad']      # nombre de couches selon un rayon du pipe
  nbsegCercle       = maillageFissureParams['nbsegCercle']   # nombre de secteur dans un cercle du pipe
  areteFaceFissure  = maillageFissureParams['areteFaceFissure']
  lgAretesVives     = 0
  if maillageFissureParams.has_key('aretesVives'):
    lgAretesVives   = maillageFissureParams['aretesVives']

  pointIn_x = 0.0
  pointIn_y = 0.0
  pointIn_z = 0.0
  isPointInterne = False
  if shapeFissureParams.has_key('pointIn_x'):
    pointIn_x = shapeFissureParams['pointIn_x']
    isPointInterne = True
  if shapeFissureParams.has_key('pointIn_y'):
    pointIn_y = shapeFissureParams['pointIn_y']
    isPointInterne = True
  if shapeFissureParams.has_key('pointIn_z'):
    pointIn_z = shapeFissureParams['pointIn_z']
    isPointInterne = True
  if isPointInterne:
    pointInterne = geompy.MakeVertex(pointIn_x, pointIn_y, pointIn_z)
  else:
    pointInterne = None

  #fichierMaillageSain = nomRep + '/' + nomFicSain + '.med'
  fichierMaillageFissure = nomRep + '/' + nomFicFissure + '.med'

  # fillings des faces en peau
  facesDefaut              = elementsDefaut[0]
  #centresDefaut            = elementsDefaut[1]
  #normalsDefaut            = elementsDefaut[2]
  #extrusionsDefaut         = elementsDefaut[3]
  dmoyen                   = elementsDefaut[4]
  bordsPartages            = elementsDefaut[5]
  #fillconts                = elementsDefaut[6]
  #idFilToCont              = elementsDefaut[7]
  maillageSain             = elementsDefaut[8]
  internalBoundary         = elementsDefaut[9]
  zoneDefaut               = elementsDefaut[10]
  zoneDefaut_skin          = elementsDefaut[11]
  zoneDefaut_internalFaces = elementsDefaut[12]
  zoneDefaut_internalEdges = elementsDefaut[13]
  #edgeFondExt              = elementsDefaut[14]
  centreFondFiss           = elementsDefaut[15]
  #tgtCentre                = elementsDefaut[16]
  if lgAretesVives == 0:
     lgAretesVives = dmoyen


  O, OX, OY, OZ = triedreBase()

  # --- restriction de la face de fissure au domaine solide :
  #     partition face fissure étendue par fillings, on garde la face interne
  
  facesPortFissure = restreintFaceFissure(shapeDefaut, facesDefaut, pointInterne)
 
  # --- pipe de fond de fissure, prolongé, partition face fissure par pipe
  #     identification des edges communes pipe et face fissure
  
  (fissPipe, edgesPipeFiss, edgesFondFiss, wirePipeFiss, wireFondFiss) = partitionneFissureParPipe(shapesFissure, elementsDefaut, rayonPipe)
  edgesFondFiss, edgesIdByOrientation = orderEdgesFromWire(wireFondFiss)
  for i,edge in enumerate(edgesFondFiss):
    geomPublishInFather(initLog.debug, wireFondFiss, edge, "edgeFondFiss%d"%i)
  
  # --- peau et face de fissure
  #
  # --- partition peau défaut - face de fissure prolongée - wire de fond de fissure prolongée
  #     il peut y avoir plusieurs faces externes, dont certaines sont découpées par la fissure
  #     liste de faces externes : facesDefaut
  #     liste de partitions face externe - fissure : partitionPeauFissFond (None quand pas d'intersection)

  partitionsPeauFissFond = construitPartitionsPeauFissure(facesDefaut, fissPipe)
    
  # --- arêtes vives détectées (dans quadranglesToShapeNoCorner
  #                             et quadranglesToShapeWithCorner)
    
  aretesVivesC = compoundFromList(bordsPartages, "areteVive")
  aretesVivesCoupees = []  # ensembles des arêtes vives identifiées sur les faces de peau dans l'itération sur partitionsPeauFissFond
   
  # --- inventaire des faces de peau coupées par la fissure
  #     pour chaque face de peau : 0, 1 ou 2 faces débouchante du fond de fissure
  #                                0, 1 ou plus edges de la face de fissure externe au pipe
  
  nbFacesFilling = len(partitionsPeauFissFond)
  
  ptEdgeFond = [ []  for i in range(nbFacesFilling)] # pour chaque face [points edge fond de fissure aux débouchés du pipe]
  fsPipePeau = [ []  for i in range(nbFacesFilling)] # pour chaque face [faces du pipe débouchantes]
  edRadFPiPo = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge radiale des faces du pipe débouchantes ]
  fsFissuExt = [ []  for i in range(nbFacesFilling)] # pour chaque face [faces de fissure externes au pipe]
  edFisExtPe = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge en peau des faces de fissure externes (pas subshape facePeau)]
  edFisExtPi = [ []  for i in range(nbFacesFilling)] # pour chaque face [edge commun au pipe des faces de fissure externes]
  facesPeaux = [None for i in range(nbFacesFilling)] # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
  edCircPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
  ptCircPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape point sur edge circulaire aux débouchés du pipe]
  gpedgeBord = [None for i in range(nbFacesFilling)] # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
  gpedgeVifs = [None for i in range(nbFacesFilling)] # pour chaque face de peau : groupes subshape des edges aux arêtes vives entre fillings
  edFissPeau = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]
  ptFisExtPi = [ []  for i in range(nbFacesFilling)] # pour chaque face de peau : [point commun edFissPeau edCircPeau]
  
  for ifil, partitionPeauFissFond in enumerate(partitionsPeauFissFond):
    if partitionPeauFissFond is not None:
      dataPPFF,aretesVivesCoupees = identifieElementsGeometriquesPeau(ifil, partitionPeauFissFond, edgesPipeFiss,
                                                                      edgesFondFiss, wireFondFiss, aretesVivesC,
                                                                      facesDefaut, centreFondFiss, rayonPipe,
                                                                      aretesVivesCoupees)      
      ptEdgeFond[ifil] = dataPPFF['endsEdgeFond']
      fsPipePeau[ifil] = dataPPFF['facesPipePeau']
      edRadFPiPo[ifil] = dataPPFF['edgeRadFacePipePeau']
      fsFissuExt[ifil] = dataPPFF['facesFissExt']
      edFisExtPe[ifil] = dataPPFF['edgesFissExtPeau']
      edFisExtPi[ifil] = dataPPFF['edgesFissExtPipe']
      facesPeaux[ifil] = dataPPFF['facePeau']
      edCircPeau[ifil] = dataPPFF['edgesCircPeau']
      ptCircPeau[ifil] = dataPPFF['verticesCircPeau']
      gpedgeBord[ifil] = dataPPFF['groupEdgesBordPeau']
      gpedgeVifs[ifil] = dataPPFF['bordsVifs']
      edFissPeau[ifil] = dataPPFF['edgesFissurePeau']
      ptFisExtPi[ifil] = dataPPFF['verticesPipePeau']

  facesPipePeau = []
  edgeRadFacePipePeau = []
  for ifil in range(nbFacesFilling):
    facesPipePeau += fsPipePeau[ifil]
    edgeRadFacePipePeau += edRadFPiPo[ifil]
  
  for i, avc in enumerate(aretesVivesCoupees):
    name = "areteViveCoupee%d"%i
    geomPublish(initLog.debug, avc, name)
  
  # --- identification des faces et edges de fissure externe pour maillage
  
  (faceFissureExterne, edgesPipeFissureExterneC,
    wirePipeFissureExterne, edgesPeauFissureExterneC) = identifieFacesEdgesFissureExterne(fsFissuExt, edFisExtPe,
                                                                                          edFisExtPi, edgesPipeFiss)

  # --- preparation maillage du pipe :
  #     - détections des points a respecter : jonction des edges/faces constituant la face de fissure externe au pipe
  #     - points sur les edges de fond de fissure et edges pipe/face fissure,
  #     - vecteurs tangents au fond de fissure (normal au disque maillé)
  
  (centres, gptsdisks, raydisks) = calculePointsAxiauxPipe(edgesFondFiss, edgesIdByOrientation, facesDefaut, 
                                                           centreFondFiss, wireFondFiss, wirePipeFiss,
                                                           lenSegPipe, rayonPipe, nbsegCercle, nbsegRad)
   
  # --- recherche des points en trop (externes au volume à remailler)
  #     - on associe chaque extrémité du pipe à une face filling 
  #     - on part des disques aux extrémités du pipe
  #     - pour chaque disque, on prend les vertices de géométrie,
  #       on marque leur position relative à la face.
  #     - on s'arrete quand tous les noeuds sont dedans
  
  (idFillingFromBout, idisklim, idiskout) = elimineExtremitesPipe(ptEdgeFond, facesDefaut, centres, gptsdisks, nbsegCercle)

  # --- construction des listes d'edges radiales sur chaque extrémité débouchante
  
  (listEdges, idFacesDebouchantes) = construitEdgesRadialesDebouchantes(idisklim, idiskout, gptsdisks, raydisks,
                                                                        facesPipePeau, edgeRadFacePipePeau, nbsegCercle)
    
  # --- création des points du maillage du pipe sur la face de peau
  
  (gptsdisks, idisklim) = creePointsPipePeau(listEdges, idFacesDebouchantes, idFillingFromBout,
                                             ptEdgeFond, ptFisExtPi, edCircPeau, gptsdisks, idisklim, nbsegRad)
  
  # --- ajustement precis des points sur edgesPipeFissureExterneC
  
  gptsdisks = ajustePointsEdgePipeFissure(edgesPipeFissureExterneC, wirePipeFissureExterne, gptsdisks, idisklim)
    
   # --- maillage effectif du pipe

  (meshPipe, meshPipeGroups, edgesCircPipeGroup) = construitMaillagePipe(gptsdisks, idisklim, nbsegCercle, nbsegRad)
  
  # --- edges de bord, faces défaut à respecter
  
  (internalBoundary, bordsLibres, grpAretesVives) = mailleAretesEtJonction(internalBoundary, aretesVivesCoupees, lgAretesVives)

  # --- maillage faces de fissure
  
  (meshFaceFiss, grpFaceFissureExterne, 
   grpEdgesPeauFissureExterne, grpEdgesPipeFissureExterne) = mailleFacesFissure(faceFissureExterne, edgesPipeFissureExterneC, edgesPeauFissureExterneC,
                                                                                meshPipeGroups, areteFaceFissure, rayonPipe, nbsegRad)

  # --- maillage faces de peau
  
  meshesFacesPeau = mailleFacesPeau(partitionsPeauFissFond, idFillingFromBout, facesDefaut,
                                    facesPeaux, edCircPeau, ptCircPeau, gpedgeBord, gpedgeVifs, edFissPeau, 
                                    bordsLibres, grpEdgesPeauFissureExterne, grpAretesVives,
                                    edgesCircPipeGroup, dmoyen, rayonPipe, nbsegRad)

  # --- regroupement des maillages du défaut

  listMeshes = [internalBoundary.GetMesh(),
                meshPipe.GetMesh(),
                meshFaceFiss.GetMesh()]
  for mp in meshesFacesPeau:
    listMeshes.append(mp.GetMesh())

  meshBoiteDefaut = smesh.Concatenate(listMeshes, 1, 1, 1e-05,False)
  # pour aider l'algo hexa-tetra à ne pas mettre de pyramides à l'exterieur des volumes repliés sur eux-mêmes
  # on désigne les faces de peau en quadrangles par le groupe "skinFaces"
  group_faceFissOutPipe = None
  group_faceFissInPipe = None
  groups = meshBoiteDefaut.GetGroups()
  for grp in groups:
    if grp.GetType() == SMESH.FACE:
      if grp.GetName() == "fisOutPi":
        group_faceFissOutPipe = grp
      elif grp.GetName() == "fisInPi":
        group_faceFissInPipe = grp

  # le maillage NETGEN ne passe pas toujours ==> utiliser GHS3D
  distene=True
  if distene:
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.GHS3D)
  else:
    algo3d = meshBoiteDefaut.Tetrahedron(algo=smeshBuilder.NETGEN)
    hypo3d = algo3d.MaxElementVolume(1000.0)
  putName(algo3d.GetSubMesh(), "boiteDefaut")
  putName(algo3d, "algo3d_boiteDefaut")
  isDone = meshBoiteDefaut.Compute()
  putName(meshBoiteDefaut, "boiteDefaut")
  logging.info("meshBoiteDefaut fini")
  
  faceFissure = meshBoiteDefaut.GetMesh().UnionListOfGroups( [ group_faceFissOutPipe, group_faceFissInPipe ], 'FACE1' )
  maillageSain = enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin,
                              zoneDefaut_internalFaces, zoneDefaut_internalEdges)
  putName(maillageSain, nomFicSain+"_coupe")
  extrusionFaceFissure, normfiss = shapeSurFissure(facesPortFissure)
  maillageComplet = RegroupeSainEtDefaut(maillageSain, meshBoiteDefaut,
                                         None, None, 'COMPLET', normfiss)

  logging.info("conversion quadratique")
  maillageComplet.ConvertToQuadratic( 1 )
  logging.info("groupes")
  groups = maillageComplet.GetGroups()
  grps = [ grp for grp in groups if grp.GetName() == 'FONDFISS']
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FONDFISS' )

  logging.info("réorientation face de fissure FACE1")
  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  nb = maillageComplet.Reorient2D( grps[0], normfiss, grps[0].GetID(1))

  logging.info("réorientation face de fissure FACE2")
  plansim = geompy.MakePlane(O, normfiss, 10000)
  fissnorm = geompy.MakeMirrorByPlane(normfiss, plansim)
  grps = [ grp for grp in groups if grp.GetName() == 'FACE2']
  nb = maillageComplet.Reorient2D( grps[0], fissnorm, grps[0].GetID(1))
  fond = maillageComplet.GetMesh().CreateDimGroup( grps, SMESH.NODE, 'FACE2' )

  logging.info("export maillage fini")
  maillageComplet.ExportMED( fichierMaillageFissure, 0, SMESH.MED_V2_2, 1 )
  putName(maillageComplet, nomFicFissure)
  logging.info("fichier maillage fissure %s", fichierMaillageFissure)

  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)

  logging.info("maillage fissure fini")
  
  return maillageComplet
