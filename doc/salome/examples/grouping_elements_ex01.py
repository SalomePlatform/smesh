# Create a Standalone Group

import SMESH_mechanic
import SMESH

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Get ids of all faces with area > 100 
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 100.)

anIds = mesh.GetIdsFromFilter(aFilter) 

# create a group consisting of faces with area > 100
aGroup1 = mesh.MakeGroupByIds("Area > 100", SMESH.FACE, anIds)

# create a group that contains all nodes from the mesh
aGroup2 = mesh.CreateEmptyGroup(SMESH.NODE, "all nodes")
aGroup2.AddFrom(mesh.mesh)


# ====================================
# Various methods of the Group object
# ====================================

aGroup = mesh.CreateEmptyGroup(SMESH.NODE, "aGroup")

# set/get group name
aGroup.SetName( "new name" )
print "name", aGroup.GetName()

# get group type (type of entities in the group, SMESH.NODE in our case)
print "type", aGroup.GetType()

# get number of entities (nodes in our case) in the group
print "size", aGroup.Size()

# check of emptiness
print "is empty", aGroup.IsEmpty()

# check of presence of an entity in the group
aGroup.Add([1,2]) # Add() method is specific to the standalone group
print "contains node 2", aGroup.Contains(2)

# get an entity by index
print "1st node", aGroup.GetID(1)

# get all entities
print "all", aGroup.GetIDs()

# get number of nodes (actual for groups of elements)
print "nb nodes", aGroup.GetNumberOfNodes()

# get underlying nodes (actual for groups of elements)
print "nodes", aGroup.GetNodeIDs()

# set/get color
import SALOMEDS
aGroup.SetColor( SALOMEDS.Color(1.,1.,0.));
print "color", aGroup.GetColor()

# ----------------------------------------------------------------------------
# methods specific to the standalone group and not present in GroupOnGeometry
# and GroupOnFilter
# ----------------------------------------------------------------------------

# clear the group's contents
aGroup.Clear()

# add contents of other object (group, sub-mesh, filter)
aGroup.AddFrom( aGroup2 )

# removes entities
aGroup.Remove( [2,3,4] )




salome.sg.updateObjBrowser(True)
