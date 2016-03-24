#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
import re

import batchmode_salome
import batchmode_geompy
import batchmode_smesh
from salome.StdMeshers import StdMeshersBuilder

smesh = batchmode_smesh.smesh
smesh.SetCurrentStudy(batchmode_salome.myStudy)

def CreateMesh (theFileName, area, len = None, nbseg = None):
    
    if not(os.path.isfile(theFileName)) or re.search("\.brep$", theFileName) is None :
        print "Incorrect file name !"
        return

    if (len is None) and (nbseg is None):
        print "Define length or number of segments !"
        return

    if (len is not None) and (nbseg is not None):
        print "Only one Hypothesis (from length and number of segments) can be defined !"
        return

    
    # ----  Import shape from BREP file and add it to the study  
    shape_mesh = batchmode_geompy.Import(theFileName, "BREP")
    Id_shape = batchmode_geompy.addToStudy(shape_mesh, "shape_mesh")


    # ---- SMESH
    print "-------------------------- create mesh"
    mesh = smesh.Mesh(shape_mesh)
      
    print "-------------------------- create Hypothesis"
    if (len is not None):
        print "-------------------------- LocalLength"
        algoReg = mesh.Segment()
        hypLength1 = algoReg.LocalLength(len)
        print "Hypothesis type : ", hypLength1.GetName()
        print "Hypothesis ID   : ", hypLength1.GetId()
        print "Hypothesis Value: ", hypLength1.GetLength()
    
    if (nbseg is not None):   
        print "-------------------------- NumberOfSegments"
        algoReg = mesh.Segment()
        hypNbSeg1 = algoReg.NumberOfSegments(nbseg)
        print "Hypothesis type : ", hypNbSeg1.GetName()
        print "Hypothesis ID   : ", hypNbSeg1.GetId()
        print "Hypothesis Value: ", hypNbSeg1.GetNumberOfSegments()

    if (area == "LengthFromEdges"):
        print "-------------------------- LengthFromEdges"
        algoMef = mesh.Triangle()
        hypLengthFromEdges = algoMef.LengthFromEdges(1)
        print "Hypothesis type     : ", hypLengthFromEdges.GetName()
        print "Hypothesis ID       : ", hypLengthFromEdges.GetId()
        print "LengthFromEdges Mode: ", hypLengthFromEdges.GetMode()
       
    else:
        print "-------------------------- MaxElementArea"
        algoMef = mesh.Triangle()
        hypArea1 = algoMef.MaxElementArea(area)
        print "Hypothesis type : ", hypArea1.GetName()
        print "Hypothesis ID   : ", hypArea1.GetId()
        print "Hypothesis Value: ", hypArea1.GetMaxElementArea()
              
    
    print "-------------------------- Regular_1D"
    listHyp = algoReg.GetCompatibleHypothesis()
    for hyp in listHyp:
        print hyp
    
    print "Algo name: ", algoReg.GetName()
    print "Algo ID  : ", algoReg.GetId()
   
    print "-------------------------- MEFISTO_2D"
    listHyp = algoMef.GetCompatibleHypothesis()
    for hyp in listHyp:
        print hyp
        
    print "Algo name: ", algoMef.GetName()
    print "Algo ID  : ", algoMef.GetId()


    # ---- add hypothesis to shape

    print "-------------------------- compute mesh"
    ret = mesh.Compute()
    print  "Compute Mesh .... ", 
    print ret
    log = mesh.GetLog(0); # no erase trace
    #for linelog in log:
    #    print linelog

    print "------------ INFORMATION ABOUT MESH ------------"
    
    print "Number of nodes    : ", mesh.NbNodes()
    print "Number of edges    : ", mesh.NbEdges()
    print "Number of faces    : ", mesh.NbFaces()
    print "Number of triangles: ", mesh.NbTriangles()

    return mesh
