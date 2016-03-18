#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder

# retrieve SMESH engine in try/except block
# to avoid problems in some cases, e.g. when generating documentation
try:
    # get instance of class smeshBuilder
    engineSmesh = salome.lcc.FindOrLoadComponent( "FactoryServer", "SMESH" )
    smesh = smeshBuilder.New(salome.myStudy, engineSmesh)
except:
    print "exception in smesh.py: instance creation failed"
    smesh = None
    pass

# load plugins and add dynamically generated methods to Mesh class,
# the same for for global variables declared by plug-ins
from salome.smesh.smeshBuilder import *
from salome.smesh.smeshBuilder import Mesh, algoCreator
for pluginName in os.environ[ "SMESH_MeshersList" ].split( ":" ):
    #
    #print "pluginName: ", pluginName
    pluginBuilderName = pluginName + "Builder"
    try:
        exec( "from salome.%s.%s import *" % (pluginName, pluginBuilderName))
    except Exception, e:
        from salome_utils import verbose
        if verbose(): print "Exception while loading %s: %s" % ( pluginBuilderName, e )
        continue
    exec( "from salome.%s import %s" % (pluginName, pluginBuilderName))
    plugin = eval( pluginBuilderName )

    # add methods creating algorithms to Mesh
    for k in dir( plugin ):
        if k[0] == '_': continue
        algo = getattr( plugin, k )
        if type( algo ).__name__ == 'classobj' and hasattr( algo, "meshMethod" ):
            if not hasattr( Mesh, algo.meshMethod ):
                setattr( Mesh, algo.meshMethod, algoCreator() )
                pass
            getattr( Mesh, algo.meshMethod ).add( algo )
            pass
        pass
    pass
del pluginName

# export the methods of smeshBuilder
if smesh:
    for k in dir( smesh ):
	if k[0] == '_': continue
	globals()[k] = getattr( smesh, k )
    del k
    pass

print """
===============================================================================
WARNING:
Usage of smesh.py is deprecated in SALOME V7.2!
smesh.py will be removed in a future version!
TODO:
The following changes in your scripts are required to avoid this message:

replace 
-------

import smesh, SMESH
smesh.SetCurrentStudy(theStudy)

with
----

import SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

you also need to modify some lines where smeshBuilder is used instead of smesh

algo=smesh.xxxx  ==> algo=smeshBuilder.xxxx 

See also SMESH User's Guide for more details

WARNING:
The smesh.py module works correctly only in the first created study.
It does not work in the second, third, etc studies!

===============================================================================
"""
