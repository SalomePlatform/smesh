# Creating groups of entities basing on nodes of other groups

import SMESH_mechanic
import SMESH

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : AREA > 100
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 100.)

# create a group by adding elements with area > 100
aSrcGroup1 = mesh.GroupOnFilter(SMESH.FACE, "Area > 100", aFilter)
print "Criterion: Area > 100, Nb = ", aSrcGroup1.Size()

# Criterion : AREA < 30
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 30.)

# create a group by adding elements with area < 30
aSrcGroup2 = mesh.GroupOnFilter(SMESH.FACE, "Area < 30", aFilter)
print "Criterion: Area < 30, Nb = ", aSrcGroup2.Size()


# Create group of edges using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], SMESH.EDGE, "Edges" )

# Create group of nodes using source groups of faces
aGrp = mesh.CreateDimGroup( [aSrcGroup1, aSrcGroup2], SMESH.NODE, "Nodes" )

salome.sg.updateObjBrowser(True)
