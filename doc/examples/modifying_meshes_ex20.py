# Cutting Quadrangles

from mechanic import *

# cut two quadrangles: 405 and 406
mesh.QuadToTri([405, 406], SMESH.FT_MinimumAngle)
