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
//  File   : DriverMED_W_SMESHDS_Mesh.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_W_SMESHDS_MESH

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Writer.h"

#include <string>
#include <list>
#include <map>

extern "C"
{
#include <med.h>
}

using namespace std;

class SMESHDS_Group;
class SMESHDS_SubMesh;

class DriverMED_W_SMESHDS_Mesh:public Mesh_Writer
{
  public:

  DriverMED_W_SMESHDS_Mesh();
  ~DriverMED_W_SMESHDS_Mesh();

  /*! sets file name; only for usage with Add(), not Write()
   */
  void SetFile(string);
  void AddGroupOfNodes();
  void AddGroupOfEdges();
  void AddGroupOfFaces();
  void AddGroupOfVolumes();

  /*! functions to prepare adding one mesh
   */
  void SetMesh(SMDS_Mesh * aMesh);
  void SetMeshId(int);
  void SetMeshName(string theMeshName);
  void AddGroup(SMESHDS_Group* theGroup);
  void AddAllSubMeshes();
  void AddSubMesh(SMESHDS_SubMesh* theSubMesh, int theID);

  /*! add one mesh
   */
  void Add();

  /*! functions to write via DriverMED_W_SMDS_Mesh (no groups)
   */
  void SetFileId(med_idt);
  void Write();

 private:

  SMDS_Mesh * myMesh;
  string myFile;
  med_idt myFileId;
  int myMeshId;
  string myMeshName;
  list<SMESHDS_Group*> myGroups;
  bool myAllSubMeshes;
  map<int,SMESHDS_SubMesh*> mySubMeshes;
  bool myDoGroupOfNodes;
  bool myDoGroupOfEdges;
  bool myDoGroupOfFaces;
  bool myDoGroupOfVolumes;
};
#endif

