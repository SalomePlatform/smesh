# Copyright (C) 2012-2013  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  __init__.py
  blocDefaut.py
  checkDecoupePartition.py
  commonSubShapes.py
  creeZoneDefautDansObjetSain.py
  creeZoneDefautFilling.py
  creeZoneDefautGeom.py
  creeZoneDefautMaillage.py
  distance2.py
  eliminateDoubles.py
  ellipsoideDefaut.py
  enleveDefaut.py
  extractionOrienteeMulti.py
  extractionOrientee.py
  facesCirculaires.py
  facesFissure.py
  facesToreInBloc.py
  facesVolumesToriques.py
  findWireEndVertices.py
  findWireIntermediateVertices.py
  fissureCoude.py
  fissureGenerique.py
  genereElemsFissureElliptique.py
  genereMeshCalculZoneDefaut.py
  geomsmesh.py
  getCentreFondFiss.py
  getStatsMaillageFissure.py
  getSubshapeIds.py
  initEtude.py
  initLog.py
  insereFissureElliptique.py
  insereFissureGenerale.py
  insereFissureLongue.py
  meshBlocPart.py
  orderEdgesFromWire.py
  partitionBlocDefaut.py
  partitionVolumeSain.py
  peauInterne.py
  produitMixte.py
  projettePointSurCourbe.py
  prolongeVertices.py
  prolongeWire.py
  propagateTore.py
  putName.py
  quadranglesToShape.py
  regroupeSainEtDefaut.py
  rotTrans.py
  shapesSurFissure.py
  shapeSurFissure.py
  sortEdges.py
  sortFaces.py
  sortGeneratrices.py
  sortSolids.py
  substractSubShapes.py
  testgmu.py
  toreFissure.py
  triedreBase.py
  whichSideMulti.py
  whichSide.py
  whichSideVertex.py
)

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS}/blocFissure/gmu)
