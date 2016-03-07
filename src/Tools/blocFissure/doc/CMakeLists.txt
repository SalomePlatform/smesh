# Copyright (C) 2012-2015  EDF R&D
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

SET(RSTFILES
  index.rst
)

SET(SPHINXOPTS )
SET(SOURCEDIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(PAPEROPT_a4 -D latex_paper_size=a4)
SET(ALLSPHINXOPTS -d doctrees ${PAPEROPT_a4} ${SPHINXOPTS} ${SOURCEDIR})

SALOME_CONFIGURE_FILE(conf.py.in conf.py)

ADD_CUSTOM_TARGET(BUILD_HTML ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory html 
  COMMAND ${CMAKE_COMMAND} -E make_directory doctrees
  COMMAND ${SPHINX_EXECUTABLE} -c ${CMAKE_CURRENT_BINARY_DIR} -b html ${ALLSPHINXOPTS} html
  DEPENDS ${RSTFILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
  
ADD_CUSTOM_TARGET(BUILD_PDF ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory latex 
  COMMAND ${SPHINX_EXECUTABLE} -c ${CMAKE_CURRENT_BINARY_DIR} -b latex ${ALLSPHINXOPTS} latex
  COMMAND make -C latex all-pdf
  DEPENDS ${RSTFILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/ 
  DESTINATION ${SALOME_INSTALL_DOC}/gui/SMESH/blocFissure
  PATTERN ".buildinfo" EXCLUDE
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/blocFissure.pdf DESTINATION ${SALOME_INSTALL_DOC}/gui/SMESH/blocFissure)

#SET(_cmd_options -c ${CMAKE_CURRENT_BINARY_DIR} -b html -d doctrees -D latex_paper_size=a4 ${CMAKE_CURRENT_SOURCE_DIR} docutils)
#SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd env_script "${SPHINX_EXECUTABLE}" "${_cmd_options}")            

#ADD_CUSTOM_TARGET(html_docs_ZCracksPlug COMMAND ${_cmd})

#INSTALL(CODE "EXECUTE_PROCESS(COMMAND \"${CMAKE_COMMAND}\" --build ${PROJECT_BINARY_DIR} --target html_docs_ZCracksPlug)")
#INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docutils/ DESTINATION ${SALOME_INSTALL_DOC}/gui/SMESH/zcracks)

#SET(make_clean_files docutils doctrees)
#SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${make_clean_files}")
