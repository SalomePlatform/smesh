# Copyright (C) 2012-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(UsePyQt4)

# --- scripts ---

# scripts / static
SET(plugin_SCRIPTS
  __init__.py
  fissureCoude_ihm.py
  fissureCoude_plugin.py
)

# --- resources ---

# uic files / to be processed by pyuic
SET(_pyuic_files
  fissureCoude.ui
  fissureGenerale.ui
)

# scripts / pyuic wrappings
PYQT4_WRAP_UIC(_pyuic_SCRIPTS ${_pyuic_files})

# --- rules ---

SALOME_INSTALL_SCRIPTS("${plugin_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS}/blocFissure/ihm)

SALOME_INSTALL_SCRIPTS("${_pyuic_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS}/blocFissure/ihm)