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

#import logging
from .geomsmesh import smesh

def putName (objmesh, name, i_suff=-1, i_pref=None):
  """Nommage des objets mesh

  @objmesh objet à nommer
  @name le nom brut
  @i_suff un éventuel suffixe
  @i_pref un éventuel préfixe
  """

  #texte = "Name = {}, i_suff = {}, i_pref = {}".format(name,i_suff,i_pref)
  #print(texte)
  #logging.info(texte)
  # suffixe éventuel :
  if ( i_suff >= 0 ):
    suffixe = "_{}".format(i_suff)
    name += suffixe

  # préfixe éventuel :
  if ( i_pref is not None):
    if isinstance(i_pref,int):
      prefixe = "Cas{:02d}_".format(i_pref)
    else:
      prefixe = "{}_".format(i_pref)
    name = prefixe + name
  #logging.info("Au final : %s", name)

  smesh.SetName(objmesh, name)
