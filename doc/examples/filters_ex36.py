# Combine several criteria into a filter

# create mesh
from mechanic import *

# get all the quadrangle faces ...
criterion1 = smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_ElemGeomType, SMESH.Geom_QUADRANGLE, SMESH.FT_LogicalAND)
# ... but those from sub_face3
criterion2 = smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_BelongToGeom, sub_face3, SMESH.FT_LogicalNOT)

quadFilter = smesh_builder.GetFilterFromCriteria([criterion1,criterion2])

# get faces satisfying the criteria
ids = mesh.GetIdsFromFilter(quadFilter)

# create a group of faces satisfying the criteria
myGroup = mesh.GroupOnFilter(SMESH.FACE,"Quads_on_cylindrical_faces",quadFilter)
