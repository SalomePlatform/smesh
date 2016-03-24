# Copyright (C) 2007-2016  EDF R&D
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

SET(PADDER_RESOURCES_FILES
  testdata/concrete.med
  testdata/ferraill.med
  testdata/teststudy.hdf
)

INSTALL(FILES ${PADDER_RESOURCES_FILES} DESTINATION ${SALOME_SMESH_INSTALL_RES_DATA}/padderdata)

MESSAGE(STATUS "Creation of ${CMAKE_CURRENT_BINARY_DIR}/SPADDERCatalog.xml")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/SPADDERCatalog.xml ${CMAKE_CURRENT_BINARY_DIR}/SPADDERCatalog.xml @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/SPADDERCatalog.xml DESTINATION ${SALOME_SMESH_INSTALL_RES_DATA})