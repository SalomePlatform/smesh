# Double edges, Double faces, Double volumes

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
# copy all elements with translation and Merge nodes
mesh.TranslateObject( mesh, smesh_builder.MakeDirStruct( 10,0,0), Copy=True )
mesh.MergeNodes( mesh.FindCoincidentNodes(1e-7) )
# create filters to find equal elements
equalEdgesFilter   = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_EqualEdges)
equalFacesFilter   = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_EqualFaces)
equalVolumesFilter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_EqualVolumes)
# get equal elements
print("Number of equal edges:",   len( mesh.GetIdsFromFilter( equalEdgesFilter )))
print("Number of equal faces:",   len( mesh.GetIdsFromFilter( equalFacesFilter )))
print("Number of equal volumes:", len( mesh.GetIdsFromFilter( equalVolumesFilter )))
