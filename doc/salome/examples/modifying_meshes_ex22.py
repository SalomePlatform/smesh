# Extrusion

# There is a series of Extrusion Along Line methods added at different times;
# a fully functional method is ExtrusionSweepObjects()

import salome, math
salome.salome_init_without_session()
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# create an empty mesh
mesh = smesh.Mesh() 

# add a node
mesh.AddNode( 0.,0.,0. )

# extrude a node into a line of 10 segments along the X axis
ids = mesh.GetNodesId()
stepVector = [1.,0.,0.]
nbSteps = 10
mesh.ExtrusionSweep( ids, stepVector, nbSteps, IsNodes=True )

# create some groups
lastNode      = mesh.GetNodesId()[-1]
lastNodeGroup = mesh.MakeGroupByIds( "node %s"% lastNode, SMESH.NODE, [lastNode])
lineGroup     = mesh.MakeGroupByIds( "line", SMESH.EDGE, mesh.GetElementsId() )

# rotate the segments around the first node to get a mesh of a disk quarter
axisZ  = [0.,0.,0., 0.,0.,1.]
groups = mesh.RotationSweepObject( lineGroup, axisZ, math.pi/2., 10, 1e-3, MakeGroups=True, TotalAngle=True )

# extrude all faces into volumes
obj        = mesh
stepVector = [0.,0.,-1.]
nbSteps    = 5
groups = mesh.ExtrusionSweepObject2D( obj, stepVector, nbSteps, MakeGroups=True )

# remove all segments created by the last command
for g in groups:
    if g.GetType() == SMESH.EDGE:
        mesh.RemoveGroupWithContents( g )

# extrude all segments into faces along Z
obj = mesh
stepVector = [0.,0.,1.]
mesh.ExtrusionSweepObject1D( obj, stepVector, nbSteps )

# extrude a group
obj        = mesh.GetGroupByName( "line_extruded", SMESH.FACE )[0]
stepVector = [0,-5.,0.]
nbSteps    = 1
mesh.ExtrusionSweepObject( obj, stepVector, nbSteps )

# extrude all nodes and triangle faces of the disk quarter, applying a scale factor
diskGroup = mesh.GetGroupByName( "line_rotated", SMESH.FACE )[0]
crit = [ smesh.GetCriterion( SMESH.FACE, SMESH.FT_ElemGeomType,'=',SMESH.Geom_TRIANGLE ),
         smesh.GetCriterion( SMESH.FACE, SMESH.FT_BelongToMeshGroup,'=', diskGroup )]
trianglesFilter = smesh.GetFilterFromCriteria( crit )

nodes      = [ diskGroup ]
edges      = []
faces      = [ trianglesFilter ]
stepVector = [0,0,1]
nbSteps    = 10
mesh.ExtrusionSweepObjects( nodes, edges, faces, stepVector, nbSteps, scaleFactors=[0.5], linearVariation=True )

# extrude a cylindrical group of faces by normal
cylGroup = None
for g in mesh.GetGroups( SMESH.FACE ):
    if g.GetName().startswith("node "):
        cylGroup = g
        break

elements = cylGroup
stepSize = 5.
nbSteps  = 2
mesh.ExtrusionByNormal( elements, stepSize, nbSteps )

salome.sg.updateObjBrowser()
