# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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
# Author : Guillaume Boulant (EDF) 
#
import salome_pluginsmanager

from spadderPlugin import runSpadderPlugin
from meshcut_plugin import MeshCut

salome_pluginsmanager.AddFunction('PADDER mesher',
                                  'Create a mesh with PADDER',
                                  runSpadderPlugin)

salome_pluginsmanager.AddFunction('MeshCut',
                                  'Cut a tetrahedron mesh by a plane',
                                  MeshCut)
