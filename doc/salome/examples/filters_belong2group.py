# Belong to Mesh Group criterion

# create mesh
from SMESH_mechanic import *
print

# create a group of all faces (quadrangles) generated on sub_face3
quads_on_face3 = mesh.MakeGroup("quads_on_face3", SMESH.FACE, SMESH.FT_BelongToGeom,'=',sub_face3)
print "There are %s quadrangles generated on '%s' and included in the group '%s'" % ( quads_on_face3.Size(), sub_face3.GetName(), quads_on_face3.GetName() )

# create a group of all the rest quadrangles, generated on other faces by combining 2 criteria:
# - negated FT_BelongToMeshGroup to select elements not included in quads_on_face3
# - FT_ElemGeomType to select quadrangles
not_on_face3 = smesh.GetCriterion( SMESH.FACE, SMESH.FT_BelongToMeshGroup,'=',quads_on_face3, SMESH.FT_LogicalNOT )
quadrangles  = smesh.GetCriterion( SMESH.FACE, SMESH.FT_ElemGeomType,'=',SMESH.Geom_QUADRANGLE )

rest_quads = mesh.MakeGroupByCriteria("rest_quads", [ not_on_face3, quadrangles ])
print "'%s' group includes all the rest %s quadrangles" % ( rest_quads.GetName(), rest_quads.Size() )

