#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

# [EDF32699]

from salome.kernel import salome
salome.salome_init()
from salome.kernel import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

from salome.kernel import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

import CORBA
import tempfile
from pathlib import Path

import unittest

FileContent = """TITRE
 AUTEUR = Youssef
FINSF


COOR_3D
N1       -1.000000000000000E+00 -1.000000000000000E+00  3.000000000000000E+00
N2        1.000000000000000E+00 -1.000000000000000E+00  3.000000000000000E+00
N3        1.000000000000000E+00  1.000000000000000E+00  3.000000000000000E+00
N4       -1.000000000000000E+00  1.000000000000000E+00  3.000000000000000E+00
N5       -0.000000000000000E+00 -1.000000000000000E+00  3.000000000000000E+00
N6        1.000000000000000E+00 -0.000000000000000E+00  3.000000000000000E+00
N7       -0.000000000000000E+00  1.000000000000000E+00  3.000000000000000E+00
N8       -1.000000000000000E+00 -0.000000000000000E+00  3.000000000000000E+00
N9        0.000000000000000E+00 -0.000000000000000E+00  3.000000000000000E+00
FINSF


QUAD4
M1          N1       N5      N9       N8
M2          N5       N2      N6       N9
M3          N9       N6      N3       N7
M4          N8       N9      N7       N4
FINSF

QUAD8
m5          N1       N2      N3       N4      N5      N6       N7
     N8
FINSF

QUAD9
lequad9     N1       N2      N3       N4      N5      N6       N7
     N8     N9
FINSF

GROUP_MA NOM = gauche
     M1     M4
FINSF

GROUP_MA NOM = gauche_et_quad8
     M1     M4     m5
FINSF

GROUP_MA NOM = gauche_et_quad8_et_quad9
     M1     M4     m5     lequad9
FINSF

FIN
"""

class TestMailInSMESH(unittest.TestCase):
    def test00(self):
        mailFileBaseName = "zetest.mail"
        with tempfile.TemporaryDirectory() as tmpdirname: 
            mailFile = str( ( Path( tmpdirname ) / mailFileBaseName ).absolute() )
            with open( mailFile, "w" ) as f:
                f.write( FileContent )

            aMeshes, aResult = smesh.CreateMeshesFromMAIL( mailFile )
            assert( aResult == SMESH.DRS_OK )
            assert( len(aMeshes) == 1 )
            aMesh = aMeshes[0]
            assert( aMesh.NbElements() == 6 )
            assert( aMesh.NbNodes() == 9 )
            assert( aMesh.NbGroups() == 3 )
            assert( set( [grp.GetName().strip() for grp in aMesh.GetGroups()] ) == set(['gauche_et_quad8', 'gauche', 'gauche_et_quad8_et_quad9']) )
            assert( [grp.GetTypes() for grp in aMesh.GetGroups()] == [[SMESH.FACE], [SMESH.FACE], [SMESH.FACE]] )

if __name__ == "__main__":
    unittest.main()
