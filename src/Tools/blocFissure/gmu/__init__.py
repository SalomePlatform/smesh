# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
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
"""init de la classe"""

import os
from . import initLog

# --- calcul path blocFissure

apath = initLog.__file__
isabs = os.path.isabs(apath)
pathGmu = os.path.split(apath)[0]
if isabs:
  pathBloc = os.path.join(pathGmu, '..')
else:
  pathBloc = os.path.join(os.getcwd(), pathGmu, '..')
pathBloc = os.path.normpath(pathBloc)
