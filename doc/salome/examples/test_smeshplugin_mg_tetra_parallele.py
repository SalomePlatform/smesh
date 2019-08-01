#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test000
Checks the availability of the external plugins :
 1 MG-TETRA-PARALLELE   (GHS3DPRLPLUGIN)
Copyright EDF R&D 2017
"""
__revision__ = "V1.0"
#
# Computation of the meshes: T/F
ComputeMeshes = True

import salome

salome.salome_init()
theStudy = salome.myStudy
#
import iparameters
IPAR = iparameters.IParameters(salome.myStudy.GetCommonParameters("Interface Applicative", 1))

# fill list AP_MODULES_LIST
IPAR.append("AP_MODULES_LIST", "Geometry")
IPAR.append("AP_MODULES_LIST", "Mesh")

ERROR = 0
MESSAGE = ""
#
while not ERROR :

###
### A. GEOM component
###
  import GEOM
  from salome.geom import geomBuilder
  geompy = geomBuilder.New()
  O = geompy.MakeVertex(0, 0, 0, "0")
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0, "OX")
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0, "OY")
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1, "OZ")
  BOX = geompy.MakeBoxDXDYDZ(200, 200, 200, "BOX")

###
### B. SMESH component
###

  import  SMESH
  from salome.smesh import smeshBuilder

  smesh = smeshBuilder.New()


# B.7. MG_Tetra_Parallel
  TEXTE = "MG_Tetra_Parallel"
  MESH_7 = smesh.Mesh(BOX)
  smesh.SetName(MESH_7.GetMesh(), "M_"+TEXTE)
  MG_CADSurf_Tp = MESH_7.Triangle(algo=smeshBuilder.MG_CADSurf)
  try :
    MG_Tetra_Parallel = MESH_7.Tetrahedron(algo=smeshBuilder.MG_Tetra_Parallel)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(MG_Tetra_Parallel.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_7.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

  break

###
### C. End
###
if ERROR :
  raise Exception("\n\nNombre d'erreurs : %d" % ERROR + MESSAGE + "\n")
else :
  print("\nAucun problème\n")

