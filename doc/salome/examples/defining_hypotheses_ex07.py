# Length from Edges

import geompy
import smesh

# create sketchers
sketcher1 = geompy.MakeSketcher("Sketcher:F 0 0:TT 70 0:TT 70 70:TT 0 70:WW")
sketcher2 = geompy.MakeSketcher("Sketcher:F 20 20:TT 50 20:TT 50 50:TT 20 50:WW")

# create a face from two wires
isPlanarFace = 1
face1 = geompy.MakeFaces([sketcher1, sketcher2], isPlanarFace)
geompy.addToStudy(face1, "Face1")

# create a mesh
tria = smesh.Mesh(face1, "Face : triangle 2D mesh")

# Define 1D meshing
algo1D = tria.Segment()
algo1D.NumberOfSegments(2)

# create and assign the algorithm for 2D meshing with triangles
algo2D = tria.Triangle()

# create and assign "LengthFromEdges" hypothesis to build triangles based on the length of the edges taken from the wire
algo2D.LengthFromEdges()

# compute the mesh
tria.Compute()
