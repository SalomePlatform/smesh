#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
#
#
#
#  File   : SMESH_test3.py
#  Module : SMESH

import smeshpy
import salome
from salome import sg
import math
#import SMESH_BasicHypothesis_idl

import geompy

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
from geompy import gg

ShapeTypeCompSolid = 1
ShapeTypeSolid = 2
ShapeTypeShell = 3
ShapeTypeFace = 4
ShapeTypeWire = 5
ShapeTypeEdge = 6
ShapeTypeVertex = 7

pi=math.pi

# ---------------------------------------------
xa=math.sin(pi/12)
ya=0
za=math.cos(pi/12)

xb=0
yb=math.sin(pi/18)
zb=math.cos(pi/18)

xc=math.cos(-pi/18)
yc=0
zc=math.sin(-pi/18)

rc1=150
hc1=300
rc2=150
rc3=150
rc4=300
# ---------------------------------------------
point_0 = geom.MakePointStruct(0, 0, 0)
point_z = geom.MakePointStruct(0, 0, 1)

point_a = geom.MakePointStruct(xa, ya, za)
point_b = geom.MakePointStruct(xb, yb, zb)
point_c = geom.MakePointStruct(xc, yc, zc)

dir_z = geom.MakeDirection(point_z)
axe_z = geom.MakeAxisStruct(0, 0, 0, 0, 0, 1)

dir_a = geom.MakeDirection(point_a)
axe_a = geom.MakeAxisStruct(0, 0, 0, xa, ya, za)

dir_b = geom.MakeDirection(point_b)
axe_b = geom.MakeAxisStruct(0, 0, 0, xb, yb, zb)

dir_c = geom.MakeDirection(point_c)
axe_c = geom.MakeAxisStruct(0, 0, 0, xc, yc, zc)

cyl_1 = geompy.MakeCylinder(point_0, dir_z, rc1, hc1)

hc2=2*hc1
cyl_t = geompy.MakeCylinder(point_0, dir_a, rc2, hc2)
cyl_a = geompy.MakeTranslation(cyl_t, 1.2*rc1, 0.1*rc1, -0.5*hc1)

hc3=2*hc1
cyl_t = geompy.MakeCylinder(point_0, dir_b, rc3, hc3)
cyl_b = geompy.MakeTranslation(cyl_t, -1.2*rc1, -0.1*rc1, -0.5*hc1)

hc4=2*hc1
cyl_t = geompy.MakeCylinder(point_0, dir_c, rc4, hc4)
cyl_t = geompy.MakeRotation(cyl_t, axe_c, pi/2)
cyl_c = geompy.MakeTranslation(cyl_t, -hc1, 0, 0)
cyl_d = geompy.MakeTranslation(cyl_t, -hc1, 0, 1.3*rc4)

inter_t = geompy.MakeBoolean(cyl_c,cyl_d, 1) # common

blob_t = geompy.MakeBoolean(cyl_1, cyl_a, 2) # cut
blob_t = geompy.MakeBoolean(blob_t, cyl_b, 2)

blob = geompy.MakeBoolean(blob_t, inter_t, 1) # common

idblob = geompy.addToStudy(blob,"blob")
#idc = geompy.addToStudy(cyl_c,"cyl_c")
#idd = geompy.addToStudy(cyl_d,"cyl_d")
