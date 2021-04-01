# -*- coding: utf-8 -*-
# Copyright (C) 2016-2021  EDF R&D
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
"""Message d'erreur"""

class fissError(Exception):
  """ Usage:
  try:
    instructions()
  except:
    raise fissError(traceback.extract_stack(),"mon message")
  """
  def __init__(self, pile, msg):
    self.pile = pile
    self.msg = msg

  def __str__(self):
    return 'msg={}\npile={}\n'.format(self.msg, repr(self.pile))
