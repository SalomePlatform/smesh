# Copyright (C) 2012-2016  EDF R&D
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
  cubeAngle2.py
  cubeAngle.py
  cubeCoin.py
  cubeMilieu.py
  cubeTransverse.py
  cylindre_2.py
  cylindre.py
  disquePerce.py
  ellipse_1.py
  ellipse_2.py
  eprouvetteCourbe.py
  eprouvetteDroite_2.py
  eprouvetteDroite.py
  execution_Cas.py
  faceGauche_2.py
  faceGauche.py
  fissureCoude_10.py
  fissureCoude_1.py
  fissureCoude_2.py
  fissureCoude_3.py
  fissure_Coude_4.py
  fissureCoude_4.py
  fissureCoude_5.py
  fissureCoude_6.py
  fissureCoude_7.py
  fissureCoude_8.py
  fissureCoude_9.py
  fissure_Coude.py
  vis_1.py
)

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/blocFissure/CasTests)
