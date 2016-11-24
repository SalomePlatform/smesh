# -*- coding: utf-8 -*-

###
### This file is generated automatically by SALOME v7.7.0 with dump python functionality
###

import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.NoteBook(theStudy)

###
### GEOM component
###


#L = 1.
#N = 20 #Nombre d elements sur un cote

def cube3D(L, N, outFile):

  N=int(N)
  from salome.geom import geomBuilder

  geompy = geomBuilder.New(theStudy)

  eps=L*1.e-6

  O = geompy.MakeVertex(0, 0, 0)
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
  cube = geompy.MakeBoxDXDYDZ(L, L, L)
  geompy.TranslateDXDYDZ(cube, -L/2., -L/2., -L/2.)

  epais=1./(2.*N)
  larg=L*1.1

  boxX = geompy.MakeBoxDXDYDZ(epais, larg, larg)
  geompy.TranslateDXDYDZ(boxX, -epais/2., -larg/2., -larg/2.)
  boxXM = geompy.TranslateDXDYDZ(boxX, -L/2., 0., 0., theCopy=True)
  boxXP = geompy.TranslateDXDYDZ(boxX,  L/2., 0., 0., theCopy=True)

  boxY = geompy.MakeBoxDXDYDZ(larg, epais, larg)
  geompy.TranslateDXDYDZ(boxY, -larg/2., -epais/2., -larg/2.)
  boxYM = geompy.TranslateDXDYDZ(boxY, 0., -L/2., 0., theCopy=True)
  boxYP = geompy.TranslateDXDYDZ(boxY, 0.,  L/2., 0., theCopy=True)

  boxZ = geompy.MakeBoxDXDYDZ(larg, larg, epais)
  geompy.TranslateDXDYDZ(boxZ, -larg/2., -larg/2., -epais/2.)
  boxZM = geompy.TranslateDXDYDZ(boxZ, 0., 0., -L/2., theCopy=True)
  boxZP = geompy.TranslateDXDYDZ(boxZ, 0., 0.,  L/2., theCopy=True)

  box = geompy.MakeBoxDXDYDZ(larg, larg, larg)

  boxXPLUS = geompy.TranslateDXDYDZ(box, 0.,       -larg/2., -larg/2., theCopy=True)
  boxXMOIN = geompy.TranslateDXDYDZ(box, -larg,    -larg/2., -larg/2., theCopy=True)

  boxYPLUS = geompy.TranslateDXDYDZ(box, -larg/2., 0.,       -larg/2., theCopy=True)
  boxYMOIN = geompy.TranslateDXDYDZ(box, -larg/2., -larg,    -larg/2., theCopy=True)

  boxZPLUS = geompy.TranslateDXDYDZ(box, -larg/2., -larg/2., 0.,       theCopy=True)
  boxZMOIN = geompy.TranslateDXDYDZ(box, -larg/2., -larg/2., -larg,    theCopy=True)


  from salome.smesh import smeshBuilder
  import SMESH

  smesh = smeshBuilder.New(theStudy)
  Nb_Segments_1 = smesh.CreateHypothesis('NumberOfSegments')
  Nb_Segments_1.SetNumberOfSegments( N )
  Length_From_Edges_1 = smesh.CreateHypothesis('LengthFromEdges')
  Regular_1D = smesh.CreateHypothesis('Regular_1D')
  NETGEN_2D_ONLY = smesh.CreateHypothesis('NETGEN_2D_ONLY', 'NETGENEngine')
  Hexa_3D = smesh.CreateHypothesis('Hexa_3D')
  Maillage_1 = smesh.Mesh(cube)
  status = Maillage_1.AddHypothesis(Nb_Segments_1)
  status = Maillage_1.AddHypothesis(Regular_1D)
  Quadrangle_2D = Maillage_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
  status = Maillage_1.AddHypothesis(Hexa_3D)
  isDone = Maillage_1.Compute()

  geometries = [boxXPLUS, boxXMOIN, boxYPLUS, boxYMOIN, boxZPLUS, boxZMOIN]

  noms = ['VOLXP', 'VOLXM', 'VOLYP', 'VOLYM', 'VOLZP', 'VOLZM']

  for cont, geo in enumerate(geometries):
    aCriteria = smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,SMESH.FT_Undefined, geo)
    aFilter_1 = smesh.GetFilterFromCriteria([aCriteria])
    aFilter_1.SetMesh(Maillage_1.GetMesh())
    VOLUME_temp = Maillage_1.GroupOnFilter( SMESH.VOLUME, noms[cont], aFilter_1 )

  nbAdded, Maillage_1, Group = Maillage_1.MakeBoundaryElements( SMESH.BND_2DFROM3D, '', '', 0, [ VOLUME_temp ])

  geometries = [boxX, boxXM, boxXP, 
                boxY, boxYM, boxYP, 
                boxZ, boxZM, boxZP]

  noms = ['FACEX', 'FACEXM', 'FACEXP', 
          'FACEY', 'FACEYM', 'FACEYP', 
          'FACEZ', 'FACEZM', 'FACEZP']

  for cont, geo in enumerate(geometries):
    aCriteria = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined, geo)
    aFilter_1 = smesh.GetFilterFromCriteria([aCriteria])
    aFilter_1.SetMesh(Maillage_1.GetMesh())
    FACE_temp = Maillage_1.GroupOnFilter( SMESH.FACE, noms[cont], aFilter_1 )
 
  Maillage_1.ExportMED( outFile, 0, SMESH.MED_V2_2, 1, None ,1)

  #if salome.sg.hasDesktop():
    #salome.sg.updateObjBrowser(1)



def cube2D(L, N, outFile):

  N=int(N)
  from salome.geom import geomBuilder

  geompy = geomBuilder.New(theStudy)

  eps=L*1.e-6

  O = geompy.MakeVertex(0, 0, 0)
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
  face = geompy.MakeFaceHW(L, L, 1)

  epais=1./(2.*N)
  larg=L*1.1

  boxX = geompy.MakeBoxDXDYDZ(epais, larg, epais)
  geompy.TranslateDXDYDZ(boxX, -epais/2., -larg/2., -epais/2.)
  boxXM = geompy.TranslateDXDYDZ(boxX, -L/2., 0., 0., theCopy=True)
  boxXP = geompy.TranslateDXDYDZ(boxX,  L/2., 0., 0., theCopy=True)

  boxY = geompy.MakeBoxDXDYDZ(larg, epais, epais)
  geompy.TranslateDXDYDZ(boxY, -larg/2., -epais/2., -epais/2.)
  boxYM = geompy.TranslateDXDYDZ(boxY, 0., -L/2., 0., theCopy=True)
  boxYP = geompy.TranslateDXDYDZ(boxY, 0.,  L/2., 0., theCopy=True)

  box = geompy.MakeBoxDXDYDZ(larg, larg, epais)

  boxXPLUS = geompy.TranslateDXDYDZ(box, 0.,       -larg/2., -epais/2., theCopy=True)
  boxXMOIN = geompy.TranslateDXDYDZ(box, -larg,    -larg/2., -epais/2., theCopy=True)

  boxYPLUS = geompy.TranslateDXDYDZ(box, -larg/2., 0.,       -epais/2., theCopy=True)
  boxYMOIN = geompy.TranslateDXDYDZ(box, -larg/2., -larg,    -epais/2., theCopy=True)


  from salome.smesh import smeshBuilder
  import SMESH

  smesh = smeshBuilder.New(theStudy)
  Nb_Segments_1 = smesh.CreateHypothesis('NumberOfSegments')
  Nb_Segments_1.SetNumberOfSegments( N )
  Length_From_Edges_1 = smesh.CreateHypothesis('LengthFromEdges')
  Regular_1D = smesh.CreateHypothesis('Regular_1D')
  NETGEN_2D_ONLY = smesh.CreateHypothesis('NETGEN_2D_ONLY', 'NETGENEngine')
  Maillage_1 = smesh.Mesh(face)

  status = Maillage_1.AddHypothesis(Nb_Segments_1)
  status = Maillage_1.AddHypothesis(Regular_1D)
  Quadrangle_2D = Maillage_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
  isDone = Maillage_1.Compute()

  geometries = [boxXPLUS, boxXMOIN, boxYPLUS, boxYMOIN]

  noms = ['FACEXP', 'FACEXM', 'FACEYP', 'FACEYM']

  for cont, geo in enumerate(geometries):
    aCriteria = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined, geo)
    aFilter_1 = smesh.GetFilterFromCriteria([aCriteria])
    aFilter_1.SetMesh(Maillage_1.GetMesh())
    FACE_temp = Maillage_1.GroupOnFilter( SMESH.FACE, noms[cont], aFilter_1 )

  nbAdded, Maillage_1, Group = Maillage_1.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [ FACE_temp ])

  geometries = [boxX, boxXM, boxXP, boxY, boxYM, boxYP]

  noms = ['EDGEX', 'EDGEXM', 'EDGEXP', 'EDGEY', 'EDGEYM', 'EDGEYP']

  for cont, geo in enumerate(geometries):
    aCriteria = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined, geo)
    aFilter_1 = smesh.GetFilterFromCriteria([aCriteria])
    aFilter_1.SetMesh(Maillage_1.GetMesh())
    EDGE_temp = Maillage_1.GroupOnFilter( SMESH.EDGE, noms[cont], aFilter_1 )
 
  Maillage_1.ExportMED( outFile, 0, SMESH.MED_V2_2, 1, None ,1)

  #if salome.sg.hasDesktop():
    #salome.sg.updateObjBrowser(1)