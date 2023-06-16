# Copyright (C) 2012-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(UseSphinx)

SALOME_INSTALL_SCRIPTS(collect_mesh_methods.py ${SALOME_INSTALL_BINS})

#SET(DOC_SMESH_MeshersList StdMeshers)
SET(smesh_file "${CMAKE_CURRENT_SOURCE_DIR}/collect_mesh_methods.py")
SET(smesh_merge_file "${CMAKE_CURRENT_SOURCE_DIR}/merge_mesh_class.py")

# Define requared environment variables
#SALOME_ACCUMULATE_ENVIRONMENT(SMESH_MeshersList NOCHECK ${DOC_SMESH_MeshersList})
#SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_CURRENT_BINARY_DIR}/tmp1)
#SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_CURRENT_BINARY_DIR}/tmp2)

SET(ADD_VAR)
LIST(APPEND ADD_VAR SMESH_MeshersList=StdMeshers)
LIST(APPEND ADD_VAR PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/tmp1)
LIST(APPEND ADD_VAR PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/tmp2)

# Make configuration file
SALOME_CONFIGURE_FILE(conf.py.in conf.py)

IF(SALOME_SMESH_BUILD_FRENCH_DOC)
  SET(BUILD_LANGUAGE_SWITCHER "true")
  SALOME_CONFIGURE_FILE(static/switchers.js.in static/switchers.js)
  ADD_MULTI_LANG_DOCUMENTATION(TARGET_NAME usr_docs MODULE SMESH LANGUAGES fr ADDITIONAL_ENVIRONMENT ${ADD_VAR})
ELSE()
  SET(BUILD_LANGUAGE_SWITCHER "false")
  SALOME_CONFIGURE_FILE(static/switchers.js.in static/switchers.js)
  ADD_MULTI_LANG_DOCUMENTATION(TARGET_NAME usr_docs MODULE SMESH ADDITIONAL_ENVIRONMENT ${ADD_VAR})
ENDIF()

# 1. Options for generating temporary python modules
SET(_cmd_tmp_gen_options ${smesh_file} -o tmp1/smeshBuilder_dynamic.py -f sphinx StdMeshers)
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_tmp_gen env_script "${PYTHON_EXECUTABLE}" "${_cmd_tmp_gen_options}" CONTEXT "SMESH_DOC" ADDITIONAL_VARIABLES ${ADD_VAR})

# 2. Options for generating final smeshBuilder.py script
SET(_cmd_smeshBuilder_gen_options ${smesh_merge_file} -o tmp2/smeshBuilder.py smeshBuilder)
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_smeshBuilder_gen env_script "${PYTHON_EXECUTABLE}" "${_cmd_smeshBuilder_gen_options}" CONTEXT "SMESH_DOC" ADDITIONAL_VARIABLES ${ADD_VAR})

ADD_CUSTOM_TARGET(pre_usr_docs
  # 1. Make temporary directories for python modules
  COMMAND ${CMAKE_COMMAND} -E make_directory tmp1
  COMMAND ${CMAKE_COMMAND} -E make_directory tmp2

  # 2. Copy python module to the temporary file
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/smeshBuilder.py ${CMAKE_CURRENT_BINARY_DIR}/tmp1/smeshBuilder_origin.py

  # 3. Copy python modules to be documented into memporary directory
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/StdMeshersBuilder.py ${CMAKE_CURRENT_BINARY_DIR}/tmp2/StdMeshersBuilder.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/smesh_algorithm.py ${CMAKE_CURRENT_BINARY_DIR}/tmp2/smesh_algorithm.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_PY/smeshstudytools.py ${CMAKE_CURRENT_BINARY_DIR}/tmp2/smeshstudytools.py

  # 4. Collect methods from StdMeshers into temporary python module
  COMMAND ${_cmd_tmp_gen}

  # 5. Generate smeshBuilder.py script
  COMMAND ${_cmd_smeshBuilder_gen}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_DEPENDENCIES(usr_docs pre_usr_docs)

INSTALL(CODE "EXECUTE_PROCESS(COMMAND \"${CMAKE_COMMAND}\" --build ${PROJECT_BINARY_DIR} --target usr_docs)")
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SMESH DESTINATION ${SALOME_INSTALL_DOC}/gui)
IF(SALOME_SMESH_BUILD_FRENCH_DOC)
  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SMESH_fr DESTINATION ${SALOME_INSTALL_DOC}/gui)
ENDIF()

SET(make_clean_files tmp1 tmp2)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${make_clean_files}")
