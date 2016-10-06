# Intersection of groups

import SMESH_mechanic
import SMESH

smesh  = SMESH_mechanic.smesh
mesh   = SMESH_mechanic.mesh
salome = SMESH_mechanic.salome

# Criterion : AREA > 20
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 20.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area > 20, Nb = ", len(anIds) 

# create a group by adding elements with area > 20
aGroup1 = mesh.CreateEmptyGroup(SMESH.FACE, "Area > 20")
aGroup1.Add(anIds)

# Criterion : AREA < 60
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 60.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area < 60, Nb = ", len(anIds) 

# create a group by adding elements with area < 60
aGroup2 = mesh.CreateEmptyGroup(SMESH.FACE, "Area < 60")
aGroup2.Add(anIds)

# create an intersection of groups : 20 < area < 60
aGroup3 = mesh.IntersectListOfGroups([aGroup1, aGroup2], "20 < Area < 60")
print "Criterion: 20 < Area < 60, Nb = ", len(aGroup3.GetListOfID())
# Please note that also there is IntersectGroups() method which works with two groups only

salome.sg.updateObjBrowser(True)
