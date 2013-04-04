# Cutting Quadrangles

import SMESH_mechanic
import SMESH

smesh = SMESH_mechanic.smesh
mesh  = SMESH_mechanic.mesh

# cut two quadrangles: 405 and 406
mesh.QuadToTri([405, 406], SMESH.FT_MinimumAngle)
