# Group color

# create mesh
from SMESH_mechanic import *
# create group of edges
all_edges = mesh.GetElementsByType(SMESH.EDGE)
grp = mesh.MakeGroupByIds("edges group", SMESH.EDGE, all_edges[:len(all_edges)/4])
import SALOMEDS
c = SALOMEDS.Color(0.1, 0.5, 1.0)
grp.SetColor(c)
# get number of the edges not belonging to the group with the given color
filter = smesh.GetFilter(SMESH.EDGE, SMESH.FT_GroupColor, c, SMESH.FT_LogicalNOT)
ids = mesh.GetIdsFromFilter(filter)
print "Number of edges not beloging to the group with color (0.1, 0.5, 1.0):", len(ids)
