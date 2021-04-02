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
"""maillage sain sans la zone de défaut"""

import logging


def enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges):
  """Maillage sain sans la zone de défaut

  TODO: a completer
  """
  logging.info('start')

  maillageSain.RemoveGroupWithContents(zoneDefaut)

  if zoneDefaut_skin is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_skin)

  if zoneDefaut_internalFaces is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_internalFaces)

  if zoneDefaut_internalEdges is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_internalEdges)

  _ = maillageSain.RemoveOrphanNodes()

  return maillageSain
