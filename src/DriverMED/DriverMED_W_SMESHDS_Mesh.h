// Copyright (C) 2007-2023  CEA, EDF, OPEN CASCADE
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

//  SMESH DriverMED : driver to read and write 'med' files
//  File   : DriverMED_W_SMESHDS_Mesh.h
//  Module : SMESH
//
#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_W_SMESHDS_MESH

#include "SMESH_DriverMED.hxx"

#include "Driver_SMESHDS_Mesh.h"
//#include "MED_Common.hxx"

#include <string>
#include <list>
#include <map>
#include <vector>

class SMESHDS_Mesh;
class SMESHDS_GroupBase;
class SMESHDS_SubMesh;
class SMDS_MeshElement;

/*!
 * \brief Write a mesh to a MED file
 */
class MESHDRIVERMED_EXPORT DriverMED_W_SMESHDS_Mesh: public Driver_SMESHDS_Mesh
{
  public:

  DriverMED_W_SMESHDS_Mesh();

  void SetFile(const std::string& theFileName, int theVersion=-1);
  void SetAutoDimension(bool toFindOutDimension) { myAutoDimension = toFindOutDimension; }
  void SetZTolerance(double tol) { myZTolerance = tol; }
  void SetSaveNumbers(bool toSave) { mySaveNumbers = toSave; }

  static std::string GetVersionString(int theMinor, int theNbDigits=2);

  void AddGroupOfNodes();
  void AddGroupOfEdges();
  void AddGroupOfFaces();
  void AddGroupOfVolumes();
  void AddGroupOf0DElems();
  void AddGroupOfBalls();

  /*! functions to prepare adding one mesh
   */
  void AddGroup(SMESHDS_GroupBase * theGroup);
  void AddAllSubMeshes();
  void AddSubMesh(SMESHDS_SubMesh* theSubMesh, int theID);
  void AddODOnVertices(bool toAdd) { myAddODOnVertices = toAdd; }

  static bool getNodesOfMissing0DOnVert(SMESHDS_Mesh*                         mesh,
                                        std::vector<const SMDS_MeshElement*>& nodes);

  /*! add one mesh
   */
  Status Perform() override;

  template<class LowLevelWriter>
  Driver_Mesh::Status PerformInternal(LowLevelWriter myMed);

 private:

  std::list<SMESHDS_GroupBase*> myGroups;
  bool                          myAllSubMeshes;
  std::vector<SMESHDS_SubMesh*> mySubMeshes;
  bool                          myDoGroupOfNodes;
  bool                          myDoGroupOfEdges;
  bool                          myDoGroupOfFaces;
  bool                          myDoGroupOfVolumes;
  bool                          myDoGroupOf0DElems;
  bool                          myDoGroupOfBalls;
  bool                          myAutoDimension;
  bool                          myAddODOnVertices;
  bool                          myDoAllInGroups;
  int                           myVersion;
  double                        myZTolerance;
  bool                          mySaveNumbers;
};

#include "MEDCouplingMemArray.hxx"

/*!
 * \brief Write a mesh to a MEDCoupling DS
 */
class MESHDRIVERMED_EXPORT DriverMED_W_SMESHDS_Mesh_Mem : public DriverMED_W_SMESHDS_Mesh
{
public:
  Status Perform() override;
  MEDCoupling::MCAuto<MEDCoupling::DataArrayByte> getData() { return _data; }
private:
  MEDCoupling::MCAuto<MEDCoupling::DataArrayByte> _data;
};

#endif
