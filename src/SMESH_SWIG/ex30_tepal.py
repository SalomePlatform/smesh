# CEA/LGLS 2008, Christian Van Wambeke (CEA/LGLS), Francis KLOSS (OCC)
# ====================================================================

import os

import geompy
import smesh

# Parameters
# ----------

results = "/tmp/ZZ"

radius =  50
height = 200

# Build a cylinder
# ----------------

base = geompy.MakeVertex(0, 0, 0)
direction = geompy.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geompy.MakeCylinder(base, direction, radius, height)

geompy.addToStudy(cylinder, "Cylinder")

# Define a mesh on a geometry
# ---------------------------

m = smesh.Mesh(cylinder)

# 2D mesh with BLSURF
# -------------------

algo2d = m.Triangle(smesh.BLSURF)

algo2d.SetPhysicalMesh(1)
algo2d.SetPhySize(5)

algo2d.SetGeometricMesh(0)

# 3D mesh with tepal
# ------------------

algo3d = m.Tetrahedron(smesh.GHS3DPRL)

algo3d.SetMEDName(results)
algo3d.SetNbPart(4)
algo3d.SetBackground(False)
algo3d.SetKeepFiles(False)
algo3d.SetToMeshHoles(True)

# Launch meshers
# --------------

status = m.Compute()

# Test if ok
# ----------

if os.access(results+".xml", os.F_OK):
    print "Ok: tepal"
else:
    print "KO: tepal"
