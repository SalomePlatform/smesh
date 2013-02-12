# Create a Group on Filter

from smesh import *
SetCurrentStudy(salome.myStudy)

box = geompy.MakeBoxDXDYDZ(10,10,10)

# make a mesh with quadrangles of different area in range [1,16]
mesh = Mesh(box,"Quad mesh")
hyp1D = mesh.Segment().StartEndLength( 1, 4 )
mesh.Quadrangle()
mesh.Compute()

# create a group on filter selecting faces of medium size
critaria = [ \
    GetCriterion(FACE, FT_Area, ">", 1.1, BinaryOp=FT_LogicalAND ),
    GetCriterion(FACE, FT_Area, "<", 15.0 )
    ]
filt = GetFilterFromCriteria( critaria )
filtGroup = mesh.GroupOnFilter( FACE, "group on filter", filt )
print "Group on filter contains %s elemens" % filtGroup.Size()

# group on filter is updated if the mesh is modified
hyp1D.SetStartLength( 2.5 )
hyp1D.SetEndLength( 2.5 )
mesh.Compute()
print "After mesh change, group on filter contains %s elemens" % filtGroup.Size()

# set a new filter defining the group
filt2 = GetFilter( FACE, FT_RangeOfIds, "1-50" )
filtGroup.SetFilter( filt2 )
print "With a new filter, group on filter contains %s elemens" % filtGroup.Size()

# group is updated at modification of the filter
filt2.SetCriteria( [ GetCriterion( FACE, FT_RangeOfIds, "1-70" )])
filtIDs3 = filtGroup.GetIDs()
print "After filter modification, group on filter contains %s elemens" % filtGroup.Size()

salome.sg.updateObjBrowser(1)
