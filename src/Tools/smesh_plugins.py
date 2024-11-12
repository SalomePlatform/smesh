# -*- coding: utf-8 -*-
# Copyright (C) 2011-2024  CEA, EDF
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
import os
from qtsalome import QIcon

try:
  from spadderPlugin import runSpadderPlugin
  salome_pluginsmanager.AddFunction('PADDER mesher',
                                    'Create a mesh with PADDER',
                                    runSpadderPlugin)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: PADDER mesher plug-in is unavailable: {}'.format(e))
  pass

try:
  from meshcut_plugin import MeshCut
  icon_file = os.path.join(os.getenv('SMESH_ROOT_DIR'),'share', 'salome', 'resources', 'smesh', 'mesh_plugins_meshcut.png')
  salome_pluginsmanager.AddFunction('MeshCut',
                                    'Cut a tetrahedron mesh by a plane',
                                    MeshCut, icon=QIcon(icon_file))

except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: MeshCut plug-in is unavailable: {}'.format(e))
  pass

try:
  from yamsplug_plugin import YamsLct
  salome_pluginsmanager.AddFunction('ReMesh with MGSurfOpt',
                                    'Run Yams',
                                    YamsLct)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: MGSurfOpt plug-in is unavailable: {}'.format(e))
  pass

try:
  from MGCleanerplug_plugin import MGCleanerLct
  salome_pluginsmanager.AddFunction('ReMesh with MGCleaner',
                                    'Run MGCleaner',
                                    MGCleanerLct)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: MGCleaner plug-in is unavailable: {}'.format(e))
  pass

try:
  from mmgplugin.mmgPlug_plugin import Mmg
  salome_pluginsmanager.AddFunction('ReMesh with MMG',
                                    'Run MMG',
                                    Mmg)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: MMG plug-in is unavailable: {}'.format(e))
  pass

try:
  from meshbooleanplugin.mesh_boolean_plugin import MeshBoolean
  salome_pluginsmanager.AddFunction('Boolean Mesh Operations', 'Perform boolean operations on meshes', MeshBoolean)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: MeshBoolean plug-in is unavailable: {}'.format(e))
  pass

try:
  from blocFissure.ihm.fissureCoude_plugin import fissureCoudeDlg
  salome_pluginsmanager.AddFunction('Meshed Pipe with a crack (blocFissure plugin)',
                                    'Create a mesh with blocFissure tool',
                                    fissureCoudeDlg)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: Meshed Pipe with a crack plug-in is unavailable: {}'.format(e))
  pass
try:
  from blocFissure.ihm.fissureGenerale_plugin import fissureGeneraleDlg
  salome_pluginsmanager.AddFunction('Add a crack in a mesh (blocFissure plugin)',
                                    'Insert a crack in an hexahedral mesh with blocFissure tool',
                                    fissureGeneraleDlg)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: Meshed Pipe with a crack plug-in is unavailable: {}'.format(e))
  pass

# ZCracks plugin requires the Zcracks tool
try:
  zcracksHome=os.environ['ZCRACKSHOME']
  if len(zcracksHome) > 1:
    #print 'ZCRACKSHOME ', zcracksHome
    from Zcracks.zcracks_plugin import ZcracksLct
    salome_pluginsmanager.AddFunction('Run Zcracks',
                                      'Add a crack in a mesh with Zcracks plug-in',
                                      ZcracksLct)
except Exception as e:
  #print 'problem zcracks'
  salome_pluginsmanager.logger.info('ERROR: Zcrack plug-in is unavailable: {}'.format(e))
  pass

# Topological to volumic mesh plugin
try:
  from TopIIVolMeshPlugin import TopIIVolMeshLct
  salome_pluginsmanager.AddFunction('Run Topological Volumic mesher',
                                    'run topological volumic mesher',
                                    TopIIVolMeshLct)
except Exception as e:
  salome_pluginsmanager.logger.info('ERROR: TopIIVolMesh plug-in is unavailable: {}'.format(e))
  pass

import sys
import importlib
import importlib.util

smesh_pyplugin_dir = os.getenv("SMESH_PYPLUGIN_DIR")
if smesh_pyplugin_dir:
  plugin_dirs = smesh_pyplugin_dir.split(":")
  # reverse plugin_dirs list to call smesh python plugin in good order
  plugin_dirs.reverse()
  for plg_dir in plugin_dirs:
    if os.path.exists(plg_dir):
      sys.path.insert(0, plg_dir)
      #for filename in sorted(
      #        filter(lambda x: os.path.isfile(os.path.join(plg_dir, x)),
      #        os.listdir(plg_dir))):
      for filename in os.listdir(plg_dir):
        if filename.endswith(".py"):
            f = os.path.join(plg_dir, filename)
            try:
              module_name = os.path.splitext(os.path.basename(f))[0]
              _specs = importlib.util.find_spec(module_name)
              _module = importlib.util.module_from_spec(_specs)
              _specs.loader.exec_module(_module)
              _module.init()
            except:
              print("Can not load python plugin from {}".format(plg_dir))
