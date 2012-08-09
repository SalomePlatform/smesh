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

# =======================================
#
from geompy import *

import smesh

# Geometry
# ========

# A small cube centered and put on a great cube build by points, edges, faces and solids

# Points
# ------

greatPoint111 = MakeVertex( 0,  0,  0)
greatPoint211 = MakeVertex(10,  0,  0)
greatPoint311 = MakeVertex(20,  0,  0)
greatPoint411 = MakeVertex(30,  0,  0)

greatPoint121 = MakeVertex( 0, 10,  0)
greatPoint221 = MakeVertex(10, 10,  0)
greatPoint321 = MakeVertex(20, 10,  0)
greatPoint421 = MakeVertex(30, 10,  0)

greatPoint112 = MakeVertex( 0,  0, 10)
greatPoint212 = MakeVertex(10,  0, 10)
greatPoint312 = MakeVertex(20,  0, 10)
greatPoint412 = MakeVertex(30,  0, 10)

greatPoint122 = MakeVertex( 0, 10, 10)
greatPoint222 = MakeVertex(10, 10, 10)
greatPoint322 = MakeVertex(20, 10, 10)
greatPoint422 = MakeVertex(30, 10, 10)

greatPoint113 = MakeVertex( 0,  0, 20)
greatPoint213 = MakeVertex(10,  0, 20)
greatPoint313 = MakeVertex(20,  0, 20)
greatPoint413 = MakeVertex(30,  0, 20)

greatPoint123 = MakeVertex( 0, 10, 20)
greatPoint223 = MakeVertex(10, 10, 20)
greatPoint323 = MakeVertex(20, 10, 20)
greatPoint423 = MakeVertex(30, 10, 20)

greatPoint114 = MakeVertex( 0,  0, 30)
greatPoint214 = MakeVertex(10,  0, 30)
greatPoint314 = MakeVertex(20,  0, 30)
greatPoint414 = MakeVertex(30,  0, 30)

greatPoint124 = MakeVertex( 0, 10, 30)
greatPoint224 = MakeVertex(10, 10, 30)
greatPoint324 = MakeVertex(20, 10, 30)
greatPoint424 = MakeVertex(30, 10, 30)


smallPoint111 = greatPoint222
smallPoint211 = greatPoint322
smallPoint121 = MakeVertex(10, 20, 10)
smallPoint221 = MakeVertex(20, 20, 10)

smallPoint112 = greatPoint223
smallPoint212 = greatPoint323
smallPoint122 = MakeVertex(10, 20, 20)
smallPoint222 = MakeVertex(20, 20, 20)

# Edges
# -----

smallEdgeX11 = MakeEdge(smallPoint111, smallPoint211)
smallEdgeX21 = MakeEdge(smallPoint121, smallPoint221)
smallEdgeX12 = MakeEdge(smallPoint112, smallPoint212)
smallEdgeX22 = MakeEdge(smallPoint122, smallPoint222)

smallEdgeY11 = MakeEdge(smallPoint111, smallPoint121)
smallEdgeY21 = MakeEdge(smallPoint211, smallPoint221)
smallEdgeY12 = MakeEdge(smallPoint112, smallPoint122)
smallEdgeY22 = MakeEdge(smallPoint212, smallPoint222)

smallEdgeZ11 = MakeEdge(smallPoint111, smallPoint112)
smallEdgeZ21 = MakeEdge(smallPoint211, smallPoint212)
smallEdgeZ12 = MakeEdge(smallPoint121, smallPoint122)
smallEdgeZ22 = MakeEdge(smallPoint221, smallPoint222)


greatEdgeX111 = MakeEdge(greatPoint111, greatPoint211)
greatEdgeX211 = MakeEdge(greatPoint211, greatPoint311)
greatEdgeX311 = MakeEdge(greatPoint311, greatPoint411)
greatEdgeX121 = MakeEdge(greatPoint121, greatPoint221)
greatEdgeX221 = MakeEdge(greatPoint221, greatPoint321)
greatEdgeX321 = MakeEdge(greatPoint321, greatPoint421)

greatEdgeX112 = MakeEdge(greatPoint112, greatPoint212)
greatEdgeX212 = MakeEdge(greatPoint212, greatPoint312)
greatEdgeX312 = MakeEdge(greatPoint312, greatPoint412)
greatEdgeX122 = MakeEdge(greatPoint122, greatPoint222)
greatEdgeX222 = smallEdgeX11
greatEdgeX322 = MakeEdge(greatPoint322, greatPoint422)

greatEdgeX113 = MakeEdge(greatPoint113, greatPoint213)
greatEdgeX213 = MakeEdge(greatPoint213, greatPoint313)
greatEdgeX313 = MakeEdge(greatPoint313, greatPoint413)
greatEdgeX123 = MakeEdge(greatPoint123, greatPoint223)
greatEdgeX223 = smallEdgeX12
greatEdgeX323 = MakeEdge(greatPoint323, greatPoint423)

greatEdgeX114 = MakeEdge(greatPoint114, greatPoint214)
greatEdgeX214 = MakeEdge(greatPoint214, greatPoint314)
greatEdgeX314 = MakeEdge(greatPoint314, greatPoint414)
greatEdgeX124 = MakeEdge(greatPoint124, greatPoint224)
greatEdgeX224 = MakeEdge(greatPoint224, greatPoint324)
greatEdgeX324 = MakeEdge(greatPoint324, greatPoint424)

greatEdgeY11 = MakeEdge(greatPoint111, greatPoint121)
greatEdgeY21 = MakeEdge(greatPoint211, greatPoint221)
greatEdgeY31 = MakeEdge(greatPoint311, greatPoint321)
greatEdgeY41 = MakeEdge(greatPoint411, greatPoint421)

greatEdgeY12 = MakeEdge(greatPoint112, greatPoint122)
greatEdgeY22 = MakeEdge(greatPoint212, greatPoint222)
greatEdgeY32 = MakeEdge(greatPoint312, greatPoint322)
greatEdgeY42 = MakeEdge(greatPoint412, greatPoint422)

greatEdgeY13 = MakeEdge(greatPoint113, greatPoint123)
greatEdgeY23 = MakeEdge(greatPoint213, greatPoint223)
greatEdgeY33 = MakeEdge(greatPoint313, greatPoint323)
greatEdgeY43 = MakeEdge(greatPoint413, greatPoint423)

greatEdgeY14 = MakeEdge(greatPoint114, greatPoint124)
greatEdgeY24 = MakeEdge(greatPoint214, greatPoint224)
greatEdgeY34 = MakeEdge(greatPoint314, greatPoint324)
greatEdgeY44 = MakeEdge(greatPoint414, greatPoint424)

greatEdgeZ111 = MakeEdge(greatPoint111, greatPoint112)
greatEdgeZ211 = MakeEdge(greatPoint211, greatPoint212)
greatEdgeZ311 = MakeEdge(greatPoint311, greatPoint312)
greatEdgeZ411 = MakeEdge(greatPoint411, greatPoint412)

greatEdgeZ121 = MakeEdge(greatPoint121, greatPoint122)
greatEdgeZ221 = MakeEdge(greatPoint221, greatPoint222)
greatEdgeZ321 = MakeEdge(greatPoint321, greatPoint322)
greatEdgeZ421 = MakeEdge(greatPoint421, greatPoint422)

greatEdgeZ112 = MakeEdge(greatPoint112, greatPoint113)
greatEdgeZ212 = MakeEdge(greatPoint212, greatPoint213)
greatEdgeZ312 = MakeEdge(greatPoint312, greatPoint313)
greatEdgeZ412 = MakeEdge(greatPoint412, greatPoint413)

greatEdgeZ122 = MakeEdge(greatPoint122, greatPoint123)
greatEdgeZ222 = smallEdgeZ11
greatEdgeZ322 = smallEdgeZ21
greatEdgeZ422 = MakeEdge(greatPoint422, greatPoint423)

greatEdgeZ113 = MakeEdge(greatPoint113, greatPoint114)
greatEdgeZ213 = MakeEdge(greatPoint213, greatPoint214)
greatEdgeZ313 = MakeEdge(greatPoint313, greatPoint314)
greatEdgeZ413 = MakeEdge(greatPoint413, greatPoint414)

greatEdgeZ123 = MakeEdge(greatPoint123, greatPoint124)
greatEdgeZ223 = MakeEdge(greatPoint223, greatPoint224)
greatEdgeZ323 = MakeEdge(greatPoint323, greatPoint324)
greatEdgeZ423 = MakeEdge(greatPoint423, greatPoint424)

# Faces
# -----

smallFaceX1 = MakeQuad(smallEdgeY11, smallEdgeZ11, smallEdgeY12, smallEdgeZ12)
smallFaceX2 = MakeQuad(smallEdgeY21, smallEdgeZ21, smallEdgeY22, smallEdgeZ22)
smallFaceY1 = MakeQuad(smallEdgeX11, smallEdgeZ11, smallEdgeX12, smallEdgeZ21)
smallFaceY2 = MakeQuad(smallEdgeX21, smallEdgeZ12, smallEdgeX22, smallEdgeZ22)
smallFaceZ1 = MakeQuad(smallEdgeX11, smallEdgeY11, smallEdgeX21, smallEdgeY21)
smallFaceZ2 = MakeQuad(smallEdgeX12, smallEdgeY12, smallEdgeX22, smallEdgeY22)


greatFaceX11 = MakeQuad(greatEdgeY11, greatEdgeZ111, greatEdgeY12, greatEdgeZ121)
greatFaceX21 = MakeQuad(greatEdgeY21, greatEdgeZ211, greatEdgeY22, greatEdgeZ221)
greatFaceX31 = MakeQuad(greatEdgeY31, greatEdgeZ311, greatEdgeY32, greatEdgeZ321)
greatFaceX41 = MakeQuad(greatEdgeY41, greatEdgeZ411, greatEdgeY42, greatEdgeZ421)

greatFaceX12 = MakeQuad(greatEdgeY12, greatEdgeZ112, greatEdgeY13, greatEdgeZ122)
greatFaceX22 = MakeQuad(greatEdgeY22, greatEdgeZ212, greatEdgeY23, greatEdgeZ222)
greatFaceX32 = MakeQuad(greatEdgeY32, greatEdgeZ312, greatEdgeY33, greatEdgeZ322)
greatFaceX42 = MakeQuad(greatEdgeY42, greatEdgeZ412, greatEdgeY43, greatEdgeZ422)

greatFaceX13 = MakeQuad(greatEdgeY13, greatEdgeZ113, greatEdgeY14, greatEdgeZ123)
greatFaceX23 = MakeQuad(greatEdgeY23, greatEdgeZ213, greatEdgeY24, greatEdgeZ223)
greatFaceX33 = MakeQuad(greatEdgeY33, greatEdgeZ313, greatEdgeY34, greatEdgeZ323)
greatFaceX43 = MakeQuad(greatEdgeY43, greatEdgeZ413, greatEdgeY44, greatEdgeZ423)

greatFaceY111 = MakeQuad(greatEdgeX111, greatEdgeZ111, greatEdgeX112, greatEdgeZ211)
greatFaceY211 = MakeQuad(greatEdgeX211, greatEdgeZ211, greatEdgeX212, greatEdgeZ311)
greatFaceY311 = MakeQuad(greatEdgeX311, greatEdgeZ311, greatEdgeX312, greatEdgeZ411)
greatFaceY121 = MakeQuad(greatEdgeX121, greatEdgeZ121, greatEdgeX122, greatEdgeZ221)
greatFaceY221 = MakeQuad(greatEdgeX221, greatEdgeZ221, greatEdgeX222, greatEdgeZ321)
greatFaceY321 = MakeQuad(greatEdgeX321, greatEdgeZ321, greatEdgeX322, greatEdgeZ421)

greatFaceY112 = MakeQuad(greatEdgeX112, greatEdgeZ112, greatEdgeX113, greatEdgeZ212)
greatFaceY212 = MakeQuad(greatEdgeX212, greatEdgeZ212, greatEdgeX213, greatEdgeZ312)
greatFaceY312 = MakeQuad(greatEdgeX312, greatEdgeZ312, greatEdgeX313, greatEdgeZ412)
greatFaceY122 = MakeQuad(greatEdgeX122, greatEdgeZ122, greatEdgeX123, greatEdgeZ222)
greatFaceY222 = smallFaceY1
greatFaceY322 = MakeQuad(greatEdgeX322, greatEdgeZ322, greatEdgeX323, greatEdgeZ422)

greatFaceY113 = MakeQuad(greatEdgeX113, greatEdgeZ113, greatEdgeX114, greatEdgeZ213)
greatFaceY213 = MakeQuad(greatEdgeX213, greatEdgeZ213, greatEdgeX214, greatEdgeZ313)
greatFaceY313 = MakeQuad(greatEdgeX313, greatEdgeZ313, greatEdgeX314, greatEdgeZ413)
greatFaceY123 = MakeQuad(greatEdgeX123, greatEdgeZ123, greatEdgeX124, greatEdgeZ223)
greatFaceY223 = MakeQuad(greatEdgeX223, greatEdgeZ223, greatEdgeX224, greatEdgeZ323)
greatFaceY323 = MakeQuad(greatEdgeX323, greatEdgeZ323, greatEdgeX324, greatEdgeZ423)

greatFaceZ11 = MakeQuad(greatEdgeX111, greatEdgeY11, greatEdgeX121, greatEdgeY21)
greatFaceZ21 = MakeQuad(greatEdgeX211, greatEdgeY21, greatEdgeX221, greatEdgeY31)
greatFaceZ31 = MakeQuad(greatEdgeX311, greatEdgeY31, greatEdgeX321, greatEdgeY41)

greatFaceZ12 = MakeQuad(greatEdgeX112, greatEdgeY12, greatEdgeX122, greatEdgeY22)
greatFaceZ22 = MakeQuad(greatEdgeX212, greatEdgeY22, greatEdgeX222, greatEdgeY32)
greatFaceZ32 = MakeQuad(greatEdgeX312, greatEdgeY32, greatEdgeX322, greatEdgeY42)

greatFaceZ13 = MakeQuad(greatEdgeX113, greatEdgeY13, greatEdgeX123, greatEdgeY23)
greatFaceZ23 = MakeQuad(greatEdgeX213, greatEdgeY23, greatEdgeX223, greatEdgeY33)
greatFaceZ33 = MakeQuad(greatEdgeX313, greatEdgeY33, greatEdgeX323, greatEdgeY43)

greatFaceZ14 = MakeQuad(greatEdgeX114, greatEdgeY14, greatEdgeX124, greatEdgeY24)
greatFaceZ24 = MakeQuad(greatEdgeX214, greatEdgeY24, greatEdgeX224, greatEdgeY34)
greatFaceZ34 = MakeQuad(greatEdgeX314, greatEdgeY34, greatEdgeX324, greatEdgeY44)

# Solids
# ------

smallBlock   = MakeHexa(smallFaceX1, smallFaceX2, smallFaceY1, smallFaceY2, smallFaceZ1, smallFaceZ2)

greatBlock11 = MakeHexa(greatFaceX11, greatFaceX21, greatFaceY111, greatFaceY121, greatFaceZ11, greatFaceZ12)
greatBlock21 = MakeHexa(greatFaceX21, greatFaceX31, greatFaceY211, greatFaceY221, greatFaceZ21, greatFaceZ22)
greatBlock31 = MakeHexa(greatFaceX31, greatFaceX41, greatFaceY311, greatFaceY321, greatFaceZ31, greatFaceZ32)

greatBlock12 = MakeHexa(greatFaceX12, greatFaceX22, greatFaceY112, greatFaceY122, greatFaceZ12, greatFaceZ13)
greatBlock22 = MakeHexa(greatFaceX22, greatFaceX32, greatFaceY212, greatFaceY222, greatFaceZ22, greatFaceZ23)
greatBlock32 = MakeHexa(greatFaceX32, greatFaceX42, greatFaceY312, greatFaceY322, greatFaceZ32, greatFaceZ33)

greatBlock13 = MakeHexa(greatFaceX13, greatFaceX23, greatFaceY113, greatFaceY123, greatFaceZ13, greatFaceZ14)
greatBlock23 = MakeHexa(greatFaceX23, greatFaceX33, greatFaceY213, greatFaceY223, greatFaceZ23, greatFaceZ24)
greatBlock33 = MakeHexa(greatFaceX33, greatFaceX43, greatFaceY313, greatFaceY323, greatFaceZ33, greatFaceZ34)

# Compound
# --------

c_l = []
c_l.append(smallBlock)
c_l.append(greatBlock11)
c_l.append(greatBlock21)
c_l.append(greatBlock31)
c_l.append(greatBlock12)
c_l.append(greatBlock22)
c_l.append(greatBlock32)
c_l.append(greatBlock13)
c_l.append(greatBlock23)
c_l.append(greatBlock33)

piece = MakeCompound(c_l)

# Add in study
# ------------

piece_id = addToStudy(piece, "ex01_cube2build")

# Meshing
# =======

smesh.SetCurrentStudy(salome.myStudy)

# Create hexahedrical mesh on piece
# ---------------------------------

hexa = smesh.Mesh(piece, "ex01_cube2build:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(4)

hexa.Quadrangle()

hexa.Hexahedron()

# Create local hypothesis
# -----------------------

algo = hexa.Segment(greatEdgeX111)

algo.Arithmetic1D(1, 4)

algo.Propagation()

# Compute the mesh
# ----------------

hexa.Compute()
