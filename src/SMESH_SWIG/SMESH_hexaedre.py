#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

#==============================================================================
#  Info.
#  Bug (from script, bug)   : hexaedre_modified.py, PAL6194, PAL7153
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#==============================================================================
#
import salome
import geompy
import smesh

import math

# -----------------------------------------------------------------------------

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
gg = salome.ImportComponentGUI("GEOM")

ShapeTypeCompSolid = 1
ShapeTypeSolid     = 2
ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeWire      = 5
ShapeTypeEdge      = 6
ShapeTypeVertex    = 7

Boolop_common = 1
Boolop_cut = 2
Boolop_fuse = 3
Boolop_section = 4

p0 = geompy.MakeVertex(0., 0., 0.)
px = geompy.MakeVertex(100., 0., 0.)
py = geompy.MakeVertex(0., 100., 0.)
pz = geompy.MakeVertex(0., 0., 100.)
vx = geompy.MakeVector(p0, px)
vy = geompy.MakeVector(p0, py)
vz = geompy.MakeVector(p0, pz)

sphereExt = geompy.MakeSphere(    0.,  0.,  0., 400.)
sphereInt = geompy.MakeSphere(    0.,-50.,  0., 350.)
sphereA   = geompy.MakeSphere( -400., 50., 50., 400.)
sphereB   = geompy.MakeSphere(  350.,-50.,-50., 350.)
ptcyle    = geompy.MakeVertex(0., -300., -450.)
cylindre  = geompy.MakeCylinder(ptcyle,vz,500.,900.)

vol1=geompy.MakeCut(sphereExt,sphereA)
vol2=geompy.MakeCut(vol1,sphereB)
vol3=geompy.MakeCut(vol2,cylindre)
blob=geompy.MakeCut(vol3,sphereInt)

idblob = geompy.addToStudy(blob,"blob")

aretes = []
aretes = geompy.SubShapeAllSorted(blob, ShapeTypeEdge)
eid=0

# -------------------------------
# --- numerotation des aretes
##for edge in aretes:
##    edname="arete%d"%eid
##    idedge=geompy.addToStudy(edge,edname)
##    eid=eid+1

salome.sg.updateObjBrowser(1)

# --- epaisseur 0 2 8 10
# --- hauteur   1 3 9 11
# --- largeur   4 5 6 7

# -----------------------------------------------------------------------------

print "-------------------------- mesh"
smesh.SetCurrentStudy(salome.myStudy)

# ---- init a Mesh with the geom shape
shape_mesh = blob
mesh=smesh.Mesh(shape_mesh, "MeshBlob")

# ---- add hypothesis and algorithms to mesh
print "-------------------------- add hypothesis to mesh"
algo1 = mesh.Segment()
algo2 = mesh.Quadrangle()
algo3 = mesh.Hexahedron()

numberOfSegmentsA = 4

algo = mesh.Segment(aretes[0])
algo.NumberOfSegments(numberOfSegmentsA)
algo = mesh.Segment(aretes[2])
algo.NumberOfSegments(numberOfSegmentsA)
algo = mesh.Segment(aretes[8])
algo.NumberOfSegments(numberOfSegmentsA)
algo = mesh.Segment(aretes[10])
algo.NumberOfSegments(numberOfSegmentsA)


numberOfSegmentsC = 15

algo = mesh.Segment(aretes[1])
algo.NumberOfSegments(numberOfSegmentsC)
algo = mesh.Segment(aretes[3])
algo.NumberOfSegments(numberOfSegmentsC)
algo = mesh.Segment(aretes[9])
algo.NumberOfSegments(numberOfSegmentsC)
algo = mesh.Segment(aretes[11])
algo.NumberOfSegments(numberOfSegmentsC)


numberOfSegmentsB = 10
algo = mesh.Segment(aretes[4])
algo.NumberOfSegments(numberOfSegmentsB)
algo = mesh.Segment(aretes[5])
algo.NumberOfSegments(numberOfSegmentsB)
algo = mesh.Segment(aretes[6])
algo.NumberOfSegments(numberOfSegmentsB)
algo = mesh.Segment(aretes[7])
algo.NumberOfSegments(numberOfSegmentsB)


# ---- compute mesh

print "-------------------------- compute mesh"
ret=mesh.Compute()
print ret
if ret != 0:
    #log=mesh.GetLog(0) # no erase trace
    #for linelog in log:
    #    print linelog
    print "Information about the Mesh:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of quadrangles : ", mesh.NbQuadrangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of hexahedrons : ", mesh.NbHexas()
else:
    print "problem when Computing the mesh"

salome.sg.updateObjBrowser(1)
