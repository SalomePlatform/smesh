# Copyright (C) 2012-2021  EDF R&D
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

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  __init__.py
  ajustePointsEdgePipeFissure.py
  blocDefaut.py
  calculePointsAxiauxPipe.py
  calculePointsAxiauxPipe_a.py
  calculePointsAxiauxPipe_b.py
  calculePointsAxiauxPipe_c.py
  casStandard.py
  checkDecoupePartition.py
  commonSubShapes.py
  compoundFromList.py
  construitEdgesRadialesDebouchantes.py
  construitEdgesRadialesDebouchantes_a.py
  construitEdgesRadialesDebouchantes_b.py
  construitEdgesRadialesDebouchantes_c.py
  construitFissureGenerale.py
  construitFissureGenerale_a.py
  construitFissureGenerale_b.py
  construitFissureGenerale_c.py
  construitMaillagePipe_a.py
  construitMaillagePipe_b.py
  construitMaillagePipe_c.py
  construitMaillagePipe_d.py
  construitMaillagePipe.py
  construitPartitionsPeauFissure.py
  creePointsPipePeau.py
  creeZoneDefautDansObjetSain.py
  creeZoneDefautFilling.py
  creeZoneDefautGeom.py
  creeZoneDefautMaillage.py
  distance2.py
  eliminateDoubles.py
  elimineExtremitesPipe.py
  ellipsoideDefaut.py
  enleveDefaut.py
  extractionOrienteeMulti.py
  extractionOrientee.py
  extractionOrientee_a.py
  facesCirculaires.py
  facesFissure.py
  facesToreInBloc.py
  facesVolumesToriques.py
  findWireEndVertices.py
  findWireIntermediateVertices.py
  findWireVertices.py
  fissError.py
  fissureCoude.py
  fissureGenerique.py
  fusionMaillageAttributionDefaut.py
  genereElemsFissureElliptique.py
  genereMeshCalculZoneDefaut.py
  geomsmesh.py
  getCentreFondFiss.py
  getStatsMaillageFissure.py
  getSubshapeIds.py
  identifieEdgesPeau_a.py
  identifieEdgesPeau_b.py
  identifieEdgesPeau_c.py
  identifieEdgesPeau.py
  identifieElementsDebouchants_a.py
  identifieElementsDebouchants_b.py
  identifieElementsDebouchants_c.py
  identifieElementsDebouchants_d.py
  identifieElementsDebouchants.py
  identifieElementsFissure.py
  identifieElementsGeometriquesPeau.py
  identifieFacesEdgesFissureExterne.py
  identifieFacesPeau.py
  initEtude.py
  initLog.py
  insereFissureElliptique.py
  insereFissureLongue_a.py
  insereFissureLongue_b.py
  insereFissureLongue_c.py
  insereFissureLongue_d.py
  insereFissureLongue_e.py
  insereFissureLongue_f.py
  insereFissureLongue_g.py
  insereFissureLongue.py
  listOfExtraFunctions.py
  mailleAretesEtJonction.py
  mailleFacesFissure.py
  mailleFacesPeau.py
  meshBlocPart.py
  orderEdgesFromWire.py
  partitionBlocDefaut.py
  partitionneFissureParPipe.py
  partitionVolumeSain.py
  peauInterne.py
  produitMixte.py
  projettePointSurCourbe.py
  prolongeVertices.py
  prolongeWire.py
  propagateTore.py
  putName.py
  quadranglesToShapeNoCorner.py
  quadranglesToShape.py
  quadranglesToShapeWithCorner.py
  regroupeSainEtDefaut.py
  restreintFaceFissure.py
  rotTrans.py
  shapesSurFissure.py
  shapeSurFissure.py
  sortEdges.py
  sortFaces.py
  sortGeneratrices.py
  sortSolids.py
  substractSubShapes.py
  toreFissure.py
  triedreBase.py
  trouveEdgesFissPeau.py
  whichSideMulti.py
  whichSide.py
  whichSideVertex.py
)

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/blocFissure/gmu)
