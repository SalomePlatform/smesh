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
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# -----------------------------------------------------------------------------

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
blob=geompy.RemoveExtraEdges(blob)

idblob = geompy.addToStudy(blob,"blob")

edgeGroups = geompy.Propagate( blob )
assert len( edgeGroups ) == 3

salome.sg.updateObjBrowser(True)

# -----------------------------------------------------------------------------

print "-------------------------- mesh"
smesh.SetCurrentStudy(salome.myStudy)

# ---- define a mesh on the geom shape 'blob'
mesh=smesh.Mesh(blob, "MeshBlob")

# ---- assign global hypothesis and algorithms to mesh
print "-------------------------- add hypothesis to mesh"
algo1 = mesh.Segment()
algo2 = mesh.Quadrangle()
algo3 = mesh.Hexahedron()

# ---- assign local hypothesis and algorithms to mesh
for edges in edgeGroups: # loop on groups of logically parallel edges
    length = geompy.BasicProperties( edges )[0]
    if   length < 500:  nbSeg = 4
    elif length < 2000: nbSeg = 10
    else:               nbSeg = 15
    algo = mesh.Segment( edges )
    algo.NumberOfSegments( nbSeg )
    pass

# ---- compute mesh
print "-------------------------- compute mesh"
ok = mesh.Compute()
if ok:
    print "Information about the Mesh:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of quadrangles : ", mesh.NbQuadrangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of hexahedrons : ", mesh.NbHexas()
else:
    print "problem when Computing the mesh"

salome.sg.updateObjBrowser(True)
