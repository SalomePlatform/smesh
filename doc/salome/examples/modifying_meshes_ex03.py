# Add 0D Element on Element Nodes


import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# create a geometry
box = geompy.MakeBoxDXDYDZ( 10, 10, 10 )
face = geompy.SubShapeAll( box, geompy.ShapeType["FACE"])[0]

# make 3D mesh
mesh = smesh.Mesh( box )
mesh.AutomaticHexahedralization(0)

# create 0D elements on all nodes of the mesh
res = mesh.Add0DElementsToAllNodes( mesh )

# find 0D elements on all nodes of the mesh, all found nodes are added to a new group
groupName = "0Dmesh"
res = mesh.Add0DElementsToAllNodes( mesh, groupName )
mesh.RemoveGroupWithContents( res ) # remove all found 0D elements

# create 0D elements on all nodes of a sub-mesh, with group creation
groupName = "0Dsubmesh"
submesh = mesh.GetSubMesh( face, "faceSM")
res = mesh.Add0DElementsToAllNodes( submesh, groupName )

# create 0D elements on all nodes of a group
group = mesh.Group( face, "faceGroup" )
res = mesh.Add0DElementsToAllNodes( group )

# remove all 0D elements
mesh.RemoveElements( mesh.GetIdsFromFilter( smesh.GetFilter( SMESH.ELEM0D,
                                                             SMESH.FT_ElemGeomType,
                                                             "=",SMESH.Geom_POINT )))

# create 0D elements on all nodes of some elements
res = mesh.Add0DElementsToAllNodes( mesh.GetElementsId() )

mesh.RemoveElements( mesh.GetElementsByType( SMESH.ELEM0D ))

# create 0D elements on some nodes
nodes = range(1,10)
res = mesh.Add0DElementsToAllNodes( mesh.GetIDSource( nodes, SMESH.NODE ))
