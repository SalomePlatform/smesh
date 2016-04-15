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

#  File   : SMESH_fix_volute.py
#  Author : Paul RASCLE, EDF
#  Module : SMESH
#  $Header$
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import math

# -----------------------------------------------------------------------------

def MakeFace(lstEdges) :
    """
    Creates a planar face from 4 edges
    """
    wire = geompy.MakeWire(lstEdges)
    face = geompy.MakeFace(wire, 1)
    return face

# -----------------------------------------------------------------------------

# ---- dimensions

##longueurPlq  = 0.686
##largeurPlq   = 0.573
##epaisseurPlq = 0.150

##hauteurFlanc   = 0.380
##epaisseurFlanc = 0.112
##rayonConge   = 0.150 - epaisseurFlanc

##epaisseurFond = 0.162
##rayonTrou = 0.075
##posAxeTrou = hauteurFlanc -(0.180 + rayonTrou)
##marge = 0.01
##tol3d = 1.e-5

longueurPlq  = 686
largeurPlq   = 573
epaisseurPlq = 150

hauteurFlanc   = 380
epaisseurFlanc = 112
rayonConge   = 150 - epaisseurFlanc

epaisseurFond = 162
rayonTrou = 75
posAxeTrou = hauteurFlanc - (180 + rayonTrou)
marge = 10
tol3d = 1.e-3

# ---- points, directions de base

p0 = geompy.MakeVertex(0., 0., 0.)

vx = geompy.MakeVectorDXDYDZ(100., 0., 0.)
vy = geompy.MakeVectorDXDYDZ(0., 100., 0.)
vz = geompy.MakeVectorDXDYDZ(0., 0., 100.)

# ---- ellipse du flanc

he = hauteurFlanc - 2*rayonConge
re = 0.5*(largeurPlq - epaisseurFond) - rayonConge
sine = re/he
cose = math.sqrt(1. - sine*sine)

ve = geompy.MakeVectorDXDYDZ(sine, 0., cose)
cyl0 = geompy.MakeCylinder(p0, ve, re, 2*he)
cyl1 = geompy.MakeRotation(cyl0, ve, 0.5)
cyle = geompy.MakeTranslation(cyl1, -marge*sine, 0., -marge*cose)

pbe = geompy.MakeVertex(3*he, -2*re, 3*he)
boxe = geompy.MakeBoxTwoPnt(p0, pbe)

cylcoup = geompy.MakeBoolean(cyle, boxe, 2)

aretes = []
aretes = geompy.SubShapeAllSorted(cylcoup, geompy.ShapeType["EDGE"])

shape = geompy.MakeCopy(aretes[0])
aShape = geompy.MakeTranslation(shape, 0., rayonConge + re, epaisseurPlq + 2*rayonConge)


# -----------------------------------------------------------------------------
# ---- decoupage de la piece en volumes a 6 faces de 4 cotes
# -----------------------------------------------------------------------------

# ---- cotes x

x0  = 0.
x0h = rayonConge
x1  = rayonConge + epaisseurFlanc
xc  = longueurPlq/2
x2  = longueurPlq - rayonConge - epaisseurFlanc
x3h = longueurPlq - rayonConge
x3  = longueurPlq

# ---- cotes y

y0  = 0.
y0h = rayonConge
y1  = largeurPlq - epaisseurFond
y1m = y1 - marge
y2  = largeurPlq
y2p = largeurPlq + marge

# ---- cotes z

z0  = 0.
z1m = epaisseurPlq - marge
z1  = epaisseurPlq
z2  = epaisseurPlq + rayonConge
z3  = epaisseurPlq + 2*rayonConge
z4  = epaisseurPlq + hauteurFlanc
z4p = epaisseurPlq + hauteurFlanc + marge

zc  = epaisseurPlq + posAxeTrou
zc2 = epaisseurPlq + (posAxeTrou - rayonTrou)/3
zc3 = epaisseurPlq + 2*(posAxeTrou - rayonTrou)/3

# ---- decoupe du fond

p11 = geompy.MakeVertex(x1, y1m, z1)
p12 = geompy.MakeVertex(x1, y1m, z2)
p13 = geompy.MakeVertex(x1, y1m, z3)
p14 = geompy.MakeVertex(x1, y1m, z4)
pc1 = geompy.MakeVertex(xc, y1m, z1)
pc2 = geompy.MakeVertex(xc, y1m, zc2)
pc3 = geompy.MakeVertex(xc, y1m, zc3)
pcc = geompy.MakeVertex(xc, y1m, zc)
pc4 = geompy.MakeVertex(xc, y1m, z4)
p21 = geompy.MakeVertex(x2, y1m, z1)
p22 = geompy.MakeVertex(x2, y1m, z2)
p23 = geompy.MakeVertex(x2, y1m, z3)
p24 = geompy.MakeVertex(x2, y1m, z4)
pcf = geompy.MakeVertex(xc, y2p, zc)

arc2 = geompy.MakeArc(p12,pc2,p22)
arc3 = geompy.MakeArc(p13,pc3,p23)

segz1  = geompy.MakeVector(p11,p21)
segz41 = geompy.MakeVector(p14,pc4)
segz42 = geompy.MakeVector(pc4,p24)
segx11 = geompy.MakeVector(p11,p12)
segx12 = geompy.MakeVector(p12,p13)
segx13 = geompy.MakeVector(p13,p14)
segxc2 = geompy.MakeVector(pc1,pc2)
segxc3 = geompy.MakeVector(pc2,pc3)
segxc4 = geompy.MakeVector(pcc,pc4)
segx21 = geompy.MakeVector(p21,p22)
segx22 = geompy.MakeVector(p22,p23)
segx23 = geompy.MakeVector(p23,p24)
segx1c1 = geompy.MakeVector(p13,pcc)
segx1c2 = geompy.MakeVector(p14,pcc)
segx2c1 = geompy.MakeVector(p23,pcc)
segx2c2 = geompy.MakeVector(p24,pcc)

facef = []
facef.append(MakeFace([segx13,segx1c2,segx1c1]))
facef.append(MakeFace([segx23,segx2c2,segx2c1]))
facef.append(MakeFace([segx2c2,segxc4,segz42]))
facef.append(MakeFace([segx1c2,segz41,segxc4]))
facef.append(MakeFace([segx1c1,arc3,segx2c1]))
facef.append(MakeFace([segx12,arc2,segx22,arc3]))
facef.append(MakeFace([segx11,segz1,segx21,arc2]))

vcccf = geompy.MakeVector(pcc, pcf)
hcccf = y2p - y1m
decf = []
for face in facef:
    decf.append(geompy.MakePrismVecH(face,vcccf,hcccf))

pc  = geompy.MakeVertex(xc, 0., zc)
py2 = geompy.MakeVertex(xc, y2, zc)
axeCyl = geompy.MakeVector(pc, py2)

cylFond = geompy.MakeCylinder(pc, vy, rayonTrou, 1.1*largeurPlq)
cylFond2 = geompy.MakeRotation(cylFond, axeCyl, math.pi)

fondec = []
for id in (0,1,2,3):
    fondec.append(geompy.MakeBoolean(decf[id], cylFond2, 2))
fondec.append(geompy.MakeBoolean(decf[4], cylFond, 2))
for id in (5,6):
    fondec.append(decf[id])

p_xcy2pz4p = geompy.MakeVertex(xc,y2p,z4p)
p_x3y2pz4p = geompy.MakeVertex(x3,y2p,z4p)
pxc = geompy.MakeVertex(xc,y0,z0)
bcut1 = geompy.MakeBoxTwoPnt(p0, p_xcy2pz4p)
bcut2 = geompy.MakeBoxTwoPnt(pxc, p_x3y2pz4p)

fondec2 = []
for id in (0,1,2,3):
    fondec2.append(fondec[id])
for id in (4,5,6):
    fondec2.append(geompy.MakeBoolean(fondec[id], bcut1, 1))
    fondec2.append(geompy.MakeBoolean(fondec[id], bcut2, 1))

# ----- autres blocs de decoupe

bcong1 = geompy.MakeBox(x0,y0,z1, x1,y1,z2)
bcong2 = geompy.MakeBox(x0,y1,z1, x1,y2,z2)
bcong3 = geompy.MakeBox(x2,y0,z1, x3,y1,z2)
bcong4 = geompy.MakeBox(x2,y1,z1, x3,y2,z2)

pcylx0 = geompy.MakeVertex(0., -marge, z2)
pcylx3 = geompy.MakeVertex(longueurPlq, -marge, z2)
pcyly0 = geompy.MakeVertex(-marge, 0., z2)

cylcongx0 = geompy.MakeCylinder(pcylx0, vy, rayonConge, largeurPlq + 2*marge)
cylcongx3 = geompy.MakeCylinder(pcylx3, vy, rayonConge, largeurPlq + 2*marge)
cylcongy0 = geompy.MakeCylinder(pcyly0, vx, rayonConge, longueurPlq + 2*marge)

bcong1 = geompy.MakeBoolean(bcong1,cylcongx0,2)
bcong2 = geompy.MakeBoolean(bcong2,cylcongx0,2)
bcong1 = geompy.MakeBoolean(bcong1,cylcongy0,2)
#NRI : inverse order of BOP
bcong3 = geompy.MakeBoolean(bcong3,cylcongy0,2)
bcong3 = geompy.MakeBoolean(bcong3,cylcongx3,2)
bcong4 = geompy.MakeBoolean(bcong4,cylcongx3,2)

pf1 = geompy.MakeVertex(0., y0h, z3)
pf2 = geompy.MakeVertex(0., y1, z3)
pf3 = geompy.MakeVertex(0., y1, z4)
pf4 = geompy.MakeVertex(0., 0.5*(largeurPlq - epaisseurFond) , z4)

vf1 = geompy.MakeEdge(pf1, pf2)
vf2 = geompy.MakeEdge(pf2, pf3)
vf3 = geompy.MakeEdge(pf3, pf4)

faceFlanc = MakeFace([vf1,vf2,vf3,aShape])

flanc1 = geompy.MakePrismVecH(faceFlanc, vx, epaisseurFlanc)
flanc2 = geompy.MakeCopy(flanc1)
flanc1 = geompy.MakeTranslation(flanc1, rayonConge, 0., 0.)
flanc2 = geompy.MakeTranslation(flanc2, longueurPlq - rayonConge - epaisseurFlanc, 0., 0.)

# ---- constitution et decoupe des blocs
boxfond2 = geompy.MakeBox(x0, y1, z0, x3, y2, z4p)

blocs = []
for dec in fondec2:
    blocs.append(geompy.MakeBoolean(boxfond2, dec, 1))

blocs.append(geompy.MakeBox(x0,y1,z0, x1,y2,z1))
blocs.append(geompy.MakeBox(x1,y1,z0, xc,y2,z1))
blocs.append(geompy.MakeBox(xc,y1,z0, x2,y2,z1))
blocs.append(geompy.MakeBox(x2,y1,z0, x3,y2,z1))
blocs.append(geompy.MakeBox(x0,y0,z0, x1,y1,z1))
blocs.append(geompy.MakeBox(x1,y0,z0, xc,y1,z1))
blocs.append(geompy.MakeBox(xc,y0,z0, x2,y1,z1))
blocs.append(geompy.MakeBox(x2,y0,z0, x3,y1,z1))
blocs.append(bcong2)
blocs.append(bcong4)
blocs.append(bcong1)
blocs.append(bcong3)
blocs.append(geompy.MakeBox(x0h,y1, z2, x1, y2, z3))
blocs.append(geompy.MakeBox(x2, y1, z2, x3h,y2, z3))
blocs.append(geompy.MakeBox(x0h,y0h,z2, x1, y1, z3))
blocs.append(geompy.MakeBox(x2, y0h,z2, x3h,y1, z3))
blocs.append(geompy.MakeBox(x0h,y1, z3, x1, y2, z4))
blocs.append(geompy.MakeBox(x2, y1, z3, x3h,y2, z4))
blocs.append(flanc1)
blocs.append(flanc2)

compbloc = geompy.MakeCompound(blocs)
idcomp = geompy.addToStudy(compbloc, "compbloc")

# ---- eliminer les faces en double, solid-->shell

compshell = geompy.MakeGlueFaces(compbloc,tol3d)
idcomp = geompy.addToStudy(compshell, "compshell")
