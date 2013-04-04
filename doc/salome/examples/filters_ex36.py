# Combine filters with Criterion structures using of "criteria".

# create mesh
from SMESH_mechanic import *
# get all the quadrangle faces ...
criterion1 = smesh.GetCriterion(SMESH.FACE, SMESH.FT_ElemGeomType, SMESH.Geom_QUADRANGLE, SMESH.FT_LogicalAND)
# ... AND do NOT get those from sub_face3
criterion2 = smesh.GetCriterion(SMESH.FACE, SMESH.FT_BelongToGeom, sub_face3, SMESH.FT_LogicalNOT)
filter = smesh.CreateFilterManager().CreateFilter()
filter.SetCriteria([criterion1,criterion2])
ids = mesh.GetIdsFromFilter(filter)

myGroup = mesh.MakeGroupByIds("Quads_on_cylindrical_faces",SMESH.FACE,ids)
