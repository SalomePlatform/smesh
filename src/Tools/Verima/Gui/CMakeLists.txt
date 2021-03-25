# Copyright (C) 2012-2021  EDF R&D
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

INCLUDE(UsePyQt)

# --- scripts ---

# scripts / static
SET(_plugin_SCRIPTS
  maFenetreChoix.py
  monEditor.py
  monNomBase.py
  visualiseDatabase.py
  __init__.py
)

# uic files / to be processed by pyuic
SET(_pyuic_FILES
  desFenetreChoix.ui
  desStat.ui
  myMainTotale.ui
  myMain.ui
  tousLesJobs.ui
  nomBase.ui
  tousLesJobs.ui
)
# scripts / pyuic wrappings
PYQT_WRAP_UIC(_pyuic_SCRIPTS ${_pyuic_FILES} TARGET_NAME _target_name_pyuic)

# --- rules ---

SALOME_INSTALL_SCRIPTS("${_plugin_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS}/Verima/Gui)

SALOME_INSTALL_SCRIPTS("${_pyuic_SCRIPTS}" ${SALOME_SMESH_INSTALL_PLUGINS}/Verima/Gui TARGET_NAME _target_name_pyuic_py)
# add dependency of compiled py files on uic files in order
# to avoid races problems when compiling in parallel
ADD_DEPENDENCIES(${_target_name_pyuic_py} ${_target_name_pyuic})
