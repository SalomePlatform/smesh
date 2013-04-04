# Double edges, Double faces, Double volumes


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook

# make a mesh on a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
mesh = smesh.Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()
# copy all elements with translation and Merge nodes
mesh.TranslateObject( mesh, smesh.MakeDirStruct( 10,0,0), Copy=True )
mesh.MergeNodes( mesh.FindCoincidentNodes(1e-7) )
# create filters to find equal elements
equalEdgesFilter   = smesh.GetFilter(SMESH.EDGE, SMESH.FT_EqualEdges)
equalFacesFilter   = smesh.GetFilter(SMESH.FACE, SMESH.FT_EqualFaces)
equalVolumesFilter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_EqualVolumes)
# get equal elements
print "Number of equal edges:",   len( mesh.GetIdsFromFilter( equalEdgesFilter ))
print "Number of equal faces:",   len( mesh.GetIdsFromFilter( equalFacesFilter ))
print "Number of equal volumes:", len( mesh.GetIdsFromFilter( equalVolumesFilter ))
