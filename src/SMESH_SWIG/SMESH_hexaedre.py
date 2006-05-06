#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/
#
#==============================================================================
#  Info.
#  Bug (from script, bug)   : hexaedre_modified.py, PAL6194, PAL7153
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#==============================================================================

import salome
from salome import sg

import geompy

import math

# -----------------------------------------------------------------------------

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
gg = salome.ImportComponentGUI("GEOM")
from salome import sg

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

import SMESH
import StdMeshers
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# ---- create Hypothesis
print "-------------------------- create Hypothesis"
numberOfSegments = 4
hypNbSegA=smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSegA.SetNumberOfSegments(numberOfSegments)
numberOfSegments = 10
hypNbSegB=smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSegB.SetNumberOfSegments(numberOfSegments)
numberOfSegments = 15
hypNbSegC=smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSegC.SetNumberOfSegments(numberOfSegments)

# ---- create Algorithms
print "-------------------------- create Algorithms"
regular1D=smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
quad2D=smesh.CreateHypothesis("Quadrangle_2D", "libStdMeshersEngine.so")
hexa3D=smesh.CreateHypothesis("Hexa_3D", "libStdMeshersEngine.so")

# ---- init a Mesh with the geom shape
shape_mesh = blob
myMesh=smesh.CreateMesh(shape_mesh)

# ---- add hypothesis and algorithms to mesh
print "-------------------------- add hypothesis to mesh"
myMesh.AddHypothesis(shape_mesh,regular1D)
myMesh.AddHypothesis(shape_mesh,quad2D)
myMesh.AddHypothesis(shape_mesh,hexa3D)

#myMesh.AddHypothesis(shape_mesh,hypNbSeg)

myMesh.AddHypothesis(aretes[0],hypNbSegA)
myMesh.AddHypothesis(aretes[2],hypNbSegA)
myMesh.AddHypothesis(aretes[8],hypNbSegA)
myMesh.AddHypothesis(aretes[10],hypNbSegA)

myMesh.AddHypothesis(aretes[1],hypNbSegC)
myMesh.AddHypothesis(aretes[3],hypNbSegC)
myMesh.AddHypothesis(aretes[9],hypNbSegC)
myMesh.AddHypothesis(aretes[11],hypNbSegC)

myMesh.AddHypothesis(aretes[4],hypNbSegB)
myMesh.AddHypothesis(aretes[5],hypNbSegB)
myMesh.AddHypothesis(aretes[6],hypNbSegB)
myMesh.AddHypothesis(aretes[7],hypNbSegB)

# ---- compute mesh

print "-------------------------- compute mesh"
ret=smesh.Compute(myMesh, shape_mesh)
print ret
if ret != 0:
    #log=myMesh.GetLog(0) # no erase trace
    #for linelog in log:
    #    print linelog
    print "Information about the Mesh:"
    print "Number of nodes       : ", myMesh.NbNodes()
    print "Number of edges       : ", myMesh.NbEdges()
    print "Number of faces       : ", myMesh.NbFaces()
    print "Number of quadrangles : ", myMesh.NbQuadrangles()
    print "Number of volumes     : ", myMesh.NbVolumes()
    print "Number of hexahedrons : ", myMesh.NbHexas()
else:
    print "problem when Computing the mesh"

salome.sg.updateObjBrowser(1)
