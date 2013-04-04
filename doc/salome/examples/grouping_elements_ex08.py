# Creating groups of entities from existing groups of superior dimensions

import SMESH_mechanic
import SMESH

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : AREA > 100
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 100.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area > 100, Nb = ", len(anIds) 

# create a group by adding elements with area > 100
aSrcGroup1 = mesh.MakeGroupByIds("Area > 100", SMESH.FACE, anIds)

# Criterion : AREA < 30
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 30.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area < 30, Nb = ", len(anIds) 

# create a group by adding elements with area < 30
aSrcGroup2 = mesh.MakeGroupByIds("Area < 30", SMESH.FACE, anIds)

# Create group of edges using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], SMESH.EDGE, "Edges" )

# Create group of nodes using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], SMESH.NODE, "Nodes" )

salome.sg.updateObjBrowser(1)
