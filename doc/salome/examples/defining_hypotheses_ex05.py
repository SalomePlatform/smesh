# Maximum Element Area

import geompy
import smesh
import salome 

# create a face
px   = geompy.MakeVertex(100., 0.  , 0.  )
py   = geompy.MakeVertex(0.  , 100., 0.  )
pz   = geompy.MakeVertex(0.  , 0.  , 100.)

vxy = geompy.MakeVector(px, py)
arc = geompy.MakeArc(py, pz, px)
wire = geompy.MakeWire([vxy, arc])

isPlanarFace = 1
face = geompy.MakeFace(wire, isPlanarFace)

# add the face in the study
id_face = geompy.addToStudy(face, "Face to be meshed")

# create a mesh
tria_mesh = smesh.Mesh(face, "Face : triangulation")

# define 1D meshing:
algo = tria_mesh.Segment()
algo.NumberOfSegments(20)

# define 2D meshing:

# assign triangulation algorithm
algo = tria_mesh.Triangle()

# apply "Max Element Area" hypothesis to each triangle
algo.MaxElementArea(100)

# compute the mesh
tria_mesh.Compute()
