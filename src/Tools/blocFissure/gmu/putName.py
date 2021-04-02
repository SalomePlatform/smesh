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

"""Nommage des objets mesh (algorithme, hypothèse, subMesh)"""

from .geomsmesh import smesh

def putName (objmesh, name, i_suff=-1, i_pref=-1):
  """Nommage des objets mesh

  @objmesh objet à nommer
  @name le nom brut
  @i_suff un éventuel suffixe
  @i_pref un éventuel préfixe
  """

  # suffixe éventuel :
  if i_suff >= 0:
    suffixe = "_{}".format(i_suff)
    name += suffixe

  # préfixe éventuel :
  if i_pref >= 0:
    prefixe = "Cas{:02d}_".format(i_pref)
    name = prefixe + name

  smesh.SetName(objmesh, name)
