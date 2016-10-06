#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

# =======================================
# Procedure that take a triangulation and split all triangles in 4 others triangles
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import os

# Values
# ------

# Path for ".med" files
path = "/tmp/ex29_%s_" % os.getenv('USER','unknown')

# Name of the shape and the mesh
name = "Carre"

# Add a node and needed edges
# ---------------------------

def node(m, f, n1, n2, lnv):
    x1, y1, z1 = m.GetNodeXYZ(n1)
    x2, y2, z2 = m.GetNodeXYZ(n2)

    x = (x1 + x2) / 2.0
    y = (y1 + y2) / 2.0
    z = (z1 + z2) / 2.0

    i = m.AddNode(x, y, z)

    in1 = m.GetShapeID(n1)
    in2 = m.GetShapeID(n2)

    if (in1==f) or (in2==f):
        m.SetNodeOnFace(i, f, 0, 0)

    else:
        e1 = m.AddEdge([ n1, i  ])
        e2 = m.AddEdge([ i , n2 ])

        if n1 in lnv:
            e = in2
        else:
            e = in1

        m.SetMeshElementOnShape(e1, e)
        m.SetMeshElementOnShape(e2, e)
        m.SetNodeOnEdge(i, e, 0)

    return i

# Add a triangle and associate to the CAD face
# --------------------------------------------

def triangle(m, f, n1, n2, n3):
    i = m.AddFace([ n1, n2, n3 ])
    m.SetMeshElementOnShape(i, f)

# Split all triangles in 4 triangles
# ----------------------------------

def SplitTrianglesIn4(m):
    # Get all triangles
    triangles = m.GetElementsByType(SMESH.FACE)

    # Remove all edges
    m.RemoveElements(m.GetElementsByType(SMESH.EDGE))

    # Get the list of nodes (ids) associated with the CAD vertices
    shape = m.GetShape()
    lnv = []
    for v in geompy.SubShapeAll(shape, geompy.ShapeType["VERTEX"]):
        lnv = lnv + m.GetSubMeshNodesId(v, True)

    # Split every triangle
    for t in triangles:
        noeud_1, noeud_2, noeud_3 = m.GetElemNodes(t)

        face = m.GetShapeIDForElem(t)

        noeud_12 = node(m, face, noeud_1, noeud_2, lnv)
        noeud_23 = node(m, face, noeud_2, noeud_3, lnv)
        noeud_13 = node(m, face, noeud_1, noeud_3, lnv)

        triangle(m, face, noeud_1 , noeud_12, noeud_13)
        triangle(m, face, noeud_2 , noeud_23, noeud_12)
        triangle(m, face, noeud_3 , noeud_13, noeud_23)
        triangle(m, face, noeud_12, noeud_23, noeud_13)

    # Remove all initial triangles
    m.RemoveElements(triangles)

    # Merge all identical nodes
    m.MergeNodes(m.FindCoincidentNodes(0.0001))

# Build a CAD square
# ------------------

x0 = 0.0 ; y0 = 0.0 ; z0 = 0.0
x1 = 1.0 ; y1 = 0.0 ; z1 = 0.0
x2 = 1.0 ; y2 = 1.0 ; z2 = 0.0
x3 = 0.0 ; y3 = 1.0 ; z3 = 0.0

P0 = geompy.MakeVertex(x0, y0, z0)
P1 = geompy.MakeVertex(x1, y1, z1)
P2 = geompy.MakeVertex(x2, y2, z2)
P3 = geompy.MakeVertex(x3, y3, z3)

square = geompy.MakeQuad4Vertices(P0, P1, P2, P3)
geompy.addToStudy(square, name)

# Refine edges and create group of mesh
# -------------------------------------

def refine(m, p1, p2, n, k, name):
    s = m.GetShape()

    g = geompy.CreateGroup(s, geompy.ShapeType["EDGE"])
    e = geompy.GetEdge(s, p1, p2)
    i = geompy.GetSubShapeID(s, e)
    geompy.AddObject(g, i)
    m.Group(g, name)

    a = m.Segment(e)
    a.NumberOfSegments(n, k)

# Mesh the square
# ---------------

MyMesh = smesh.Mesh(square)

refine(MyMesh, P1, P2,  8,  7, "Droite")
refine(MyMesh, P3, P0,  9, 10, "Gauche")
refine(MyMesh, P0, P1,  7,  9, "Bas"   )
refine(MyMesh, P2, P3, 12, 14, "Haut"  )

algo2D = MyMesh.Triangle()
algo2D.MaxElementArea(0.07)

MyMesh.Compute()

MyMesh.ExportMED(path+"110_triangles.med", 0)

# Disturb the mesh
# ----------------

MyMesh.MoveNode( 37, 0.05    , 0.368967 , 0 )
MyMesh.MoveNode( 38, 0.34    , 0.0762294, 0 )
MyMesh.MoveNode( 40, 0.8     , 0.42     , 0 )
MyMesh.MoveNode( 42, 0.702662, 0.74     , 0 )
MyMesh.MoveNode( 46, 0.4     , 0.374656 , 0 )
MyMesh.MoveNode( 47, 0.13    , 0.63     , 0 )
MyMesh.MoveNode( 49, 0.222187, 0.3      , 0 )
MyMesh.MoveNode( 54, 0.557791, 0.05     , 0 )
MyMesh.MoveNode( 55, 0.7     , 0.2      , 0 )
MyMesh.MoveNode( 56, 0.73    , 0.52     , 0 )
MyMesh.MoveNode( 58, 0.313071, 0.31     , 0 )
MyMesh.MoveNode( 59, 0.8     , 0.56     , 0 )
MyMesh.MoveNode( 62, 0.592703, 0.95     , 0 )
MyMesh.MoveNode( 63, 0.28    , 0.5      , 0 )
MyMesh.MoveNode( 65, 0.49    , 0.93     , 0 )
MyMesh.MoveNode( 68, 0.501038, 0.65     , 0 )
MyMesh.MoveNode( 69, 0.37    , 0.63     , 0 )
MyMesh.MoveNode( 70, 0.597025, 0.52     , 0 )
MyMesh.MoveNode( 72, 0.899   , 0.878589 , 0 )
MyMesh.MoveNode( 73, 0.92    , 0.85     , 0 )
MyMesh.MoveNode( 74, 0.820851, 0.75     , 0 )

NbCells1 = 110
MyMesh.ExportMED(path+"110_triangles_2.med", 0)

# First mesh refining
# -------------------

SplitTrianglesIn4(MyMesh)

NbCells2 = NbCells1*4
print("Mesh with "+str(NbCells2)+" cells computed.")

MyMesh.ExportMED(path+str(NbCells2)+"_triangles.med", 0)

# Second mesh refining
# --------------------

SplitTrianglesIn4(MyMesh)

NbCells3 = NbCells2*4
print("Mesh with "+str(NbCells3)+" cells computed.")

MyMesh.ExportMED(path+str(NbCells3)+"_triangles.med",0)

# Third mesh refining
# -------------------

SplitTrianglesIn4(MyMesh)

NbCells4 = NbCells3*4
print("Mesh with "+str(NbCells4)+" cells computed.")

MyMesh.ExportMED(path+str(NbCells4)+"_triangles.med", 0)

# Update the object browser
# -------------------------

salome.sg.updateObjBrowser(True)
