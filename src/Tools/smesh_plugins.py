# -*- coding: utf-8 -*-
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
import salome_pluginsmanager
import os

try:
  from spadderPlugin import runSpadderPlugin
  salome_pluginsmanager.AddFunction('PADDER mesher',
                                    'Create a mesh with PADDER',
                                    runSpadderPlugin)
except:
  salome_pluginsmanager.logger.info('ERROR: PADDER mesher plug-in is unavailable')
  pass

try:
  from meshcut_plugin import MeshCut
  salome_pluginsmanager.AddFunction('MeshCut',
                                    'Cut a tetrahedron mesh by a plane',
                                    MeshCut)

except:
  salome_pluginsmanager.logger.info('ERROR: MeshCut plug-in is unavailable')
  pass

try:
  from yamsplug_plugin import YamsLct
  salome_pluginsmanager.AddFunction('ReMesh with MGSurfOpt',
                                    'Run Yams',
                                    YamsLct)
except:
  salome_pluginsmanager.logger.info('ERROR: MGSurfOpt plug-in is unavailable')
  pass

try:
  from MGCleanerplug_plugin import MGCleanerLct
  salome_pluginsmanager.AddFunction('ReMesh with MGCleaner',
                                    'Run MGCleaner',
                                    MGCleanerLct)
except:
  salome_pluginsmanager.logger.info('ERROR: MGCleaner plug-in is unavailable')
  pass

try:
  from blocFissure.ihm.fissureCoude_plugin import fissureCoudeDlg
  salome_pluginsmanager.AddFunction('Meshed Pipe with a crack (blocFissure plugin)',
                                    'Create a mesh with blocFissure tool',
                                    fissureCoudeDlg)
except:
  salome_pluginsmanager.logger.info('ERROR: Meshed Pipe with a crack plug-in is unavailable')
  pass
try:
  from blocFissure.ihm.fissureGenerale_plugin import fissureGeneraleDlg
  salome_pluginsmanager.AddFunction('Add a crack in a mesh (blocFissure plugin)',
                                    'Insert a crack in an hexahedral mesh with blocFissure tool',
                                    fissureGeneraleDlg)
except:
  salome_pluginsmanager.logger.info('ERROR: Meshed Pipe with a crack plug-in is unavailable')
  pass

# ZCracks plugin requires the Zcracks tool
try:
  zcracksHome=os.environ['ZCRACKSHOME']
  if len(zcracksHome) > 1:
    #print 'ZCRACKSHOME ', zcracksHome
    from Zcracks.zcracks_plugin import ZcracksLct
    salome_pluginsmanager.AddFunction('Run Zcrack',
                                      'Run Zcrack',
                                      ZcracksLct)
except:
  #print 'probleme zcracks'
  salome_pluginsmanager.logger.info('ERROR: Zcrack plug-in is unavailable')
  pass
