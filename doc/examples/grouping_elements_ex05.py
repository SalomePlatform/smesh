# Union of groups

from mechanic import *
import SALOMEDS

# Criterion : AREA > 20
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_MoreThan, 20.)

anIds = mesh.GetIdsFromFilter(aFilter)

print("Criterion: Area > 20, Nb = ", len( anIds )) 

# create a group by adding elements with area > 20
aGroup1 = mesh.CreateEmptyGroup(SMESH.FACE, "Area > 20")
aGroup1.Add(anIds)

# Criterion : AREA = 20
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_EqualTo, 20.)

anIds = mesh.GetIdsFromFilter(aFilter)

print("Criterion: Area = 20, Nb = ", len( anIds )) 

# create a group by adding elements with area = 20
aGroup2 = mesh.CreateEmptyGroup( SMESH.FACE, "Area = 20" )

aGroup2.Add(anIds)

# create union group : area >= 20
aGroup3 = mesh.UnionListOfGroups([aGroup1, aGroup2], "Area >= 20")
aGroup3.SetColor( SALOMEDS.Color(1.,1.,0.));
print("Criterion: Area >= 20, Nb = ", len(aGroup3.GetListOfID()))
# Please note that also there is UnionGroups() method which works with two groups only

# Criterion : AREA < 20
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Area, SMESH.FT_LessThan, 20.)

anIds = mesh.GetIdsFromFilter(aFilter)

print("Criterion: Area < 20, Nb = ", len(anIds))

# create a group by adding elements with area < 20
aGroup4 = mesh.CreateEmptyGroup(SMESH.FACE, "Area < 20")
aGroup4.Add(anIds)
aGroup4.SetColor( SALOMEDS.Color(1.,0.,0.));

# create union group : area >= 20 and area < 20
aGroup5 = mesh.UnionListOfGroups([aGroup3, aGroup4], "Any Area")
print("Criterion: Any Area, Nb = ", len(aGroup5.GetListOfID()))
