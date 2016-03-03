# Borders at multi-connection

import salome
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)
import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# make a mesh on a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
mesh = smesh.Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()
# copy all elements with translation and merge nodes
mesh.TranslateObject( mesh, [10,0,0], Copy=True )
mesh.MergeNodes( mesh.FindCoincidentNodes( 1e-5 ))

# get mesh edges with number of connected elements (faces and volumes) == 3
filter = smesh.GetFilter(SMESH.EDGE, SMESH.FT_MultiConnection, 3)
ids = mesh.GetIdsFromFilter(filter)
print "Number of border edges with 3 faces connected:", len(ids)
