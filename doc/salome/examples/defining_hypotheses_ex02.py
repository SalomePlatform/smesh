# Deflection and Number of Segments

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a face from arc and straight segment
px = geompy.MakeVertex(100., 0.  , 0.  )
py = geompy.MakeVertex(0.  , 100., 0.  )
pz = geompy.MakeVertex(0.  , 0.  , 100.)

exy = geompy.MakeEdge(px, py)
arc = geompy.MakeArc(py, pz, px)

wire = geompy.MakeWire([exy, arc])

isPlanarFace = 1
face1 = geompy.MakeFace(wire, isPlanarFace)
geompy.addToStudy(face1,"Face1")

# get edges from the face
e_straight,e_arc = geompy.SubShapeAll(face1, geompy.ShapeType["EDGE"])
geompy.addToStudyInFather(face1, e_arc, "Arc Edge")

# create hexahedral mesh
hexa = smesh.Mesh(face1, "Face : triangle mesh")

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
