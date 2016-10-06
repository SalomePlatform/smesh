# Create a Group on Filter


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


box = geompy.MakeBoxDXDYDZ(10,10,10)

# make a mesh with quadrangles of different area in range [1,16]
mesh = smesh.Mesh(box,"Quad mesh")
hyp1D = mesh.Segment().StartEndLength( 1, 4 )
mesh.Quadrangle()
mesh.Compute()

# create a group on filter selecting faces of medium size
critaria = [ \
    smesh.GetCriterion(SMESH.FACE, SMESH.FT_Area, ">", 1.1, BinaryOp=SMESH.FT_LogicalAND ),
    smesh.GetCriterion(SMESH.FACE, SMESH.FT_Area, "<", 15.0 )
    ]
filt = smesh.GetFilterFromCriteria( critaria )
filtGroup = mesh.GroupOnFilter( SMESH.FACE, "group on filter", filt )
print "Group on filter contains %s elemens" % filtGroup.Size()

# group on filter is updated if the mesh is modified
hyp1D.SetStartLength( 2.5 )
hyp1D.SetEndLength( 2.5 )
mesh.Compute()
print "After mesh change, group on filter contains %s elemens" % filtGroup.Size()

# set a new filter defining the group
filt2 = smesh.GetFilter( SMESH.FACE, SMESH.FT_RangeOfIds, "1-50" )
filtGroup.SetFilter( filt2 )
print "With a new filter, group on filter contains %s elemens" % filtGroup.Size()

# group is updated at modification of the filter
filt2.SetCriteria( [ smesh.GetCriterion( SMESH.FACE, SMESH.FT_RangeOfIds, "1-70" )])
filtIDs3 = filtGroup.GetIDs()
print "After filter modification, group on filter contains %s elemens" % filtGroup.Size()

salome.sg.updateObjBrowser(True)
