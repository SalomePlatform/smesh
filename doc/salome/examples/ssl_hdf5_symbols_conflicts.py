#!/usr/bin/env python

"""
tuleap26358 : Non regression test pointing to an incompatibiliy between hdf5 symbols in CGNS and hdf5 symbols
in hdf5 library.
"""

import salome
salome.standalone()
salome.salome_init()
import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

inputMED="Mesh_tri.med"

([Mesh_tri_1], status) = smesh.CreateMeshesFromMED(inputMED)
import SMESH
if status != SMESH.DRS_OK:
    raise RuntimeError("Test failed")
