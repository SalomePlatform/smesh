# Add Polygon

import math

import salome
salome.salome_init()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# create an empty mesh structure
mesh = smesh.Mesh() 

# a method to build a polygonal mesh element with <nb_vert> angles:
def MakePolygon (a_mesh, x0, y0, z0, radius, nb_vert):
    al = 2.0 * math.pi / nb_vert
    node_ids = []

    # Create nodes for a polygon
    for ii in range(nb_vert):
        nid = mesh.AddNode(x0 + radius * math.cos(ii*al),
                           y0 + radius * math.sin(ii*al),
                                                     z0)
        node_ids.append(nid)
        pass

    # Create a polygon
    return mesh.AddPolygonalFace(node_ids)

# Create three polygons
f1 = MakePolygon(mesh, 0, 0,  0, 30, 13)
f2 = MakePolygon(mesh, 0, 0, 10, 21,  9)
f3 = MakePolygon(mesh, 0, 0, 20, 13,  6)

salome.sg.updateObjBrowser(True)
