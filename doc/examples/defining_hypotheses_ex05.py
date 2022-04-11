# Maximum Element Area

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a face
px   = geom_builder.MakeVertex(100., 0.  , 0.  )
py   = geom_builder.MakeVertex(0.  , 100., 0.  )
pz   = geom_builder.MakeVertex(0.  , 0.  , 100.)

vxy = geom_builder.MakeVector(px, py)
arc = geom_builder.MakeArc(py, pz, px)
wire = geom_builder.MakeWire([vxy, arc])

isPlanarFace = 1
face = geom_builder.MakeFace(wire, isPlanarFace)

# add the face in the study
id_face = geom_builder.addToStudy(face, "Face to be meshed")

# create a mesh
tria_mesh = smesh_builder.Mesh(face, "Face : triangulation")

# define 1D meshing:
algo = tria_mesh.Segment()
algo.NumberOfSegments(20)

# define 2D meshing:

# assign triangulation algorithm
algo = tria_mesh.Triangle()

# assign "Max Element Area" hypothesis
algo.MaxElementArea(100)

# compute the mesh
tria_mesh.Compute()
