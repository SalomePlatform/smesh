#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

# Tetrahedrization of the geometry generated by the Python script GEOM_Partition1.py
# Hypothesis and algorithms for the mesh generation are global
# -- Rayon de la bariere
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

from math import sqrt


#---------------------------------------------------------------

barier_height = 7.0
barier_radius = 5.6 / 2 # Rayon de la bariere
colis_radius = 1.0 / 2  # Rayon du colis
colis_step = 2.0        # Distance s�parant deux colis
cc_width = 0.11         # Epaisseur du complement de colisage

# --

cc_radius = colis_radius + cc_width
colis_center = sqrt(2.0)*colis_step/2

# --

boolean_common  = 1
boolean_cut     = 2
boolean_fuse    = 3
boolean_section = 4

# --

p0 = geompy.MakeVertex(0.,0.,0.)
vz = geompy.MakeVectorDXDYDZ(0.,0.,1.)

# --

barier = geompy.MakeCylinder(p0, vz, barier_radius, barier_height)

# --

colis = geompy.MakeCylinder(p0, vz, colis_radius, barier_height)
cc    = geompy.MakeCylinder(p0, vz,    cc_radius, barier_height)

colis_cc = geompy.MakeCompound([colis, cc])
colis_cc = geompy.MakeTranslation(colis_cc, colis_center, 0.0, 0.0)

colis_cc_multi = geompy.MultiRotate1D(colis_cc, vz, 4)

# --

Compound1 = geompy.MakeCompound([colis_cc_multi, barier])
SubShape_theShape = geompy.SubShapeAll(Compound1,geompy.ShapeType["SOLID"])
alveole = geompy.MakePartition(SubShape_theShape)

print "Analysis of the geometry to mesh (right after the Partition) :"

subShellList = geompy.SubShapeAll(alveole, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(alveole, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(alveole, geompy.ShapeType["EDGE"])

print "number of Shells in alveole : ", len(subShellList)
print "number of Faces  in alveole : ", len(subFaceList)
print "number of Edges  in alveole : ", len(subEdgeList)

subshapes = geompy.SubShapeAll(alveole, geompy.ShapeType["SHAPE"])

## there are 9 sub-shapes

comp1 = geompy.MakeCompound( [ subshapes[0], subshapes[1] ] )
comp2 = geompy.MakeCompound( [ subshapes[2], subshapes[3] ] )
comp3 = geompy.MakeCompound( [ subshapes[4], subshapes[5] ] )
comp4 = geompy.MakeCompound( [ subshapes[6], subshapes[7] ] )

compGOs = []
compGOs.append( comp1 )
compGOs.append( comp2 )
compGOs.append( comp3 )
compGOs.append( comp4 )
comp = geompy.MakeCompound( compGOs )

alveole = geompy.MakeCompound( [ comp, subshapes[8] ])

idalveole = geompy.addToStudy(alveole, "alveole")

print "Analysis of the geometry to mesh (right after the MakeCompound) :"

subShellList = geompy.SubShapeAll(alveole, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(alveole, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(alveole, geompy.ShapeType["EDGE"])

print "number of Shells in alveole : ", len(subShellList)
print "number of Faces  in alveole : ", len(subFaceList)
print "number of Edges  in alveole : ", len(subEdgeList)

status = geompy.CheckShape(alveole)
print " check status ", status


# ---- init a Mesh with the alveole
shape_mesh = salome.IDToObject( idalveole )

mesh = smesh.Mesh(shape_mesh, "MeshAlveole")

print "-------------------------- create Hypothesis (In this case global hypothesis are used)"

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

regular1D = mesh.Segment()
hypNbSeg = regular1D.NumberOfSegments(numberOfSegments)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "NumberOfSegments_" + str(numberOfSegments))

print "-------------------------- MaxElementArea"

maxElementArea = 0.1

mefisto2D = mesh.Triangle()
hypArea = mefisto2D.MaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()
smesh.SetName(hypArea, "MaxElementArea_" + str(maxElementArea))

print "-------------------------- MaxElementVolume"

maxElementVolume = 0.5

netgen3D = mesh.Tetrahedron(smeshBuilder.NETGEN)
hypVolume = netgen3D.MaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()
smesh.SetName(hypVolume, "MaxElementVolume_" + str(maxElementVolume))

print "-------------------------- compute the mesh of alveole "
ret = mesh.Compute()

if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the Mesh_mechanic:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "problem when computing the mesh"

salome.sg.updateObjBrowser(1)
