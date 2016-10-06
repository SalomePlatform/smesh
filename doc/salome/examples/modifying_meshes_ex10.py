# Add Polyhedron


import salome
salome.salome_init()

from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import math

# create an empty mesh structure
mesh = smesh.Mesh()  

# Create nodes for 12-hedron with pentagonal faces
al = 2 * math.pi / 5.0
cosal = math.cos(al)
aa = 13
rr = aa / (2.0 * math.sin(al/2.0))
dr = 2.0 * rr * cosal
r1 = rr + dr
dh = rr * math.sqrt(2.0 * (1.0 - cosal * (1.0 + 2.0 * cosal)))
hh = 2.0 * dh - dr * (rr*(cosal - 1) + (rr + dr)*(math.cos(al/2) - 1)) / dh

dd = [] # top
cc = [] # below top
bb = [] # above bottom
aa = [] # bottom

for i in range(5):
    cos_bot = math.cos(i*al)
    sin_bot = math.sin(i*al)

    cos_top = math.cos(i*al + al/2.0)
    sin_top = math.sin(i*al + al/2.0)

    nd = mesh.AddNode(rr * cos_top, rr * sin_top, hh     ) # top
    nc = mesh.AddNode(r1 * cos_top, r1 * sin_top, hh - dh) # below top
    nb = mesh.AddNode(r1 * cos_bot, r1 * sin_bot,      dh) # above bottom
    na = mesh.AddNode(rr * cos_bot, rr * sin_bot,       0) # bottom
    dd.append(nd) # top
    cc.append(nc) # below top
    bb.append(nb) # above bottom
    aa.append(na) # bottom
    pass

# Create a polyhedral volume (12-hedron with pentagonal faces)
mesh.AddPolyhedralVolume([dd[0], dd[1], dd[2], dd[3], dd[4],  # top
                          dd[0], cc[0], bb[1], cc[1], dd[1],  # -
                          dd[1], cc[1], bb[2], cc[2], dd[2],  # -
                          dd[2], cc[2], bb[3], cc[3], dd[3],  # - below top
                          dd[3], cc[3], bb[4], cc[4], dd[4],  # -
                          dd[4], cc[4], bb[0], cc[0], dd[0],  # -
                          aa[4], bb[4], cc[4], bb[0], aa[0],  # .
                          aa[3], bb[3], cc[3], bb[4], aa[4],  # .
                          aa[2], bb[2], cc[2], bb[3], aa[3],  # . above bottom
                          aa[1], bb[1], cc[1], bb[2], aa[2],  # .
                          aa[0], bb[0], cc[0], bb[1], aa[1],  # .
                          aa[0], aa[1], aa[2], aa[3], aa[4]], # bottom
                         [5,5,5,5,5,5,5,5,5,5,5,5])

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(True)
