# Geometry type

# create mesh with volumes
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# get all triangles, quadrangles, tetrahedrons, pyramids
filter_tri = smesh.GetFilter(smesh.FACE, smesh.FT_ElemGeomType, smesh.Geom_TRIANGLE)
filter_qua = smesh.GetFilter(smesh.FACE, smesh.FT_ElemGeomType, smesh.Geom_QUADRANGLE)
filter_tet = smesh.GetFilter(smesh.VOLUME, smesh.FT_ElemGeomType, smesh.Geom_TETRA)
filter_pyr = smesh.GetFilter(smesh.VOLUME, smesh.FT_ElemGeomType, smesh.Geom_PYRAMID)
ids_tri = mesh.GetIdsFromFilter(filter_tri)
ids_qua = mesh.GetIdsFromFilter(filter_qua)
ids_tet = mesh.GetIdsFromFilter(filter_tet)
ids_pyr = mesh.GetIdsFromFilter(filter_pyr)
print "Number of triangles:", len(ids_tri)
print "Number of quadrangles:", len(ids_qua)
print "Number of tetrahedrons:", len(ids_tet)
print "Number of pyramids:", len(ids_pyr)
