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
SET(_py_SCRIPTS
  __init__.py
  plugindialog.py
  inputdialog.py
  inputdata.py
)

# --- resources ---

# uic files / to be processed by pyuic
SET(_spadderpy_DATA
  parameters.png
  input.png
  select.png
  compute.png
  refresh.png
  publish.png
  clear.png
  addinput.png
  deleteinput.png
  concrete.png
  steelbar.png
)

SET(_pyuic_FILES
  plugindialog.ui
  inputframe.ui
)

# scripts / pyuic wrappings
PYQT_WRAP_UIC(_pyuic_SCRIPTS ${_pyuic_FILES} TARGET_NAME _target_name_pyuic)

# --- rules ---
SALOME_INSTALL_SCRIPTS("${_pyuic_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/smesh/spadder/gui TARGET_NAME _target_name_pyuic_py)
# add dependency of compiled py files on uic files in order
# to avoid races problems when compiling in parallel
ADD_DEPENDENCIES(${_target_name_pyuic_py} ${_target_name_pyuic})

SALOME_INSTALL_SCRIPTS("${_py_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/smesh/spadder/gui)

INSTALL(FILES ${_spadderpy_DATA} DESTINATION ${SALOME_INSTALL_PYTHON}/salome/smesh/spadder/gui)
