# Number of connectivities of a node

# create a mesh
from SMESH_mechanic import *

# get nodes connected to more than 6 tetrahedra
conn_nb_filter = smesh.GetFilter(SMESH.NODE, SMESH.FT_NodeConnectivityNumber,'>', 6 )
ids = mesh.GetIdsFromFilter( conn_nb_filter )
print "Number of nodes connected to more than 6 tetrahedra:", len(ids)
