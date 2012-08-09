#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  File   : smesh.py
#  Author : Francis KLOSS, OCC
#  Module : SMESH
#
"""@package smesh
 \brief Module smesh
"""

import salome
from salome import *

import geompy
import smeshDC
from smeshDC import *

# get instance of class smeshDC
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.init_smesh(salome.myStudy,geompy.geom)

# load plugins
from smeshDC import Mesh, algoCreator
for pluginName in os.environ["SMESH_MeshersList"].split(":"):

  pluginName += "DC"
  try:
    exec("from %s import *" % pluginName )
  except Exception, e:
    print "Exception while loading %s: %s" % ( pluginName, e )
    continue
  exec("import %s" % pluginName )
  plugin = eval(pluginName)

  # add methods creating algorithms to Mesh
  for k in dir(plugin):
    if k[0] == '_':continue
    algo = getattr(plugin,k)
    if type( algo ).__name__ == 'classobj' and hasattr( algo, "meshMethod"):
      if not hasattr( Mesh, algo.meshMethod ):
        setattr( Mesh, algo.meshMethod, algoCreator())
      getattr( Mesh, algo.meshMethod ).add( algo )

# Export the methods of smeshDC
for k in dir(smesh):
  if k[0] == '_':continue
  globals()[k]=getattr(smesh,k)
del k
