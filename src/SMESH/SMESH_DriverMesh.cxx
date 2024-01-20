// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  File   : SMESH_DriverMesh.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "utilities.h"

#include "SMESH_DriverMesh.hxx"

#include "SMESH_Mesh.hxx"
#include "SMESH_Gen.hxx"

#include <MEDFileMesh.hxx>
#include <MEDCouplingUMesh.hxx>

using namespace MEDCoupling;

/**
 * @brief Compares the mesh from two mesh files (MED)
 *
 * @param mesh_file1 First file
 * @param mesh_file2 Second file
 * @param mesh_name Name of the mesh in the files
 *
 * @return true if the mesh within the files are identical
 */
bool SMESH_DriverMesh::diffMEDFile(const std::string mesh_file1, const std::string mesh_file2, const std::string mesh_name){
  MEDFileUMesh* medmesh1 = MEDFileUMesh::New(mesh_file1, mesh_name);
  MEDFileUMesh* medmesh2 = MEDFileUMesh::New(mesh_file2, mesh_name);
  MEDCouplingUMesh *m0_1=medmesh1->getMeshAtLevel(0,false);
  MEDCouplingUMesh *m0_2=medmesh2->getMeshAtLevel(0,false);
  return m0_1->isEqual(m0_2, 1e-12);
}

std::string getMeshName(std::string mesh_file){
  // TODO: Memory leak but desctructor private check with AG
  MEDFileUMesh * myMedMesh = MEDFileUMesh::New(mesh_file);

  return myMedMesh->getLevel0Mesh()->getName();
}

/**
 * @brief Import a mesh from a mesh file (MED) into a SMESH_Mesh object
 *
 * @param mesh_file the file
 * @param aMesh the object
 * @param mesh_name the name of the mesh in the file
 *
 * @return error code
 */
int SMESH_DriverMesh::importMesh(const std::string mesh_file, SMESH_Mesh& aMesh){
  // TODO: change that as it depends on the language
  std::string mesh_name = getMeshName(mesh_file);
  MESSAGE("Importing mesh from " << mesh_file << " mesh " << mesh_name);
  int ret = aMesh.MEDToMesh(mesh_file.c_str(), mesh_name.c_str());
  return ret;
}

/**
 * @brief Export the content of a SMESH_Mesh into a mesh file (MED)
 *
 * @param mesh_file the file
 * @param aMesh the object
 * @param mesh_name name of the mesh in the file
 *
 * @return error code
 */
 
int SMESH_DriverMesh::exportMesh(const std::string mesh_file, SMESH_Mesh& aMesh, const std::string mesh_name){

  MESSAGE("Exporting mesh to " << mesh_file);
  aMesh.ExportMED(mesh_file.c_str(), // theFile
                  mesh_name.c_str(), // theMeshName
                  false, // theAutoGroups
                  -1, // theVersion
                  nullptr, // theMeshPart
                  true, // theAutoDimension
                  true, // theAddODOnVertices
                  1e-8, // theZTolerance
                  true // theSaveNumbers
                  );
  return true;
}
