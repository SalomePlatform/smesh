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
#  File   : SMESH_fix_volute.py
#  Author : Paul RASCLE, EDF
#  Module : SMESH
#  $Header$

import salome
import geompy
import math

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")

ShapeTypeCompSolid = 1
ShapeTypeSolid     = 2
ShapeTypeShell     = 3
ShapeTypeFace      = 4
ShapeTypeWire      = 5
ShapeTypeEdge      = 6
ShapeTypeVertex    = 7

# -----------------------------------------------------------------------------

def MakeFace(lstEdges) :
    """
    Creates a face from 4 edges
    """
    lstWire = []
    for edge in lstEdges :
        lstWire.append(edge._get_Name())
    wire = geom.MakeWire(lstWire)
    face = geom.MakeFace(wire, 1)
    return face

def MakeShell(lstFaces) :
    """
    Creates a shell from 6 faces
    """
    lstShell = []
    for face in lstFaces :
        lstShell.append(face._get_Name())
    shell = geom.MakeSewing(lstShell, 0.00001)
    return shell

def MakeCompound(lstShells) :
    """
    Creates a compound from several shells
    """
    lstCompound = []
    for shell in lstShells :
        lstCompound.append(shell._get_Name())
    compound = geom.MakeCompound(lstCompound)
    return compound

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
posAxeTrou = hauteurFlanc -(180 + rayonTrou)
marge = 10
tol3d = 1.e-3

# ---- points, directions de base

p0 = geom.MakePointStruct(0., 0., 0.)
px = geom.MakePointStruct(100., 0., 0.)
vx = geom.MakeDirection(px)
py = geom.MakePointStruct(0., 100., 0.)
vy = geom.MakeDirection(py)
pz = geom.MakePointStruct(0., 0., 100.)
vz = geom.MakeDirection(pz)

# ---- ellipse du flanc

he = hauteurFlanc -2*rayonConge
re = 0.5*(largeurPlq - epaisseurFond) - rayonConge
sine = re/he
cose = math.sqrt(1.-sine*sine)
pe = geom.MakePointStruct(sine, 0., cose)
ve = geom.MakeDirection(pe)
cyl0 = geom.MakeCylinder(p0, ve, re, 2*he)
axecy = geom.MakeAxisStruct(0., 0., 0.,sine, 0., cose)
cyl1 = geom.MakeRotation(cyl0, axecy, 0.5)
cyle = geom.MakeTranslation(cyl1,
                            -marge*sine, 0., -marge*cose)
boxe = geom.MakeBox(0., 0., 0., 3*he, -2*re, 3*he)
#idcyle = geompy.addToStudy(cyle, "cyle")
#idboxe = geompy.addToStudy(boxe, "boxe")
cylcoup = geom.MakeBoolean(cyle, boxe, 2)
#idcylcoup = geompy.addToStudy(cylcoup, "cylcoup")
aretes = []
aretes = geom.SubShapeAllSorted(cylcoup, ShapeTypeEdge)

##eid=0
##for edge in aretes:
##    edname="arete%d"%eid
##    idedge=geompy.addToStudy(edge,edname)
##    eid=eid+1
    
shape = geom.MakeCopy(aretes[0])
#idarete = geompy.addToStudy(shape, "arete")
aShape = geom.MakeTranslation(shape,
                              0., rayonConge +re, epaisseurPlq +2*rayonConge)


# -----------------------------------------------------------------------------
# ---- decoupage de la piece en volumes a 6 faces de 4 cotes
# -----------------------------------------------------------------------------

# ---- cotes x

x0 = 0.
x0h = rayonConge
x1 = rayonConge + epaisseurFlanc
xc = longueurPlq/2
x2 = longueurPlq - rayonConge - epaisseurFlanc
x3h = longueurPlq - rayonConge
x3 = longueurPlq

# ---- cotes y

y0 = 0.
y0h = rayonConge
y1 = largeurPlq - epaisseurFond
y1m = y1 -marge
y2 = largeurPlq
y2p = largeurPlq + marge

# ---- cotes z

z0 = 0.
z1m = epaisseurPlq -marge
z1 = epaisseurPlq
z2 = epaisseurPlq + rayonConge
z3 = epaisseurPlq + 2*rayonConge
z4 = epaisseurPlq + hauteurFlanc
z4p = epaisseurPlq + hauteurFlanc + marge

zc = epaisseurPlq + posAxeTrou
zc2 = epaisseurPlq + (posAxeTrou -rayonTrou)/3
zc3 = epaisseurPlq + 2*(posAxeTrou -rayonTrou)/3

# ---- decoupe du fond

p11 = geom.MakePointStruct(x1, y1m, z1)
p12 = geom.MakePointStruct(x1, y1m, z2)
p13 = geom.MakePointStruct(x1, y1m, z3)
p14 = geom.MakePointStruct(x1, y1m, z4)
pc1 = geom.MakePointStruct(xc, y1m, z1)
pc2 = geom.MakePointStruct(xc, y1m, zc2)
pc3 = geom.MakePointStruct(xc, y1m, zc3)
pcc = geom.MakePointStruct(xc, y1m, zc)
pc4 = geom.MakePointStruct(xc, y1m, z4)
p21 = geom.MakePointStruct(x2, y1m, z1)
p22 = geom.MakePointStruct(x2, y1m, z2)
p23 = geom.MakePointStruct(x2, y1m, z3)
p24 = geom.MakePointStruct(x2, y1m, z4)
pcf = geom.MakePointStruct(xc, y2p, zc)

arc2 = geom.MakeArc(p12,pc2,p22)
arc3 = geom.MakeArc(p13,pc3,p23)

segz1 = geom.MakeVector(p11,p21)
#segz4 = geom.MakeVector(p14,p24)
segz41 = geom.MakeVector(p14,pc4)
segz42 = geom.MakeVector(pc4,p24)
segx11 = geom.MakeVector(p11,p12)
segx12 = geom.MakeVector(p12,p13)
segx13 = geom.MakeVector(p13,p14)
segxc2 = geom.MakeVector(pc1,pc2)
segxc3 = geom.MakeVector(pc2,pc3)
segxc4 = geom.MakeVector(pcc,pc4)
segx21 = geom.MakeVector(p21,p22)
segx22 = geom.MakeVector(p22,p23)
segx23 = geom.MakeVector(p23,p24)
segx1c1 = geom.MakeVector(p13,pcc)
segx1c2 = geom.MakeVector(p14,pcc)
segx2c1 = geom.MakeVector(p23,pcc)
segx2c2 = geom.MakeVector(p24,pcc)

facef = []
facef.append(MakeFace([segx13,segx1c2,segx1c1]))
facef.append(MakeFace([segx23,segx2c2,segx2c1]))
facef.append(MakeFace([segx2c2,segxc4,segz42]))
facef.append(MakeFace([segx1c2,segz41,segxc4]))
facef.append(MakeFace([segx1c1,arc3,segx2c1]))
facef.append(MakeFace([segx12,arc2,segx22,arc3]))
facef.append(MakeFace([segx11,segz1,segx21,arc2]))

decf =[]
for face in facef:
    decf.append(geom.MakePrism(face,pcc,pcf))

axeCyl = geom.MakeAxisStruct( 0.5*longueurPlq,
                              0.,
                              epaisseurPlq + posAxeTrou,
                              0.,
                              largeurPlq,
                              0.)
pAxe1 = geom.MakePointStruct(xc, 0., zc)
cylFond = geom.MakeCylinder(pAxe1, vy, rayonTrou, 1.1*largeurPlq)
cylFond2 = geom.MakeRotation(geom.MakeCopy(cylFond),axeCyl,math.pi)
#idcylfond2 = geompy.addToStudy(cylFond2,"cylFond2")

fondec =[]
for id in (0,1,2,3):
    fondec.append(geom.MakeBoolean(decf[id],cylFond2,2))
fondec.append(geom.MakeBoolean(decf[4],cylFond,2))
for id in (5,6):
    fondec.append(decf[id])

bcut1=geom.MakeBox(x0,y0,z0, xc,y2p,z4p)
bcut2=geom.MakeBox(xc,y0,z0, x3,y2p,z4p)
fondec2 = []
for id in (0,1,2,3):
    fondec2.append(fondec[id])
for id in (4,5,6):
    fondec2.append(geom.MakeBoolean(fondec[id],bcut1,1))
    fondec2.append(geom.MakeBoolean(fondec[id],bcut2,1))

##iff=0
##for ff in fondec2:
##   idfo = geompy.addToStudy(ff, "ff%d"%(iff))
##   iff = iff +1

# ----- autres blocs de decoupe

bcong1=geom.MakeBox(x0,y0,z1, x1,y1,z2)
bcong2=geom.MakeBox(x0,y1,z1, x1,y2,z2)
bcong3=geom.MakeBox(x2,y0,z1, x3,y1,z2)
bcong4=geom.MakeBox(x2,y1,z1, x3,y2,z2)

pcylx0 = geom.MakePointStruct(0., -marge, z2)
cylcongx0 = geom.MakeCylinder(pcylx0, vy, rayonConge, largeurPlq +2*marge)
#idcylcongx0 = geompy.addToStudy(cylcongx0,"cylcongx0")
pcylx3 = geom.MakePointStruct(longueurPlq, -marge, z2)
cylcongx3 = geom.MakeCylinder(pcylx3, vy, rayonConge, largeurPlq +2*marge)
#idcylcongx3 = geompy.addToStudy(cylcongx3,"cylcongx3")
pcyly0 = geom.MakePointStruct(-marge, 0., z2)
cylcongy0 = geom.MakeCylinder(pcyly0, vx, rayonConge, longueurPlq +2*marge)
#idcylcongy0 = geompy.addToStudy(cylcongy0,"cylcongy0")

bcong1=geom.MakeBoolean(bcong1,cylcongx0,2)
bcong2=geom.MakeBoolean(bcong2,cylcongx0,2)
bcong1=geom.MakeBoolean(bcong1,cylcongy0,2)
#NRI : inverse order of BOP
bcong3=geom.MakeBoolean(bcong3,cylcongy0,2)
bcong3=geom.MakeBoolean(bcong3,cylcongx3,2)
bcong4=geom.MakeBoolean(bcong4,cylcongx3,2)

pf1 = geom.MakePointStruct(0., y0h, z3)
pf2 = geom.MakePointStruct(0., y1, z3)
pf3 = geom.MakePointStruct(0., y1, z4)
pf4 = geom.MakePointStruct(0.,0.5*(largeurPlq - epaisseurFond) , z4)
vf1 = geom.MakeEdge(pf1,pf2)
vf2 = geom.MakeEdge(pf2,pf3)
vf3 = geom.MakeEdge(pf3,pf4)
faceFlanc = MakeFace([vf1,vf2,vf3,aShape])
#idfaceFlanc = geompy.addToStudy(faceFlanc,"faceFlanc")
pfe = geom.MakePointStruct(epaisseurFlanc, 0., 0.)
flanc1 = geom.MakePrism(faceFlanc, p0, pfe)
flanc2 = geom.MakeCopy(flanc1)
flanc1 = geom.MakeTranslation(flanc1,
                              rayonConge,0.,0.)
flanc2 = geom.MakeTranslation(flanc2,
                              longueurPlq-rayonConge-epaisseurFlanc,0.,0.)

# ---- constitution et decoupe des blocs
boxfond2 = geom.MakeBox(x0, y1, z0, x3, y2, z4p)
#idboxfond2 = geompy.addToStudy(boxfond2,"boxfond2")

blocs = []
for dec in fondec2:
    blocs.append(geom.MakeBoolean(boxfond2, dec, 1))
blocs.append(geom.MakeBox(x0,y1,z0, x1,y2,z1))
blocs.append(geom.MakeBox(x1,y1,z0, xc,y2,z1))
blocs.append(geom.MakeBox(xc,y1,z0, x2,y2,z1))
blocs.append(geom.MakeBox(x2,y1,z0, x3,y2,z1))
blocs.append(geom.MakeBox(x0,y0,z0, x1,y1,z1))
blocs.append(geom.MakeBox(x1,y0,z0, xc,y1,z1))
blocs.append(geom.MakeBox(xc,y0,z0, x2,y1,z1))
blocs.append(geom.MakeBox(x2,y0,z0, x3,y1,z1))
blocs.append(bcong2)
blocs.append(bcong4)
blocs.append(bcong1)
blocs.append(bcong3)
blocs.append(geom.MakeBox(x0h,y1, z2, x1, y2, z3))
blocs.append(geom.MakeBox(x2, y1, z2, x3h,y2, z3))
blocs.append(geom.MakeBox(x0h,y0h,z2, x1, y1, z3))
blocs.append(geom.MakeBox(x2, y0h,z2, x3h,y1, z3))
blocs.append(geom.MakeBox(x0h,y1, z3, x1, y2, z4))
blocs.append(geom.MakeBox(x2, y1, z3, x3h,y2, z4))
blocs.append(flanc1)
blocs.append(flanc2)
 
compbloc = MakeCompound(blocs)
idcomp = geompy.addToStudy(compbloc,"compbloc")

# ---- eliminer les faces en double, solid-->shell

compshell = geom.MakeGlueFaces(compbloc,tol3d)
idcomp = geompy.addToStudy(compshell,"compshell")

