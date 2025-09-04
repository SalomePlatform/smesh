# Create a Group on Filter

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(10,10,10)

# make a mesh with quadrangles of different area in range [1,16]
mesh = smesh_builder.Mesh(box,"Quad mesh")
hyp1D = mesh.Segment().StartEndLength( 1, 4 )
mesh.Quadrangle()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# create a group on filter selecting faces of medium size
critaria = [ \
    smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_Area, ">", 1.1, BinaryOp=SMESH.FT_LogicalAND ),
    smesh_builder.GetCriterion(SMESH.FACE, SMESH.FT_Area, "<", 15.0 )
    ]
filt = smesh_builder.GetFilterFromCriteria( critaria )
filtGroup = mesh.GroupOnFilter( SMESH.FACE, "group on filter", filt )
print("Group on filter contains %s elements" % filtGroup.Size())

# group on filter is updated if the mesh is modified
hyp1D.SetStartLength( 2.5 )
hyp1D.SetEndLength( 2.5 )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("After mesh change, group on filter contains %s elements" % filtGroup.Size())

# set a new filter defining the group
filt2 = smesh_builder.GetFilter( SMESH.FACE, SMESH.FT_RangeOfIds, "1-50" )
filtGroup.SetFilter( filt2 )
print("With a new filter, group on filter contains %s elements" % filtGroup.Size())

# group is updated at modification of the filter
filt2.SetCriteria( [ smesh_builder.GetCriterion( SMESH.FACE, SMESH.FT_RangeOfIds, "1-70" )])
filtIDs3 = filtGroup.GetIDs()
print("After filter modification, group on filter contains %s elements" % filtGroup.Size())
