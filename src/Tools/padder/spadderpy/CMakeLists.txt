# Copyright (C) 2012-2016  EDF R&D
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

IF(SALOME_BUILD_GUI)
  ADD_SUBDIRECTORY(gui)
  ADD_SUBDIRECTORY(plugin)
ENDIF(SALOME_BUILD_GUI)

# --- scripts ---

# scripts / static
SET(py_SCRIPTS
  __init__.py
  configreader.py
)

# --- rules ---

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/padder.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/padder.cfg @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/padder.cfg DESTINATION ${SALOME_SMESH_INSTALL_PLUGINS})

SALOME_INSTALL_SCRIPTS("${py_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/smesh/spadder)
