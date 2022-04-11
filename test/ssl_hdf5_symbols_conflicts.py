#!/usr/bin/env python

"""
tuleap26358 : Non regression test pointing to an incompatibiliy between hdf5 symbols in CGNS and hdf5 symbols
in hdf5 library.
"""

import inspect
import os

import salome
salome.standalone()
salome.salome_init()
import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

inputMED = os.path.abspath(os.path.join(os.path.dirname(inspect.getfile(lambda: None)), 'data', 'Mesh_tri.med'))

([Mesh_tri_1], status) = smesh.CreateMeshesFromMED(inputMED)
import SMESH
if status != SMESH.DRS_OK:
    raise RuntimeError("Test failed")
