# -*- coding: utf-8 -*-
# Copyright (C) 2011-2015  EDF R&D
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
import salome_pluginsmanager

from spadderPlugin import runSpadderPlugin
from meshcut_plugin import MeshCut
from yamsplug_plugin import YamsLct
from MGCleanerplug_plugin import MGCleanerLct
from blocFissure.ihm.fissureCoude_plugin import fissureCoudeDlg

salome_pluginsmanager.AddFunction('PADDER mesher',
                                  'Create a mesh with PADDER',
                                  runSpadderPlugin)

salome_pluginsmanager.AddFunction('MeshCut',
                                  'Cut a tetrahedron mesh by a plane',
                                  MeshCut)

salome_pluginsmanager.AddFunction('ReMesh with MGSurfOpt ( formerly Yams )',
                                  'Run Yams',
                                  YamsLct)

salome_pluginsmanager.AddFunction('ReMesh with MGCleaner',
                                  'Run MGCleaner',
                                  MGCleanerLct)

salome_pluginsmanager.AddFunction('Meshed Pipe with a crack',
                                  'Create a mesh with blocFissure tool',
                                  fissureCoudeDlg)

# ZCracks plugin requires the module EFICAS to be installed
# thus it is first tested if this module is available before
# adding the plugin to salome_pluginsmanager
enable_zcracks = True
try:
  import eficasSalome
except:
  enable_zcracks = False

if enable_zcracks:
  from zcracks_plugin import ZcracksLct
  salome_pluginsmanager.AddFunction('Run Zcrack',
                                    'Run Zcrack',
                                    ZcracksLct)
