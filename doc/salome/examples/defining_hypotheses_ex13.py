# Radial Quadrangle 1D-2D example

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# Create face from the wire and add to study
Face = geompy.MakeSketcher("Sketcher:F 0 0:TT 20 0:R 90:C 20 90:WF", [0, 0, 0, 1, 0, 0, 0, 0, 1])
geompy.addToStudy(Face,"Face")
circle, radius1, radius2 = geompy.SubShapeAllSorted(Face, geompy.ShapeType["EDGE"])
geompy.addToStudyInFather(Face, radius1,"radius1")
geompy.addToStudyInFather(Face, radius2,"radius2")
geompy.addToStudyInFather(Face, circle,"circle")


# Define geometry for mesh, and Radial Quadrange algorithm
mesh = smesh.Mesh(Face)
radial_Quad_algo = mesh.Quadrangle(algo=smeshBuilder.RADIAL_QUAD)

# The Radial Quadrange algorithm can work without any hypothesis
# In this case it uses "Default Nb of Segments" preferences parameter to discretize edges
# So by default there will be 15 segments in both radial and circular directions
mesh.Compute()

# The Radial Quadrange uses global or local 1d hypotheses if it does
# not have its own hypotheses.
# Define global hypotheses to discretize radial edges and a local one for circular edge
# So that there will be 5 radial layers and 10 circular segments
global_Nb_Segments = mesh.Segment().NumberOfSegments(5)
local_Nb_Segments  = mesh.Segment(circle).NumberOfSegments(10)
mesh.Compute()

# Define own parameters of Radial Quadrange algorithm
# The number of radial layers will be 4
radial_Quad_algo.NumberOfLayers( 4 )
mesh.Compute()
