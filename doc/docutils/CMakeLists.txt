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

# This file mainly prepares the environment in which sphinx should run:
#  this sets the PYTHONPATH and LD_LIBRARY_PATH to include OMNIORB, DOCUTILS, SETUPTOOLS, etc ...

# Prepare Sphinx options
SET(SPHINXOPTS
)
SET(SOURCEDIR
  ${CMAKE_CURRENT_SOURCE_DIR}
)
SET(SPHINXBUILD
  sphinx-build
)
SET(PAPEROPT_a4
  -D
  latex_paper_size=a4
)
SET(ALLSPHINXOPTS
  -d
  doctrees
  ${PAPEROPT_a4}
  ${SPHINXOPTS}
  ${SOURCEDIR}
)

SALOME_CONFIGURE_FILE(conf.py.in conf.py)

# Write the shell file preparing the env for the Sphinx run:
#    This uses the paths to SPHINX, DOCUTILS and SETUPTOOLS:
#    Their root dir were found when looking for Sphinx (FindSalomeSphinx.cmake):

SET(_pydir python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
IF(WIN32)
  STRING(REPLACE "/" "\\" SCR "@SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}\\lib\\${_pydir}\\site-packages\\salome\;%PYTHONPATH%
  @SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}\\bin\\salome\;%PYTHONPATH%
  @SET PATH=${CMAKE_INSTALL_PREFIX}\\lib\\salome\;%PATH%
  @SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}\\${SALOME_INSTALL_PYTHON}\;%PYTHONPATH%
  @SET PYTHONPATH=${CMAKE_INSTALL_PREFIX}\\${SALOME_INSTALL_PYTHON_SHARED}\;%PYTHONPATH%
  @SET PYTHONPATH=${HDF5_ROOT_DIR}\\lib\;%PYTHONPATH%
  @SET PYTHONPATH=${MPI_ROOT_DIR}\\lib\;%PYTHONPATH%

  @SET PYTHONPATH=${KERNEL_ROOT_DIR}\\bin\\salome\;%PYTHONPATH%
  @SET PATH=${KERNEL_ROOT_DIR}\\lib\\salome\;%PATH%
  @SET PYTHONPATH=${KERNEL_ROOT_DIR}\\${SALOME_INSTALL_PYTHON}\;%PYTHONPATH%
  @SET PYTHONPATH=${KERNEL_ROOT_DIR}\\${SALOME_INSTALL_PYTHON_SHARED}\;%PYTHONPATH%

  @SET PYTHONPATH=${OMNIORB_ROOT_DIR}\\lib\\x86_win32\;%PYTHONPATH%
  @SET PYTHONPATH=${OMNIORB_ROOT_DIR}\\lib\\python\;%PYTHONPATH%
  @SET PATH=${OMNIORB_ROOT_DIR}\\lib\\x86_win32\;%PATH%
  ")
  SET(EXT "bat")
  SET(CALL_STR "call")
ELSE(WIN32)
  SET(SCR  " export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/${_pydir}/site-packages/salome:\${PYTHONPATH}
  export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/bin/salome:\${PYTHONPATH}
  export LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib/salome:\${LD_LIBRARY_PATH}
  export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON}:\${PYTHONPATH}
  export PYTHONPATH=${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON_SHARED}:\${PYTHONPATH}
  export PYTHONPATH=${HDF5_ROOT_DIR}/lib:\${PYTHONPATH}
  export PYTHONPATH=${MPI_ROOT_DIR}/lib:\${PYTHONPATH}

  export PYTHONPATH=${KERNEL_ROOT_DIR}/bin/salome:\${PYTHONPATH}
  export LD_LIBRARY_PATH=${KERNEL_ROOT_DIR}/lib/salome:\${LD_LIBRARY_PATH}
  export PYTHONPATH=${KERNEL_ROOT_DIR}/${SALOME_INSTALL_PYTHON}:\${PYTHONPATH}
  export PYTHONPATH=${KERNEL_ROOT_DIR}/${SALOME_INSTALL_PYTHON_SHARED}:\${PYTHONPATH}

  export PYTHONPATH=${OMNIORB_ROOT_DIR}/lib:\${PYTHONPATH}
  export PYTHONPATH=${OMNIORB_ROOT_DIR}/lib/${_pydir}/site-packages:\${PYTHONPATH}
  export PYTHONPATH=${OMNIORB_ROOT_DIR}/lib64/${_pydir}/site-packages:\${PYTHONPATH}
  export PYTHONPATH=${OMNIORBPY_ROOT_DIR}/lib:\${PYTHONPATH}
  export PYTHONPATH=${OMNIORBPY_ROOT_DIR}/lib/${_pydir}/site-packages:\${PYTHONPATH}
  export PYTHONPATH=${OMNIORBPY_ROOT_DIR}/lib64/${_pydir}/site-packages:\${PYTHONPATH}
  export LD_LIBRARY_PATH=${OMNIORB_ROOT_DIR}/lib:\${LD_LIBRARY_PATH}
  export PYTHONPATH=${SETUPTOOLS_ROOT_DIR}/lib/${_pydir}/site-packages:\${PYTHONPATH}
  export PYTHONPATH=${DOCUTILS_ROOT_DIR}/lib/${_pydir}/site-packages:\${PYTHONPATH}
  export PYTHONPATH=${SPHINX_ROOT_DIR}/lib/${_pydir}/site-packages:\${PYTHONPATH}
"
  )
  SET(EXT "sh")
  SET(CALL_STR ".")
ENDIF(WIN32)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/env_s.${EXT} "${SCR}")

ADD_CUSTOM_TARGET(html_docs ${CALL_STR} ${CMAKE_CURRENT_BINARY_DIR}/env_s.${EXT} && ${SPHINX_EXECUTABLE} $(SPHINXBUILD) -c ${PROJECT_BINARY_DIR}/doc/docutils -b html ${ALLSPHINXOPTS} docutils
)

INSTALL(CODE "EXECUTE_PROCESS(COMMAND \"${CMAKE_COMMAND}\" --build ${PROJECT_BINARY_DIR} --target html_docs)")
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docutils DESTINATION ${SALOME_INSTALL_DOC}/tui/SMESH)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/doc/salome/tui/images/head.png ${PROJECT_SOURCE_DIR}/doc/salome/tui/images/smeshscreen.png 
        DESTINATION ${SALOME_INSTALL_DOC}/tui/SMESH)
