# Geometry type

# create mesh with volumes
from mechanic import *

# get all triangles, quadrangles, tetrahedrons, pyramids
filter_tri = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_ElemGeomType, SMESH.Geom_TRIANGLE)
filter_qua = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_ElemGeomType, SMESH.Geom_QUADRANGLE)
filter_tet = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_ElemGeomType, SMESH.Geom_TETRA)
filter_pyr = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_ElemGeomType, SMESH.Geom_PYRAMID)
ids_tri = mesh.GetIdsFromFilter(filter_tri)
ids_qua = mesh.GetIdsFromFilter(filter_qua)
ids_tet = mesh.GetIdsFromFilter(filter_tet)
ids_pyr = mesh.GetIdsFromFilter(filter_pyr)
print("Number of triangles:", len(ids_tri))
print("Number of quadrangles:", len(ids_qua))
print("Number of tetrahedrons:", len(ids_tet))
print("Number of pyramids:", len(ids_pyr))
