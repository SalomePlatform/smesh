# Radial Quadrangle 1D-2D example

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# Create face from the wire and add to study
Face = geom_builder.MakeSketcher("Sketcher:F 0 0:TT 20 0:R 90:C 20 90:WF", [0, 0, 0, 1, 0, 0, 0, 0, 1])
geom_builder.addToStudy(Face,"Face")
circle, radius1, radius2 = geom_builder.SubShapeAllSorted(Face, geom_builder.ShapeType["EDGE"])
geom_builder.addToStudyInFather(Face, radius1,"radius1")
geom_builder.addToStudyInFather(Face, radius2,"radius2")
geom_builder.addToStudyInFather(Face, circle,"circle")

# Define geometry for mesh, and Radial Quadrange algorithm
mesh = smesh_builder.Mesh(Face)
radial_Quad_algo = mesh.Quadrangle(algo=smeshBuilder.RADIAL_QUAD)

# The Radial Quadrange algorithm can work without any hypothesis
# In this case it uses "Default Nb of Segments" preferences parameter to discretize edges
# So by default there will be 15 segments in both radial and circular directions
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# The Radial Quadrange uses global or local 1d hypotheses if it does
# not have its own hypotheses.
# Define global hypotheses to discretize radial edges and a local one for circular edge
# So that there will be 5 radial layers and 10 circular segments
global_Nb_Segments = mesh.Segment().NumberOfSegments(5)
local_Nb_Segments  = mesh.Segment(circle).NumberOfSegments(10)
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# Define own parameters of Radial Quadrange algorithm
# The number of radial layers will be 4
radial_Quad_algo.NumberOfLayers( 4 )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
