# Add Polygon

from salome.kernel import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

# create an empty mesh structure
mesh = smesh_builder.Mesh() 

# a method to build a polygonal mesh element with <nb_vert> angles:
def MakePolygon (a_mesh, x0, y0, z0, radius, nb_vert, smesh_builder):
    import math

    al = 2.0 * math.pi / nb_vert
    node_ids = []

    # Create nodes for a polygon
    for ii in range(nb_vert):
        nid = smesh_builder.AddNode(x0 + radius * math.cos(ii*al),
                                    y0 + radius * math.sin(ii*al),
                                    z0)
        node_ids.append(nid)
        pass

    # Create a polygon
    return smesh_builder.AddPolygonalFace(node_ids)

# Create three polygons
f1 = MakePolygon(mesh, 0, 0,  0, 30, 13, smesh_builder=mesh)
f2 = MakePolygon(mesh, 0, 0, 10, 21,  9, smesh_builder=mesh)
f3 = MakePolygon(mesh, 0, 0, 20, 13,  6, smesh_builder=mesh)
