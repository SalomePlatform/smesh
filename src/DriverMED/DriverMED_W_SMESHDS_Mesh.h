// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH DriverMED : driver to read and write 'med' files
//  File   : DriverMED_W_SMESHDS_Mesh.h
//  Module : SMESH
//
#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_W_SMESHDS_MESH

#include "SMESH_DriverMED.hxx"

#include "Driver_SMESHDS_Mesh.h"
#include "MED_Factory.hxx"

#include <string>
#include <list>
#include <map>

class SMESHDS_Mesh;
class SMESHDS_GroupBase;
class SMESHDS_SubMesh;

class MESHDRIVERMED_EXPORT DriverMED_W_SMESHDS_Mesh: public Driver_SMESHDS_Mesh
{
  public:

  DriverMED_W_SMESHDS_Mesh();

  virtual void SetFile(const std::string& theFileName);
  void SetFile(const std::string& theFileName, MED::EVersion theId);
  static std::string GetVersionString(const MED::EVersion theVersion, int theNbDigits=2);

  /*! sets file name; only for usage with Add(), not Write()
   */
  void AddGroupOfNodes();
  void AddGroupOfEdges();
  void AddGroupOfFaces();
  void AddGroupOfVolumes();

  /*! functions to prepare adding one mesh
   */
  void AddGroup(SMESHDS_GroupBase * theGroup);
  void AddAllSubMeshes();
  void AddSubMesh(SMESHDS_SubMesh* theSubMesh, int theID);

  /*! add one mesh
   */
  virtual Status Perform();

 private:

  MED::PWrapper myMed;
  std::list<SMESHDS_GroupBase*> myGroups;
  bool myAllSubMeshes;
  std::map<int,SMESHDS_SubMesh*> mySubMeshes;
  bool myDoGroupOfNodes;
  bool myDoGroupOfEdges;
  bool myDoGroupOfFaces;
  bool myDoGroupOfVolumes;
};


#endif

