# -*- coding: utf-8 -*-
"""
Created on Tue Jun 24 09:14:13 2014

@author: I48174 (Olivier HOAREAU)
"""

import logging
from geomsmesh import geompy
from geomsmesh import smesh
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import GEOM
import SMESH

from listOfExtraFunctions import createNewMeshesFromCorner
from listOfExtraFunctions import createLinesFromMesh

# -----------------------------------------------------------------------------
# --- groupe de quadrangles de face transformé en face géométrique par filling

def fusionMaillageDefaut(maillageSain, maillageDefautCible, maillageInterneCible, zoneDefaut_skin, shapeDefaut, listOfCorners):
    """ """
    # TODO: rédiger la docstring
    
    logging.info("start")
    
    facesNonCoupees = []
    facesCoupees = []
    maillagesNonCoupes = []
    maillagesCoupes = []
        
    # On crée une liste contenant le maillage de chaque face.
    listOfNewMeshes = createNewMeshesFromCorner(maillageDefautCible, listOfCorners)
    
    i = 0
    while i < len(listOfNewMeshes):
        lines = createLinesFromMesh(listOfNewMeshes[i])
        setOfLines = []
        for line in lines:
            # On possède l'information 'ID' de chaque noeud composant chaque
            # ligne de la face. A partir de l'ID, on crée un vertex. Un
            # ensemble de vertices constitue une ligne. Un ensemble de lignes
            # constitue la face.
            tmpCoords = [maillageDefautCible.GetNodeXYZ(node) for node in line]
            tmpPoints = [geompy.MakeVertex(val[0], val[1], val[2]) for val in tmpCoords]
            line = geompy.MakeInterpol(tmpPoints, False, False)
            setOfLines.append(line)
        
        # A partir des lignes de la face,
        # on recrée un objet GEOM temporaire par filling.
        filling = geompy.MakeFilling(geompy.MakeCompound(setOfLines), 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default, True)
        #logging.debug("face de filling")
        #geomPublish(initLog.debug, filling, 'filling_{0}'.format(i + 1))
        
        tmpPartition = geompy.MakePartition([filling], [shapeDefaut], [], [], geompy.ShapeType["FACE"], 0, [], 0)
        tmpExplodeRef = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], True)
        tmpExplodeNum = geompy.ExtractShapes(tmpPartition, geompy.ShapeType["EDGE"], True)
        if len(tmpExplodeRef) == len(tmpExplodeNum):
            logging.debug("face de filling non coupee")
            geompy.addToStudy( filling, "faceNonCoupee_{0}".format(i + 1)) # doit etre publie pour critere OK plus bas
            facesNonCoupees.append(filling)
            maillagesNonCoupes.append(listOfNewMeshes[i])
        else:
            logging.debug("face de filling coupee")
            geompy.addToStudy( filling, "faceCoupee_{0}".format(i + 1))
            facesCoupees.append(filling)
            maillagesCoupes.append(listOfNewMeshes[i])
        
        i += 1
    
    listOfInternMeshes = [maillageInterneCible] + [msh.GetMesh() for msh in maillagesNonCoupes]
    
    newMaillageInterne = smesh.Concatenate(listOfInternMeshes, 1, 1, 1e-05, False)
    
    facesEnTrop = []
    
    criteres = [smesh.GetCriterion(SMESH.FACE, SMESH.FT_BelongToGenSurface, SMESH.FT_Undefined, face) for face in facesNonCoupees]
    filtres = [smesh.GetFilterFromCriteria([critere]) for critere in criteres]
    for i, filtre in enumerate(filtres):
        filtre.SetMesh(maillageSain.GetMesh())
        faceEnTrop = maillageSain.GroupOnFilter(SMESH.FACE, 'faceEnTrop_{0}'.format(i + 1), filtre)
        facesEnTrop.append(faceEnTrop)
    
    newZoneDefaut_skin = maillageSain.GetMesh().CutListOfGroups([zoneDefaut_skin], facesEnTrop, 'newZoneDefaut_skin')
    
    smesh.SetName(newMaillageInterne, 'newInternalBoundary')
        
    return newZoneDefaut_skin, newMaillageInterne
    