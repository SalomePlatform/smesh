# CEA/LGLS 2004-2005, Francis KLOSS (OCC)
# =======================================

from geompy import *

import smesh

# Geometrie
# =========

# Creer un cylindre surplombe d'une demi-sphere le tout troue par un petit cylindre.
# Decouper en hexahedre et mailler.

# Donnees
# -------

cylindre_rayon   = 100
cylindre_hauteur = 400

trou_rayon = 20
trou_z     = cylindre_rayon/2

plan_trim = 2000

# Cylindre
# --------

cylindre_base = MakeVertex(0, 0, 0)
cylindre_dir  = MakeVectorDXDYDZ(1, 0, 0)
cylindre      = MakeCylinder(cylindre_base, cylindre_dir, cylindre_rayon, cylindre_hauteur)

# Dome
# ----

dome_sphere = MakeSpherePntR(cylindre_base, cylindre_rayon)
dome        = MakeFuse(dome_sphere, cylindre)

# Cheminee
# --------

cheminee_base = MakeVertex(-cylindre_hauteur/2, 0, trou_z)
cheminee_trou = MakeCylinder(cheminee_base, cylindre_dir, trou_rayon, 2*cylindre_hauteur)
cheminee      = MakeCut(dome, cheminee_trou)

# Decoupage et reparation
# -----------------------

blocs_plan1 = MakePlane(cheminee_base, MakeVectorDXDYDZ(0, 1, 0), plan_trim)
blocs_plan2 = MakePlane(cheminee_base, MakeVectorDXDYDZ(0, 0, 1), plan_trim)

blocs_part = MakePartition([cheminee], [blocs_plan1, blocs_plan2], [], [], ShapeType["SOLID"])

piece = RemoveExtraEdges(blocs_part)

# Etude
# -----

piece_id = addToStudy(piece, "ex17_dome1")

# Maillage
# ========

# Maillage hexahedrique
# ---------------------

hexa = smesh.Mesh(piece, "ex17_dome1:hexa")

algo = hexa.Segment()
algo.NumberOfSegments(20)

hexa.Quadrangle()

hexa.Hexahedron()

# Calcul du maillage
# ------------------

hexa.Compute()
