# Aspect ratio
# This script demonstrates various usages of filters

# create mesh
from SMESH_mechanic import *

# get faces with aspect ratio > 2.5
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_AspectRatio, SMESH.FT_MoreThan, 2.5)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with aspect ratio > 2.5:", len(ids)

# get faces with aspect ratio > 1.5
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_AspectRatio, '>', 1.5, mesh=mesh)
ids = filter.GetIDs()
print "Number of faces with aspect ratio > 1.5:", len(ids)

# copy the faces with aspect ratio > 1.5 to another mesh;
# this demostrates that a filter can be used where usually a group or sub-mesh is acceptable
filter.SetMesh( mesh.GetMesh() ) # - actually non necessary as mesh is set at filter creation
mesh2 = smesh.CopyMesh( filter, "AR > 1.5" )
print "Number of copied faces with aspect ratio > 1.5:", mesh2.NbFaces()

# create a group (Group on Filter) of faces with Aspect Ratio < 1.5
group = mesh.MakeGroup("AR < 1.5", SMESH.FACE, SMESH.FT_AspectRatio, '<', 1.5)
print "Number of faces with aspect ratio < 1.5:", group.Size()

# combine several criteria to Create a Group of only Triangular faces with Aspect Ratio < 1.5;
# note that contents of a GroupOnFilter is dynamically updated as the mesh changes
crit = [ smesh.GetCriterion( SMESH.FACE, SMESH.FT_AspectRatio, '<', 1.5, BinaryOp=SMESH.FT_LogicalAND ),
         smesh.GetCriterion( SMESH.FACE, SMESH.FT_ElemGeomType,'=', SMESH.Geom_TRIANGLE ) ]
triaGroup = mesh.MakeGroupByCriteria( "Tria AR < 1.5", crit )
print "Number of triangles with aspect ratio < 1.5:", triaGroup.Size()

# get range of values of Aspect Ratio of all faces in the mesh
aspects = mesh.GetMinMax( SMESH.FT_AspectRatio )
print "MESH: Min aspect = %s, Max aspect = %s" % ( aspects[0], aspects[1] )

# get max value of Aspect Ratio of faces in triaGroup
grAspects = mesh.GetMinMax( SMESH.FT_AspectRatio, triaGroup )
print "GROUP: Max aspect = %s" % grAspects[1]

# get Aspect Ratio of an element
aspect = mesh.FunctorValue( SMESH.FT_AspectRatio, ids[0] )
print "Aspect ratio of the face %s = %s" % ( ids[0], aspect )
