# Creating groups of entities from existing groups of superior dimensions

import SMESH_mechanic

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : AREA > 100
aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Area, smesh.FT_MoreThan, 100.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area > 100, Nb = ", len(anIds) 

# create a group by adding elements with area > 100
aSrcGroup1 = mesh.MakeGroupByIds("Area > 100", smesh.FACE, anIds)

# Criterion : AREA < 30
aFilter = smesh.GetFilter(smesh.FACE, smesh.FT_Area, smesh.FT_LessThan, 30.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area < 30, Nb = ", len(anIds) 

# create a group by adding elements with area < 30
aSrcGroup2 = mesh.MakeGroupByIds("Area < 30", smesh.FACE, anIds)

# Create group of edges using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], smesh.EDGE, "Edges" )

# Create group of nodes using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], smesh.NODE, "Nodes" )

salome.sg.updateObjBrowser(1)
