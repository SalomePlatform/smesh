# Create a Standalone Group

import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Get ids of all faces with area > 100 
aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Area, smesh.FT_MoreThan, 100.)

anIds = mesh.GetIdsFromFilter(aFilter) 

# create a group consisting of faces with area > 100
aGroup1 = mesh.MakeGroupByIds("Area > 100", smesh.FACE, anIds)

# create a group that contains all nodes from the mesh
aGroup2 = mesh.CreateEmptyGroup(smesh.NODE, "all nodes")
aGroup2.AddFrom(mesh.mesh)

salome.sg.updateObjBrowser(1)
