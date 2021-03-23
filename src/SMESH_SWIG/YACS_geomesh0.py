#!/usr/bin/env python3
# Copyright (C) 2018-2021  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# test used in YACS, ForEachLoop

def geomesh(l0, r0, h0, d0, d1, my_container, direc):
  print("Géometrie et maillage barre : (l0, r0, h0, d0, d1)=", (l0, r0, h0, d0, d1))

  import sys
  import salome
  salome.salome_init()
  import GEOM
  from salome.geom import geomBuilder
  import math
  import os
  import SALOMEDS
  import  SMESH
  from salome.smesh import smeshBuilder
  
  my_container.load_component_Library("GEOM")
  #engineGeom = my_container.create_component_instance("GEOM")
  engineGeom = my_container.load_impl("GEOM","")
  geompy = geomBuilder.New(engineGeom)
  my_container.load_component_Library("SMESH")
  #engineSmesh = my_container.create_component_instance("SMESH")
  engineSmesh = my_container.load_impl("SMESH","")
  smesh = smeshBuilder.New(engineSmesh,engineGeom)
  print("instances Names:", engineGeom.instanceName, engineSmesh.instanceName)
  print("instances:", engineGeom, engineSmesh)
  print("builders:", geompy, smesh)
  
  volume = (2.*l0*r0 + 0.75*math.pi*r0*r0)*h0
  O = geompy.MakeVertex(0, 0, 0)
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
  Vertex_1 = geompy.MakeVertex(-l0, -r0, 0)
  Vertex_2 = geompy.MakeVertex(-l0-r0, 0, 0)
  Vertex_3 = geompy.MakeVertex(-l0, r0, 0)
  Vertex_4 = geompy.MakeVertex(l0, r0, 0)
  Vertex_5 = geompy.MakeVertex(l0+r0, 0, 0)
  Vertex_6 = geompy.MakeVertex(l0, -r0, 0)
  Arc_1 = geompy.MakeArc(Vertex_1, Vertex_2, Vertex_3)
  Arc_2 = geompy.MakeArc(Vertex_4, Vertex_5, Vertex_6)
  Line_1 = geompy.MakeLineTwoPnt(Vertex_3, Vertex_4)
  Line_2 = geompy.MakeLineTwoPnt(Vertex_6, Vertex_1)
  Face_1 = geompy.MakeFaceWires([Arc_1, Arc_2, Line_1, Line_2], 1)
  barre0 = geompy.MakePrismVecH(Face_1, OZ, h0)
  Vertex_1a = geompy.MakeVertex(-l0, -r0/2, 0)
  Vertex_2a = geompy.MakeVertex(-l0-r0/2, 0, 0)
  Vertex_3a = geompy.MakeVertex(-l0, r0/2, 0)
  Vertex_4a = geompy.MakeVertex(l0, r0/2, 0)
  Vertex_5a = geompy.MakeVertex(l0+r0/2, 0, 0)
  Vertex_6a = geompy.MakeVertex(l0, -r0/2, 0)
  Arc_1a = geompy.MakeArc(Vertex_1a, Vertex_2a, Vertex_3a)
  Arc_2a = geompy.MakeArc(Vertex_4a, Vertex_5a, Vertex_6a)
  Line_1a = geompy.MakeLineTwoPnt(Vertex_3a, Vertex_4a)
  Line_2a = geompy.MakeLineTwoPnt(Vertex_6a, Vertex_1a)
  Face_1a = geompy.MakeFaceWires([Arc_1a, Arc_2a, Line_1a, Line_2a], 1)
  barrea = geompy.MakePrismVecH(Face_1a, OZ, h0)
  barreb = geompy.MakeCut(barre0, barrea)
  Plane_1 = geompy.MakePlane(Vertex_1, OX, 2000)
  Plane_2 = geompy.MakePlane(Vertex_6, OX, 2000)
  barre = geompy.MakePartition([barreb], [Plane_1, Plane_2], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  v1 = geompy.MakeVertex(-l0-r0, 0, h0/2.)
  v2 = geompy.MakeVertex(l0+r0, 0, h0/2.)
  f1 = geompy.GetShapesNearPoint(barre, v1, geompy.ShapeType["FACE"])
  f2 = geompy.GetShapesNearPoint(barre, v2, geompy.ShapeType["FACE"])
  #f1 = geompy.CreateGroup(barre, geompy.ShapeType["FACE"])
  #geompy.UnionIDs(f1, [3])
  #f2 = geompy.CreateGroup(barre, geompy.ShapeType["FACE"])
  #geompy.UnionIDs(f2, [20])

  Auto_group_for_Sub_mesh_1 = geompy.CreateGroup(barre, geompy.ShapeType["FACE"])
  geompy.UnionList(Auto_group_for_Sub_mesh_1, [f1, f2])
  nom = r'barre_l_{:03d}__r_{:05.2f}__h_{:05.2f}__d0_{:05.2f}__d1_{:05.2f}'.format(int(l0), r0, h0, d0, d1)
  nombrep = nom + ".brep"
  geompy.ExportBREP(barre, direc + os.sep + nombrep )
  props = geompy.BasicProperties(barre)
  geomvol = props[2]

  #geompy.addToStudy( barre, 'barre' )
  #geompy.addToStudyInFather( barre, f1, 'f1' )
  #geompy.addToStudyInFather( barre, f2, 'f2' )

  smesh.SetEnablePublish( False )

  isTetra = False
  barre_1 = smesh.Mesh(barre)
  # SO = salome.myStudy.FindObjectIOR(salome.myStudy.ConvertObjectToIOR(barre_1.GetMesh()))
  # if SO:
  #   print ("_______",SO.GetID(),SO.GetName())
  # else:
  #   print ("_______NO_SO!!!")
  if (isTetra):
    NETGEN_1D_2D_3D = barre_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
    NETGEN_3D_Parameters_1 = NETGEN_1D_2D_3D.Parameters()
    NETGEN_3D_Parameters_1.SetMaxSize( d0 )
    NETGEN_3D_Parameters_1.SetSecondOrder( 0 )
    NETGEN_3D_Parameters_1.SetOptimize( 1 )
    NETGEN_3D_Parameters_1.SetFineness( 3 )
    NETGEN_3D_Parameters_1.SetChordalError( 0.1 )
    NETGEN_3D_Parameters_1.SetChordalErrorEnabled( 0 )
    NETGEN_3D_Parameters_1.SetMinSize( d0 )
    NETGEN_3D_Parameters_1.SetUseSurfaceCurvature( 1 )
    NETGEN_3D_Parameters_1.SetFuseEdges( 1 )
    NETGEN_3D_Parameters_1.SetQuadAllowed( 0 )
    NETGEN_1D_2D = barre_1.Triangle(algo=smeshBuilder.NETGEN_1D2D,geom=Auto_group_for_Sub_mesh_1)
    NETGEN_2D_Parameters_1 = NETGEN_1D_2D.Parameters()
    NETGEN_2D_Parameters_1.SetMaxSize( d1 )
    NETGEN_2D_Parameters_1.SetSecondOrder( 0 )
    NETGEN_2D_Parameters_1.SetOptimize( 1 )
    NETGEN_2D_Parameters_1.SetFineness( 3 )
    NETGEN_2D_Parameters_1.SetChordalError( 0.1 )
    NETGEN_2D_Parameters_1.SetChordalErrorEnabled( 0 )
    NETGEN_2D_Parameters_1.SetMinSize( d1 )
    NETGEN_2D_Parameters_1.SetUseSurfaceCurvature( 1 )
    NETGEN_2D_Parameters_1.SetFuseEdges( 1 )
    NETGEN_2D_Parameters_1.SetQuadAllowed( 0 )
  else:
    Regular_1D = barre_1.Segment()
    Number_of_Segments_1 = Regular_1D.NumberOfSegments(15)
    Quadrangle_2D = barre_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    Hexa_3D = barre_1.Hexahedron(algo=smeshBuilder.Hexa)   
  isDone = barre_1.Compute()
  
  f1_1 = barre_1.GroupOnGeom(f1,'f1',SMESH.FACE)
  f2_1 = barre_1.GroupOnGeom(f2,'f2',SMESH.FACE)
  smesh.SetName(barre_1, nom)
  nommed = nom + ".med"
  barre_1.ExportMED( direc + os.sep + nommed, auto_groups=0, minor=0, overwrite=1, meshPart=None, autoDimension=1 )
  measure = smesh.CreateMeasurements()
  meshvol = measure.Volume(barre_1.mesh)
  print("Maillage publié : ", direc + os.sep + nommed)
  clearMesh(barre_1, salome.myStudy, nom)
  deltag = abs(geomvol - volume)/volume
  deltam = abs(meshvol - geomvol)/geomvol
  delta = abs(meshvol - volume)/volume
  print("volumes:", volume, geomvol, meshvol, deltag, deltam)
  assert(deltag < 1.e-5)
  assert(deltam < 2.e-3)
  #import time
  #time.sleep(30)
  return delta


def clearMesh(theMesh, theStudy, aName):
  theMesh.Clear()
  aMesh = theMesh.GetMesh()
  aMesh.UnRegister()
  # aStudyBuilder = theStudy.NewBuilder()
  # SO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(aMesh))
  # objects_to_unpublish = [SO]
  # refs = theStudy.FindDependances(SO)
  # objects_to_unpublish += refs
  # for o in objects_to_unpublish:
  #   if o is not None:
  #     aStudyBuilder.RemoveObjectWithChildren(o)
  print("clearMesh done:", aName)

def genere(r0, h0, my_container, direc):
  l0 = 50.0
  d0 = min(r0/2., h0/6.)
  d1 = d0/2.
  res = geomesh(l0, r0, h0, d0, d1, my_container, direc)
  return res

def genere2(r0h0, my_container, direc):
  l0 = 50.0
  r0 = r0h0[0]
  h0 = r0h0[1]
  d0 = min(r0/2., h0/6.)
  d1 = d0/2.
  res = geomesh(l0, r0, h0, d0, d1, my_container, direc)
  return res
