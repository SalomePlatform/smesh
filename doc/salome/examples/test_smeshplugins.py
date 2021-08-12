#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test000
Checks the availability of the external plugins :
 1 NETGENPLUGIN
 2 GMSHPLUGIN
 3 MG-CADSURF           (BLSURFPLUGIN)
 4 MG-TETRA             (GHS3DPLUGIN)
 5 MG-HEXA              (HEXOTICPLUGIN)
 6 MG-HYBRID            (HYBRIDPLUGIN)
Copyright EDF R&D 2017
"""
__revision__ = "V1.0"
#
# Computation of the meshes: T/F
ComputeMeshes = True

import salome

salome.salome_init_without_session()
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

# B.1. NETGEN
  TEXTE = "NETGEN_1D2D3D"
  MESH_1 = smesh.Mesh(BOX)
  smesh.SetName(MESH_1.GetMesh(), "M_"+TEXTE)
  try :
    NETGEN_2D3D = MESH_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(NETGEN_2D3D.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_1.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

# B.2. Gmsh
  TEXTE = "Gmsh"
  MESH_2 = smesh.Mesh(BOX)
  smesh.SetName(MESH_2.GetMesh(), "M_"+TEXTE)
  try :
    GMSH = MESH_2.Tetrahedron(algo=smeshBuilder.GMSH)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(GMSH.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_2.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

# B.3. MG_CADSurf
  TEXTE = "MG_CADSurf"
  MESH_3 = smesh.Mesh(BOX)
  smesh.SetName(MESH_3.GetMesh(), "M_"+TEXTE)
  try :
    MG_CADSurf = MESH_3.Triangle(algo=smeshBuilder.MG_CADSurf)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
#   On arrete tout en cas de problème car les suivants en dépendent
    break
  else :
    if ComputeMeshes :
      smesh.SetName(MG_CADSurf.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_3.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

# B.4. MG_Tetra
  TEXTE = "MG_Tetra"
  MESH_4 = smesh.Mesh(BOX)
  smesh.SetName(MESH_4.GetMesh(), "M_"+TEXTE)
  MG_CADSurf_Te = MESH_4.Triangle(algo=smeshBuilder.MG_CADSurf)
  try :
    MG_Tetra = MESH_4.Tetrahedron(algo=smeshBuilder.MG_Tetra)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(MG_Tetra.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_4.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

# B.5. MG_Hexa
  TEXTE = "MG_Hexa"
  MESH_5 = smesh.Mesh(BOX)
  smesh.SetName(MESH_5.GetMesh(), "M_"+TEXTE)
  MG_CADSurf_He = MESH_5.Triangle(algo=smeshBuilder.MG_CADSurf)
  try :
    MG_Hexa = MESH_5.Hexahedron(algo=smeshBuilder.MG_Hexa)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(MG_Hexa.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_5.Compute()
      if not OK_COMPUTE :
        MESSAGE += "\nErreur avec "+TEXTE
        ERROR += 1
      else :
        print(TEXTE+": OK")

# B.6. MG_Hybrid
  TEXTE = "MG_Hybrid"
  MESH_6 = smesh.Mesh(BOX)
  smesh.SetName(MESH_6.GetMesh(), "M_"+TEXTE)
  MG_CADSurf_Hy = MESH_6.Triangle(algo=smeshBuilder.MG_CADSurf)
  try :
    MG_Hybrid = MESH_6.Tetrahedron(algo=smeshBuilder.HYBRID)
  except :
    MESSAGE += "\nImpossible d'utiliser "+TEXTE
    ERROR += 1
  else :
    if ComputeMeshes :
      smesh.SetName(MG_Hybrid.GetAlgorithm(), TEXTE)
      OK_COMPUTE = MESH_6.Compute()
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

