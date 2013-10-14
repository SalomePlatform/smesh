# Copyright (C) 2012-2013  CEA/DEN, EDF R&D
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

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  dataBase.py
  exportToCSV.py
  importFromCSV.py
  tableDeBase.py
  tableGroupes.py
  tableGroupesRef.py
  tableMachines.py
  tableMaillages.py
  tableMailles.py
  tableMailleurs.py
  tablePerfs.py
  tableRatios.py
  tableTailles.py
  tableVersions.py
  versions.py
  ) 

SET(plugin_DB
  myMesh.db
  )

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS})
SALOME_INSTALL_SCRIPTS("${plugin_DB}" ${SALOME_SMESH_INSTALL_PLUGINS})
