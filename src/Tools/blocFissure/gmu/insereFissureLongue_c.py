# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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
"""Insertion de fissure longue - maillage pipe fond fissure"""

import logging
import math

import salome
from salome.smesh import smeshBuilder
import SMESH

from .geomsmesh import geompy
from .geomsmesh import smesh

from .sortEdges import sortEdges
from .putName import putName
from .distance2 import distance2

def insereFissureLongue_c (pipeFondFiss, disques, rayons, demiCercles, demiCerclesPeau, generatrices, \
                           VerticesEndPipeFiss, verticesEdgePeauFiss, \
                           groupFaceFissInPipe, groupEdgeFondFiss, groupsDemiCerclesPipe, groupGenerFiss, \
                           profondeur, rayonPipe):
  """maillage pipe fond fissure"""
  logging.info('start')

  meshFondFiss = smesh.Mesh(pipeFondFiss)
  algo2d = meshFondFiss.Quadrangle(algo=smeshBuilder.QUADRANGLE)
  algo3d = meshFondFiss.Prism()
  putName(algo3d.GetSubMesh(), "pipe")
  putName(algo3d, "algo3d_pipe")
  putName(algo2d, "algo2d_pipe")

  for i, face in enumerate(disques):
    algo2d = meshFondFiss.Quadrangle(algo=smeshBuilder.RADIAL_QUAD,geom=face)
    putName(algo2d.GetSubMesh(), "disque", i)
    putName(algo2d, "algo2d_disque", i)

  for i, edge in enumerate(rayons):
    algo1d = meshFondFiss.Segment(geom=edge)
    hypo1d = algo1d.NumberOfSegments(4)
    putName(algo1d.GetSubMesh(), "rayon", i)
    putName(algo1d, "algo1d_rayon", i)
    putName(hypo1d, "hypo1d_rayon", i)

  for i, edge in enumerate(demiCercles):
    algo1d = meshFondFiss.Segment(geom=edge)
    hypo1d = algo1d.NumberOfSegments(6)
    putName(algo1d.GetSubMesh(), "demiCercle", i)
    putName(algo1d, "algo1d_demiCercle", i)
    putName(hypo1d, "hypo1d_demiCercle", i)

  generSorted, minlg, maxlg = sortEdges(generatrices)
  nbSegGenLong = int(math.sqrt(3.0)*maxlg/(profondeur - rayonPipe)) # on veut 2 triangles equilateraux dans la largeur de la face
  nbSegGenBout = 6
  logging.info("min %s, max %s, nombre de segments %s, nombre de generatrices %s", minlg, maxlg, nbSegGenLong, len(generSorted))
  for i, edge in enumerate(generSorted):
    algo1d = meshFondFiss.Segment(geom=edge)
    if i < 6:
      hypo1d = algo1d.NumberOfSegments(nbSegGenBout)
    else:
      hypo1d = algo1d.NumberOfSegments(nbSegGenLong)
    putName(algo1d.GetSubMesh(), "generatrice", i)
    putName(algo1d, "algo1d_generatrice", i)
    putName(hypo1d, "hypo1d_generatrice", i)

  disks = list()
  for i, face in enumerate(disques[:4]):
    name = "disk%d"%i
    disks.append(meshFondFiss.GroupOnGeom(face, name, SMESH.FACE))
  _ = meshFondFiss.GetMesh().UnionListOfGroups( disks, 'PEAUEXT' )

  _ = meshFondFiss.GroupOnGeom(VerticesEndPipeFiss[0], "PFOR", SMESH.NODE)
  _ = meshFondFiss.GroupOnGeom(VerticesEndPipeFiss[1], "PFEX", SMESH.NODE)

  _ = meshFondFiss.GroupOnGeom(groupFaceFissInPipe, "fisInPi", SMESH.FACE)
  _ = meshFondFiss.GroupOnGeom(groupEdgeFondFiss, "FONDFISS", SMESH.EDGE)
  _ = meshFondFiss.GroupOnGeom(groupEdgeFondFiss, "nfondfis", SMESH.NODE)

  groups_demiCercles = list()
  groupnodes_demiCercles = list()
  for i, group in enumerate(groupsDemiCerclesPipe):
    name = "Cercle%d"%i
    groups_demiCercles.append(meshFondFiss.GroupOnGeom(group, name, SMESH.EDGE))
    name = "nCercle%d"%i
    groupnodes_demiCercles.append(meshFondFiss.GroupOnGeom(group, name, SMESH.NODE))
  group_generFiss = meshFondFiss.GroupOnGeom(groupGenerFiss, "GenFiss", SMESH.EDGE)
  groupnode_generFiss = meshFondFiss.GroupOnGeom(groupGenerFiss, "GenFiss", SMESH.NODE)

  is_done = meshFondFiss.Compute()
  text = "meshFondFiss.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  grpNode0 = meshFondFiss.IntersectGroups(groupnode_generFiss, groupnodes_demiCercles[0], "Node0")
  grpNode1 = meshFondFiss.IntersectGroups(groupnode_generFiss, groupnodes_demiCercles[1], "Node1")
  idNode0 = grpNode0.GetID(1)
  idNode1 = grpNode1.GetID(1)
  coordsMesh = list()
  coordsMesh.append(meshFondFiss.GetNodeXYZ(idNode0))
  coordsMesh.append(meshFondFiss.GetNodeXYZ(idNode1))

  for vertex in verticesEdgePeauFiss:
    coord = geompy.PointCoordinates(vertex)
    if distance2(coord, coordsMesh[0]) < 0.1:
      meshFondFiss.MoveNode(idNode0, coord[0], coord[1], coord[2])
    if distance2(coord, coordsMesh[1]) < 0.1:
      meshFondFiss.MoveNode(idNode1, coord[0], coord[1], coord[2])

  for groupNodes in groupnodes_demiCercles:
    for idNode in groupNodes.GetListOfID():
      coordMesh = meshFondFiss.GetNodeXYZ(idNode)
      vertex = geompy.MakeVertex(coordMesh[0], coordMesh[1], coordMesh[2])
      minDist = 100000
      minCoord = None
      imin = -1
      for i, edge in enumerate(demiCerclesPeau):
        discoord = geompy.MinDistanceComponents(vertex, edge)
        if discoord[0] <minDist:
          minDist = discoord[0]
          minCoord = discoord[1:]
          imin = i
      if imin >= 0 and minDist > 1.E-6:
        logging.debug("node id moved : %s distance=%s", idNode, minDist)
        meshFondFiss.MoveNode(idNode, coordMesh[0] + minCoord[0], coordMesh[1] + minCoord[1], coordMesh[2] + minCoord[2])

  return meshFondFiss, groups_demiCercles, group_generFiss, nbSegGenLong, nbSegGenBout
