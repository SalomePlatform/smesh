# Ball diameter

# create a mesh
from SMESH_mechanic import *

# create several balls with increasing diameter
for i in range(1,10):
    diameter = float( i )
    mesh.AddBall( i, diameter )
    pass

# get balls with diameter > 5.
diam_filter = smesh.GetFilter(SMESH.BALL, SMESH.FT_BallDiameter,'>', 5. )
ids = mesh.GetIdsFromFilter( diam_filter )
print "Number of balls with diameter > 5:", len(ids)
