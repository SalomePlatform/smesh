# Ball diameter

# create a mesh
from mechanic import *

# create several balls with increasing diameter
for i in range(1,10):
    diameter = float( i )
    mesh.AddBall( i, diameter )
    pass

# get balls with diameter > 5.
diam_filter = smesh_builder.GetFilter(SMESH.BALL, SMESH.FT_BallDiameter,'>', 5. )
ids = mesh.GetIdsFromFilter( diam_filter )
print("Number of balls with diameter > 5:", len(ids))
