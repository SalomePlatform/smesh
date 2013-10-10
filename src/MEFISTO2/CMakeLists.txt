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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${PLATFORM_INCLUDES}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKernel}
  ${CAS_TKMath}
  ${KERNEL_SALOMELocalTrace}
)

# --- headers ---

# header files / no moc processing
SET(MEFISTO2D_HEADERS
  aptrte.h 
  Rn.h
)

# --- sources ---

# sources / static
SET(MEFISTO2D_SOURCES
  aptrte.cxx
  trte.f
)

# --- rules ---

ADD_LIBRARY(MEFISTO2D ${MEFISTO2D_SOURCES})
TARGET_LINK_LIBRARIES(MEFISTO2D ${_link_LIBRARIES} )
INSTALL(TARGETS MEFISTO2D EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MEFISTO2D_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
