# Deflection and Number of Segments

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a face from arc and straight segment
px = geom_builder.MakeVertex(100., 0.  , 0.  )
py = geom_builder.MakeVertex(0.  , 100., 0.  )
pz = geom_builder.MakeVertex(0.  , 0.  , 100.)

exy = geom_builder.MakeEdge(px, py)
arc = geom_builder.MakeArc(py, pz, px)

wire = geom_builder.MakeWire([exy, arc])

isPlanarFace = 1
face1 = geom_builder.MakeFace(wire, isPlanarFace)
geom_builder.addToStudy(face1,"Face1")

# get edges from the face
e_straight,e_arc = geom_builder.SubShapeAll(face1, geom_builder.ShapeType["EDGE"])
geom_builder.addToStudyInFather(face1, e_arc, "Arc Edge")

# create hexahedral mesh
hexa = smesh_builder.Mesh(face1, "Face : triangle mesh")

# define "NumberOfSegments" hypothesis to cut a straight edge in a fixed number of segments
algo1D = hexa.Segment()
algo1D.NumberOfSegments(6)

# define "MaxElementArea" hypothesis
algo2D = hexa.Triangle()
algo2D.MaxElementArea(70.0)

# define a local "Deflection1D" hypothesis on the arc
algo_local = hexa.Segment(e_arc)
algo_local.Deflection1D(1.0)

# compute the mesh
hexa.Compute()
