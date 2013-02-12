# Cutting Quadrangles

import SMESH_mechanic

smesh = SMESH_mechanic.smesh
mesh  = SMESH_mechanic.mesh

# cut two quadrangles: 405 and 406
mesh.QuadToTri([405, 406], smesh.FT_MinimumAngle)
