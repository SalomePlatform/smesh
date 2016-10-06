# Cut of groups

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
aGroupMain = mesh.MakeGroupByIds("Area > 20", SMESH.FACE, anIds)

# Criterion : AREA < 60
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 60.)

anIds = mesh.GetIdsFromFilter(aFilter)

print "Criterion: Area < 60, Nb = ", len(anIds) 

# create a group by adding elements with area < 60
aGroupTool = mesh.MakeGroupByIds("Area < 60", SMESH.FACE, anIds)
 
# create a cut of groups : area >= 60
aGroupRes = mesh.CutGroups(aGroupMain, aGroupTool, "Area >= 60")
print "Criterion: Area >= 60, Nb = ", len(aGroupRes.GetListOfID())
# Please note that also there is CutListOfGroups() method which works with lists of groups of any lengths

salome.sg.updateObjBrowser(True)
