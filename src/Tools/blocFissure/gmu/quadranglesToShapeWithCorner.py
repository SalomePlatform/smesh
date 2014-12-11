# -*- coding: utf-8 -*-
"""
Created on Tue Jun 24 09:14:13 2014

@author: I48174
"""

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import GEOM

from listOfExtraFunctions import createNewMeshesFromCorner
from listOfExtraFunctions import createLinesFromMesh

# -----------------------------------------------------------------------------
# --- groupe de quadrangles de face transformé en face géométrique par filling

def quadranglesToShapeWithCorner(meshQuad, shapeDefaut, shapeFissureParams, centreFondFiss, listOfCorners):
    """ """
    # TODO: rédiger la docstring
    
    logging.info("start")

    #fillings = [[], []]
    tmpFillings = []
    noeuds_bords = []
    #bords_Partages = [[], []]
    tmpBords = []
    fillconts = []
    idFilToCont = []
    
    facesNonCoupees = []
    facesCoupees = []
    aretesNonCoupees = []
    aretesCoupees = []
    
    setOfNodes = []
    setOfLines = []
    listOfEdges = []
    # On crée une liste contenant le maillage de chaque face.
    listOfNewMeshes = createNewMeshesFromCorner(meshQuad, listOfCorners)
    for msh in listOfNewMeshes:
        # On crée une liste de noeuds correspondant aux faces suivant
        # le modèle liste[face][ligne][noeud].
        lines = createLinesFromMesh(msh, listOfCorners[0])
        setOfNodes.append(lines)
    
    for face in setOfNodes:
        tmpFace = []
        for line in face:
            # On possède l'information 'ID' de chaque noeud composant chaque
            # ligne de chaque face. A partir de l'ID, on crée un vertex. Un
            # ensemble de vertices constitue une ligne. Un ensemble de lignes
            # constitue une face.
            tmpCoords = [meshQuad.GetNodeXYZ(node) for node in line]
            tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
            line = geompy.MakeInterpol(tmpPoints, False, False)
            tmpFace.append(line)
        setOfLines.append(tmpFace)
    
    for i, face in enumerate(setOfLines):
        # A partir des lignes de chaque face,
        # on recrée un objet GEOM temporaire par filling.
        filling = geompy.MakeFilling(geompy.MakeCompound(face), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
        geomPublish(initLog.debug, filling, 'filling_{0}'.format(i + 1)) 
        tmpFillings.append(filling)

    for face in setOfNodes:
        # On prend la première ligne qui correspond aux bords partagés
        listOfEdges.append(face[0])
    
    for edge in listOfEdges:
        # On utilise les points de bords pour créer des aretes vives
        tmpCoords = [meshQuad.GetNodeXYZ(node) for node in list(edge)]
        tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
        line = geompy.MakeInterpol(tmpPoints, False, False)
        tmpBords.append(line)
    
    for i, filling in enumerate(tmpFillings):
        tmpPartition = geompy.MakePartition([filling], [shapeDefaut], [], [], geompy.ShapeType["FACE"], 0, [], 0, True)
        tmpExplodeRef = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], True)
        tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True)
        if len(tmpExplodeRef) == len(tmpExplodeNum):
            geomPublish(initLog.debug, filling, "faceNonCoupee_{0}".format(i + 1))
            facesNonCoupees.append(filling)
        else:
            geomPublish(initLog.debug, filling, "faceCoupee_{0}".format(i + 1))
            facesCoupees.append(filling)
    fillings = facesCoupees, facesNonCoupees
    
    for i, filling in enumerate(tmpBords):
        tmpPartition = geompy.MakePartition([shapeDefaut], [filling], [], [], geompy.ShapeType["SHELL"], 0, [], 0, True)
        tmpExplodeRef = geompy.ExtractShapes(shapeDefaut, geompy.ShapeType["EDGE"], True) + geompy.ExtractShapes(shapeDefaut, geompy.ShapeType["VERTEX"], True)
        tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True) + geompy.ExtractShapes(tmpPartition, geompy.ShapeType["VERTEX"], True)
        if len(tmpExplodeRef) == len(tmpExplodeNum):
            geomPublish(initLog.debug, filling, "areteNonCoupee_{0}".format(i + 1))
            aretesNonCoupees.append(filling)
        else:
            geomPublish(initLog.debug, filling, "areteCoupee_{0}".format(i + 1))
            aretesCoupees.append(filling)
    bords_Partages = aretesCoupees, aretesNonCoupees
    
# TODO: A enlever
#    for i, face in enumerate(setOfLines):
#        for j, line in enumerate(face):
#            geomPublish(initLog.debug, line, 'face{0}_ligne{1}'.format(i + 1, j + 1))

 #TODO: A enlever
#    for i, filling in enumerate(fillings[0]):
#        geomPublish(initLog.debug, filling, 'filling_{0}'.format(i + 1))
#        for j, line in enumerate(setOfLines[i]):
#            geompy.addToStudyInFather(filling, line, 'line_{0}'.format(j + 1))
    
    return fillings, noeuds_bords, bords_Partages, fillconts, idFilToCont
    