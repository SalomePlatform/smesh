# Entity type

# create a mesh
from SMESH_mechanic import *

# make the mesh quadratic
mesh.ConvertToQuadratic()
# make some elements bi-quadratic
for face in SubFaceL[: len(SubFaceL)/2]:
    mesh.ConvertToQuadratic( theSubMesh=mesh.Group( face ), theToBiQuad=True )

# get triangles with 7 nodes
filter_tri = smesh.GetFilter(SMESH.FACE, SMESH.FT_EntityType,'=', SMESH.Entity_BiQuad_Triangle )
ids_tri = mesh.GetIdsFromFilter(filter_tri)
print "Number of bi-quadratic triangles:", len(ids_tri)
