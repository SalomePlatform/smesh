//  SMESH DriverMED : driver to read and write 'med' files
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : DriverMED_R_SMESHDS_Mesh.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERMED_R_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_R_SMESHDS_MESH

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Reader.h"
#include "DriverMED_Family.h"

#include <list>
extern "C"
{
#include <med.h>
}

using namespace std;

class SMESHDS_Group;
class SMESHDS_SubMesh;

class DriverMED_R_SMESHDS_Mesh:public Mesh_Reader
{
 public:

  DriverMED_R_SMESHDS_Mesh();
  ~DriverMED_R_SMESHDS_Mesh();

  enum ReadStatus {
    DRS_OK,
    DRS_EMPTY,          // a MED file contains no mesh with the given name
    DRS_WARN_RENUMBER,  // a MED file has overlapped ranges of element numbers,
                        // so the numbers from the file are ignored
    DRS_WARN_SKIP_ELEM, // some elements were skipped due to incorrect file data
    DRS_FAIL            // general failure (exception etc.)
  };

  void Read();
  ReadStatus ReadMySelf();
  void Add();

  list<string> GetGroupNames();
  void GetGroup(SMESHDS_Group* theGroup);
  void CreateAllSubMeshes();
  void GetSubMesh(SMESHDS_SubMesh* theSubMesh, const int theId);

  list<string> GetMeshNames();

  void SetMesh(SMDS_Mesh * aMesh);
  void SetFile(string);
  void SetFileId(med_idt);
  void SetMeshId(int);
  void SetMeshName(string theMeshName);

 private:

  SMDS_Mesh * myMesh;
  string myFile;
  med_idt myFileId;
  int myMeshId;
  string myMeshName;
  map<int, DriverMED_FamilyPtr> myFamilies;

};
#endif
