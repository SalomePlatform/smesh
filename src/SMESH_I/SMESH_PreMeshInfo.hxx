// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESH_PreMeshInfo.hxx
// Created   : Fri Feb 10 13:25:02 2012
// Author    : Edward AGAPOV (eap)


#ifndef __SMESH_PreMeshInfo_HXX__
#define __SMESH_PreMeshInfo_HXX__

#include "SMDS_MeshInfo.hxx"
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SALOMEDS)

#include <TopAbs_ShapeEnum.hxx>

class DriverMED_R_SMESHDS_Mesh;
class HDFfile;
class HDFgroup;
class SMESH_Mesh_i;

/*!
 * \brief Class loading and holding information of a mesh object (mesh, group, submesh)
 *        not yet loaded from HDF file of study.
 *
 * Usage scenario:
 * - SMESH_PreMeshInfo::LoadFromFile() // reads info of all objects
 * - if ( myPreInfo ) myPreInfo->NbElements() // access to info by objects
 * - myPreInfo->FullLoadFromFile() // reads all mesh data and resets myPreInfo=NULL
 */

class SMESH_PreMeshInfo : public SMDS_MeshInfo
{
public:
  // fills SMESH_PreMeshInfo* field of all objects of mesh
  static void LoadFromFile( SMESH_Mesh_i*      mesh,
                            const int          meshID,
                            const std::string& medFile,
                            const std::string& hdfFile,
                            const bool         toRemoveFiles);

  // saves SMESH_PreMeshInfo to the study file
  static void SaveToFile( SMESH_Mesh_i* mesh,
                          const int     meshID,
                          HDFfile*      hdfFile);

  // remove all SMESH_PreMeshInfo fields from mesh and its child objects w/o data loading
  static void ForgetAllData( SMESH_Mesh_i* mesh );

  // reads all data and remove all SMESH_PreMeshInfo fields from objects
  void FullLoadFromFile() const;

  // remove all SMESH_PreMeshInfo fields from objects w/o data loading
  void ForgetAllData() const;

  // calls either FullLoadFromFile() or ForgetAllData() depending on preferences;
  // is called on hypothesis modification
  void ForgetOrLoad() const;

  // meshods of SMESH_IDSource interface
  SMESH::array_of_ElementType* GetTypes() const;
  SMESH::long_array*           GetMeshInfo() const;
  bool                         IsMeshInfoCorrect() const;

  ~SMESH_PreMeshInfo();

  // TEMPORARY method to remove study files on closing study;
  // RIGHT WAY: study files are remove automatically when meshes are destroyed
  static void RemoveStudyFiles_TMP_METHOD(SALOMEDS::SComponent_ptr smeshComp);

private:

  // creation by LoadFromFile() only
  SMESH_PreMeshInfo(SMESH_Mesh_i*      mesh,
                    const int          meshID,
                    const std::string& medFile,
                    const std::string& hdfFile);

  SMESH_PreMeshInfo* newInstance()
  { return new SMESH_PreMeshInfo( _mesh,_meshID,_medFileName,_hdfFileName ); }

  // reading from the new study, for which SaveToFile() was called
  bool readPreInfoFromHDF();
  void hdf2meshInfo( const std::string& dataSetName, HDFgroup* infoHdfGroup );

  // reading from the old study, for which SaveToFile() was not called
  bool readMeshInfo();
  void readGroupInfo();
  void readSubMeshInfo();
  SMDSAbs_EntityType getElemType( const TopAbs_ShapeEnum shapeType,
                                  const int              nbElemsInSubMesh,
                                  bool&                  isKoType) const;

  void readSubMeshes(DriverMED_R_SMESHDS_Mesh* reader) const;

  // general data
  std::string   _medFileName, _hdfFileName;
  bool          _toRemoveFiles;
  int           _meshID;
  SMESH_Mesh_i* _mesh;
  bool          _isInfoOk;

  int _elemCounter; /* used as a counter while mesh info reading and
                       as a signal that mesh info is incorrect after reading
                     */
};

#endif
