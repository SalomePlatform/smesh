//  SMESH DriverMED : tool to split groups on families
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : DriverMED_Family.hxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef _INCLUDE_DRIVERMED_FAMILY
#define _INCLUDE_DRIVERMED_FAMILY

#include "SMDS_Mesh.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "MEDA_Wrapper.hxx"

#include <boost/shared_ptr.hpp>
#include <set>

using namespace std;

#define REST_NODES_FAMILY 1
#define REST_EDGES_FAMILY -1
#define REST_FACES_FAMILY -2
#define REST_VOLUMES_FAMILY -3
#define FIRST_NODE_FAMILY 2
#define FIRST_ELEM_FAMILY -4

class DriverMED_Family;
typedef boost::shared_ptr<DriverMED_Family> DriverMED_FamilyPtr;

class DriverMED_Family
{
 public:

  // Methods for groups storing to MED

  static list<DriverMED_FamilyPtr> MakeFamilies (const map <int, SMESHDS_SubMesh*>& theSubMeshes,
						 const list<SMESHDS_Group*>& theGroups,
						 const bool doGroupOfNodes,
						 const bool doGroupOfEdges,
						 const bool doGroupOfFaces,
						 const bool doGroupOfVolumes);
  // Split each group from list <theGroups> and each sub-mesh from list <theSubMeshes>
  // on some parts (families) on the basis of the elements membership in other groups
  // from <theGroups> and other sub-meshes from <theSubMeshes>.
  // Resulting families have no common elements.

  MEDA::PFamilyInfo GetFamilyInfo (const MEDA::PMeshInfo& theMeshInfo) const;
  // Create TFamilyInfo for this family

  const set<const SMDS_MeshElement *>& GetElements () const { return myElements; }
  // Returns elements of this family

  int GetId () const { return myId; }
  // Returns a family ID

 public:

  // Methods for groups reading from MED

  void AddElement (const SMDS_MeshElement* theElement) { myElements.insert(theElement); }

  void AddGroupName (string theGroupName) { myGroupNames.insert(theGroupName); }

  void SetType (const SMDSAbs_ElementType theType) { myType = theType; }
  SMDSAbs_ElementType GetType () { return myType; }

  bool MemberOf (string theGroupName) const
    { return (myGroupNames.find(theGroupName) != myGroupNames.end()); }

  const MED::TStringSet& GetGroupNames () const { return myGroupNames; }

 private:
  void Init (SMESHDS_Group* group);
  // Initialize the tool by SMESHDS_Group

  static list<DriverMED_FamilyPtr> SplitByType (SMESHDS_SubMesh* theSubMesh,
						const int        theId);
  // Split <theSubMesh> on some parts (families)
  // on the basis of the elements type.

  void Split (DriverMED_FamilyPtr by,
	      DriverMED_FamilyPtr common);
  // Remove from <Elements> elements, common with <by>,
  // Remove from <by> elements, common with <Elements>,
  // Create family <common> from common elements, with combined groups list.

  void SetId (const int theId) { myId = theId; }
  // Sets a family ID

  bool IsEmpty () const { return myElements.empty(); }
  // Check, if this family has empty list of elements

 private:
  int                           myId;
  SMDSAbs_ElementType           myType;
  set<const SMDS_MeshElement *> myElements;
  MED::TStringSet               myGroupNames;
};

#endif
