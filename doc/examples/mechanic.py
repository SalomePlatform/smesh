# Inspired by SMESH_mechanic.py.
# Creates geometry and a mesh to be used in other examples
# --------------------------------------------------------

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Geometry
# --------

# create planar face on a wire combined from a segment and three arcs

p1 = geom_builder.MakeVertex(100.0, 0.0, 0.0)
p2 = geom_builder.MakeVertex(50.0, 50.0, 0.0)
p3 = geom_builder.MakeVertex(100.0, 100.0, 0.0)
arc1 = geom_builder.MakeArc(p1, p2, p3)

p4 = geom_builder.MakeVertex(170.0, 100.0, 0.0)
seg1 = geom_builder.MakeVector(p3, p4)

p5 = geom_builder.MakeVertex(200.0, 70.0, 0.0)
p6 = geom_builder.MakeVertex(170.0, 40.0, 0.0)
arc2 = geom_builder.MakeArc(p4, p5, p6)

p7 = geom_builder.MakeVertex(120.0, 30.0, 0.0)
arc3 = geom_builder.MakeArc(p6, p7, p1)

wire1 = geom_builder.MakeWire([arc1, seg1, arc2, arc3])
face1 = geom_builder.MakeFace(wire1, True)

# create main shape by extruding just created planar face

pO = geom_builder.MakeVertex(0.0, 0.0, 0.0)
pz = geom_builder.MakeVertex(0.0, 0.0, 100.0)
vz = geom_builder.MakeVector(pO, pz)
prism1 = geom_builder.MakePrismVecH(face1, vz, 100.0)

# create two cylinders

pc1 = geom_builder.MakeVertex(90.0, 50.0, -40.0)
pc2 = geom_builder.MakeVertex(170.0, 70.0, -40.0)

cyl1 = geom_builder.MakeCylinder(pc1, vz, 20.0, 180.0)
cyl2 = geom_builder.MakeCylinder(pc2, vz, 20.0, 180.0)

# create final shape by cutting one cylinder and fusing with another

shape = geom_builder.MakeBoolean(prism1, cyl1, 2)
mechanic = geom_builder.MakeBoolean(shape, cyl2, 3)

# get all faces from shape and keep sub of them into variables for future processing

shape_faces = geom_builder.SubShapeAllSorted(mechanic, geom_builder.ShapeType["FACE"])

sub_face1 = shape_faces[0]
sub_face2 = shape_faces[4]
sub_face3 = shape_faces[5]
sub_face4 = shape_faces[10]

# Mesh
# ----

# create tria+tetra mesh from mechanic shape

mesh = smesh_builder.Mesh(mechanic, "mechanic")
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(25)
mesh.Tetrahedron()

# create quad sub-meshes on faces sub_face1 - sub_face4

mesh.Quadrangle(sub_face1)
mesh.Quadrangle(sub_face2)
mesh.Quadrangle(sub_face3)
mesh.Quadrangle(sub_face4)

# finally compute whole mesh

if not mesh.Compute(): raise Exception("Error when computing Mesh")
