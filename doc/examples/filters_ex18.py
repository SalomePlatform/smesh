# Borders at multi-connection

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# make a mesh on a box
box = geom_builder.MakeBoxDXDYDZ(100,100,100)
mesh = smesh_builder.Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
if not mesh.Compute(): raise Exception("Error when computing Mesh")
# copy all elements with translation and merge nodes
mesh.TranslateObject( mesh, [10,0,0], Copy=True )
mesh.MergeNodes( mesh.FindCoincidentNodes( 1e-5 ))

# get mesh edges with number of connected elements (faces and volumes) == 3
filter = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_MultiConnection, 3)
ids = mesh.GetIdsFromFilter(filter)
print("Number of border edges with 3 faces connected:", len(ids))
