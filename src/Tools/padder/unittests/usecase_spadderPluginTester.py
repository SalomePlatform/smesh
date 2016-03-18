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

# Author(s): Guillaume Boulant (23/03/2011)
#

# This script illustrates the standard use case of the component
# SPADDERPluginTester from within a SALOME script. This component is
# dedicated to test purpose only

import salome
import SPADDERPluginTest

# We first have to update the SALOME components list by loading the
# SPADDER catalog (load on demand only) 
from salome.smesh import spadder
spadder.loadSpadderCatalog()

# Basic test
print "Basic tests"
c=salome.lcc.FindOrLoadComponent("FactoryServer","SPADDERPluginTester")
z=c.demo(2.,3.)

# Test of usage of KERNEL services from the test component
print "Test of usage of KERNEL services from the test component"
c.testkernel()

# Test of usage of SMESH engine from the test component
# WARN: the SMESH engine must be loaded first
print "Test of usage of SMESH engine from the test component"
import SMESH
salome.lcc.FindOrLoadComponent("FactoryServer","SMESH")
c.testsmesh(salome.myStudyId)

print "Test completed : OK"
