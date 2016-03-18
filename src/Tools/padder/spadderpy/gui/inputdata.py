# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2016  EDF R&D
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
# Author : Guillaume Boulant (EDF)
#

from salome.kernel.enumerate import Enumerate
from salome.kernel.datamodeler import DataModeler, TypeString, TypeInteger

# __MEM__: Note that this module does not depend on the SPADDER
# component on purpose (we could have use a derived structure of
# SPADDER_ORB.MeshJobParameter). This choice is made to ease the test
# and development of the gui part of the plugin. If this data
# structure becomes too important, we could make another arrangement
# and use directly a SPADDER_ORB.MeshJobParameter.

class InputData(DataModeler):
    MESHTYPES=Enumerate([
        'CONCRETE',
        'STEELBAR'
        ])

    def __init__(self):
        DataModeler.__init__(self)
        self.addAttribute(
            name  = "meshObject",
            void  = True
            )
        self.addAttribute(
            name  = "meshName",
            type  = TypeString,
            range = None
            )
        self.addAttribute(
            name  = "meshType",
            type  = TypeInteger,
            range = self.MESHTYPES.listvalues()
            )
        self.addAttribute(
            name  = "groupName",
            type  = TypeString,
            range = None
            )

#
# ==============================================================================
# Basic use cases and unit tests
# ==============================================================================
#
def TEST_getName():
    testdata = InputData()
    testdata.meshName   = "myMesh"
    testdata.meshObject = None
    testdata.meshType   = InputData.MESHTYPES.CONCRETE
    if testdata.meshName != "myMesh" :
        return False
    return True

if __name__ == "__main__":
    from salome.kernel.unittester import run
    run("inputdata","TEST_getName")
