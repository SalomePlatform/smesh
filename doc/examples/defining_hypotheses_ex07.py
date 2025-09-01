# Length from Edges

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create sketchers
sketcher1 = geom_builder.MakeSketcher("Sketcher:F 0 0:TT 70 0:TT 70 70:TT 0 70:WW")
sketcher2 = geom_builder.MakeSketcher("Sketcher:F 20 20:TT 50 20:TT 50 50:TT 20 50:WW")

# create a face from two wires
isPlanarFace = 1
face1 = geom_builder.MakeFaces([sketcher1, sketcher2], isPlanarFace)
geom_builder.addToStudy(face1, "Face1")

# create a mesh
tria = smesh_builder.Mesh(face1, "Face : triangle 2D mesh")

# Define 1D meshing
algo1D = tria.Segment()
algo1D.LocalLength(3.)

# create and assign the algorithm for 2D meshing with triangles
algo2D = tria.Triangle()

# create and assign "LengthFromEdges" hypothesis to build triangles with
# linear size close to the length of the segments generated on the face wires (3.)
algo2D.LengthFromEdges()

# compute the mesh
if not tria.Compute(): raise Exception("Error when computing Mesh")
