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
//  File   : DriverMED_Family.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#include "DriverMED_Family.h"
#include "MED_Factory.hxx"

#include <sstream>	

using namespace std;

//=============================================================================
/*!
 *  Split each group from list <aGroups> on some parts (families)
 *  on the basis of the elements membership in other groups from this list.
 *  Resulting families have no common elements.
 */
//=============================================================================
list<DriverMED_FamilyPtr> DriverMED_Family::MakeFamilies
                         (const map <int, SMESHDS_SubMesh*>& theSubMeshes,
                          const list<SMESHDS_GroupBase*>& theGroups,
                          const bool doGroupOfNodes,
                          const bool doGroupOfEdges,
                          const bool doGroupOfFaces,
                          const bool doGroupOfVolumes)
{
  list<DriverMED_FamilyPtr> aFamilies;

  string anAllNodesGroupName = "Group_Of_All_Nodes";
  string anAllEdgesGroupName = "Group_Of_All_Edges";
  string anAllFacesGroupName = "Group_Of_All_Faces";
  string anAllVolumesGroupName = "Group_Of_All_Volumes";

  // Reserve four ids for families of free elements
  // (1 - nodes, -1 - edges, -2 - faces, -3 - volumes).
  // 'Free' means here not belonging to any group.
  int aNodeFamId = FIRST_NODE_FAMILY;
  int aElemFamId = FIRST_ELEM_FAMILY;

  // Process sub-meshes
  map<int, SMESHDS_SubMesh*>::const_iterator aSMIter = theSubMeshes.begin();
  for (; aSMIter != theSubMeshes.end(); aSMIter++)
  {
    if ( aSMIter->second->IsComplexSubmesh() )
      continue; // submesh containing other submeshs
    list<DriverMED_FamilyPtr> aSMFams = SplitByType((*aSMIter).second, (*aSMIter).first);
    list<DriverMED_FamilyPtr>::iterator aSMFamsIter = aSMFams.begin();
    for (; aSMFamsIter != aSMFams.end(); aSMFamsIter++)
    {
      DriverMED_FamilyPtr aFam2 = (*aSMFamsIter);

      list<DriverMED_FamilyPtr>::iterator aFamsIter = aFamilies.begin();
      while (aFamsIter != aFamilies.end())
      {
        DriverMED_FamilyPtr aFam1 = *aFamsIter;
        list<DriverMED_FamilyPtr>::iterator aCurrIter = aFamsIter++;
        if (aFam1->myType == aFam2->myType)
        {
          DriverMED_FamilyPtr aCommon (new DriverMED_Family);
          aFam1->Split(aFam2, aCommon);
          if (!aCommon->IsEmpty())
          {
            aFamilies.push_back(aCommon);
          }
          if (aFam1->IsEmpty())
          {
            aFamilies.erase(aCurrIter);
          }
          if (aFam2->IsEmpty()) break;
        }
      }
      // The rest elements of family
      if (!aFam2->IsEmpty())
      {
        aFamilies.push_back(aFam2);
      }
    }
  }

  // Process groups
  list<SMESHDS_GroupBase*>::const_iterator aGroupsIter = theGroups.begin();
  for (; aGroupsIter != theGroups.end(); aGroupsIter++)
  {
    DriverMED_FamilyPtr aFam2 (new DriverMED_Family);
    aFam2->Init(*aGroupsIter);

    list<DriverMED_FamilyPtr>::iterator aFamsIter = aFamilies.begin();
    while (aFamsIter != aFamilies.end())
    {
      DriverMED_FamilyPtr aFam1 = *aFamsIter;
      list<DriverMED_FamilyPtr>::iterator aCurrIter = aFamsIter++;
      if (aFam1->myType == aFam2->myType)
      {
        DriverMED_FamilyPtr aCommon (new DriverMED_Family);
        aFam1->Split(aFam2, aCommon);
        if (!aCommon->IsEmpty())
        {
          aFamilies.push_back(aCommon);
        }
        if (aFam1->IsEmpty())
        {
          aFamilies.erase(aCurrIter);
        }
        if (aFam2->IsEmpty()) break;
      }
    }
    // The rest elements of group
    if (!aFam2->IsEmpty())
    {
      aFamilies.push_back(aFam2);
    }
  }

  list<DriverMED_FamilyPtr>::iterator aFamsIter = aFamilies.begin();
  for (; aFamsIter != aFamilies.end(); aFamsIter++)
  {
    DriverMED_FamilyPtr aFam = *aFamsIter;
    if (aFam->myType == SMDSAbs_Node) {
      aFam->SetId(aNodeFamId++);
      if (doGroupOfNodes) aFam->myGroupNames.insert(anAllNodesGroupName);
    }
    else {
      aFam->SetId(aElemFamId--);
      if (aFam->myType == SMDSAbs_Edge) {
        if (doGroupOfEdges) aFam->myGroupNames.insert(anAllEdgesGroupName);
      }
      else if (aFam->myType == SMDSAbs_Face) {
        if (doGroupOfFaces) aFam->myGroupNames.insert(anAllFacesGroupName);
      }
      else if (aFam->myType == SMDSAbs_Volume) {
        if (doGroupOfVolumes) aFam->myGroupNames.insert(anAllVolumesGroupName);
      }
    }
  }

  // Create families for elements, not belonging to any group
  if (doGroupOfNodes)
  {
    DriverMED_FamilyPtr aFreeNodesFam (new DriverMED_Family);
    aFreeNodesFam->SetId(REST_NODES_FAMILY);
    aFreeNodesFam->myType = SMDSAbs_Node;
    aFreeNodesFam->myGroupNames.insert(anAllNodesGroupName);
    aFamilies.push_back(aFreeNodesFam);
  }

  if (doGroupOfEdges)
  {
    DriverMED_FamilyPtr aFreeEdgesFam (new DriverMED_Family);
    aFreeEdgesFam->SetId(REST_EDGES_FAMILY);
    aFreeEdgesFam->myType = SMDSAbs_Edge;
    aFreeEdgesFam->myGroupNames.insert(anAllEdgesGroupName);
    aFamilies.push_back(aFreeEdgesFam);
  }

  if (doGroupOfFaces)
  {
    DriverMED_FamilyPtr aFreeFacesFam (new DriverMED_Family);
    aFreeFacesFam->SetId(REST_FACES_FAMILY);
    aFreeFacesFam->myType = SMDSAbs_Face;
    aFreeFacesFam->myGroupNames.insert(anAllFacesGroupName);
    aFamilies.push_back(aFreeFacesFam);
  }

  if (doGroupOfVolumes)
  {
    DriverMED_FamilyPtr aFreeVolumesFam (new DriverMED_Family);
    aFreeVolumesFam->SetId(REST_VOLUMES_FAMILY);
    aFreeVolumesFam->myType = SMDSAbs_Volume;
    aFreeVolumesFam->myGroupNames.insert(anAllVolumesGroupName);
    aFamilies.push_back(aFreeVolumesFam);
  }

  DriverMED_FamilyPtr aNullFam (new DriverMED_Family);
  aNullFam->SetId(0);
  aNullFam->myType = SMDSAbs_All;
  aFamilies.push_back(aNullFam);

  return aFamilies;
}

//=============================================================================
/*!
 *  Create TFamilyInfo for this family
 */
//=============================================================================
MED::PFamilyInfo 
DriverMED_Family::GetFamilyInfo(const MED::PWrapper& theWrapper, 
				const MED::PMeshInfo& theMeshInfo) const
{
  string aValue;
  ostringstream aStr;
  aStr << myId;
  aValue = aStr.str();
  MED::TStringVector anAttrDescs (1, "");  // 1 attribute with empty description,
  MED::TIntVector anAttrIds (1, myId);        // Id=0,
  MED::TIntVector anAttrVals (1, myId);       // Value=0

  MED::PFamilyInfo anInfo = theWrapper->CrFamilyInfo(theMeshInfo,
						     aValue,
						     myId,
						     myGroupNames,
						     anAttrDescs,
						     anAttrIds,
						     anAttrVals);

//  cout << endl;
//  cout << "Groups: ";
//  set<string>::iterator aGrIter = myGroupNames.begin();
//  for (; aGrIter != myGroupNames.end(); aGrIter++)
//  {
//    cout << " " << *aGrIter;
//  }
//  cout << endl;
//
//  cout << "Elements: ";
//  set<const SMDS_MeshElement *>::iterator anIter = myElements.begin();
//  for (; anIter != myElements.end(); anIter++)
//  {
//    cout << " " << (*anIter)->GetID();
//  }
//  cout << endl;

  return anInfo;
}

//=============================================================================
/*!
 *  Initialize the tool by SMESHDS_GroupBase
 */
//=============================================================================
void DriverMED_Family::Init (SMESHDS_GroupBase* theGroup)
{
  // Elements
  myElements.clear();
  SMDS_ElemIteratorPtr elemIt = theGroup->GetElements();
  while (elemIt->more())
  {
    myElements.insert(elemIt->next());
  }

  // Type
  myType = theGroup->GetType();

  // Groups list
  myGroupNames.clear();
  myGroupNames.insert(string(theGroup->GetStoreName()));
}

//=============================================================================
/*!
 *  Split <theSubMesh> on some parts (families)
 *  on the basis of the elements type.
 */
//=============================================================================
list<DriverMED_FamilyPtr> DriverMED_Family::SplitByType (SMESHDS_SubMesh* theSubMesh,
                                                         const int        theId)
{
  list<DriverMED_FamilyPtr> aFamilies;
  DriverMED_FamilyPtr aNodesFamily   (new DriverMED_Family);
  DriverMED_FamilyPtr anEdgesFamily  (new DriverMED_Family);
  DriverMED_FamilyPtr aFacesFamily   (new DriverMED_Family);
  DriverMED_FamilyPtr aVolumesFamily (new DriverMED_Family);

  char submeshGrpName[ 30 ];
  sprintf( submeshGrpName, "SubMesh %d", theId );

  SMDS_NodeIteratorPtr aNodesIter = theSubMesh->GetNodes();
  while (aNodesIter->more())
  {
    const SMDS_MeshNode* aNode = aNodesIter->next();
    aNodesFamily->AddElement(aNode);
  }

  SMDS_ElemIteratorPtr anElemsIter = theSubMesh->GetElements();
  while (anElemsIter->more())
  {
    const SMDS_MeshElement* anElem = anElemsIter->next();
    switch (anElem->GetType())
    {
    case SMDSAbs_Edge:
      anEdgesFamily->AddElement(anElem);
      break;
    case SMDSAbs_Face:
      aFacesFamily->AddElement(anElem);
      break;
    case SMDSAbs_Volume:
      aVolumesFamily->AddElement(anElem);
      break;
    default:
      break;
    }
  }

  if (!aNodesFamily->IsEmpty()) {
    aNodesFamily->SetType(SMDSAbs_Node);
    aNodesFamily->AddGroupName(submeshGrpName);
    aFamilies.push_back(aNodesFamily);
  }
  if (!anEdgesFamily->IsEmpty()) {
    anEdgesFamily->SetType(SMDSAbs_Edge);
    anEdgesFamily->AddGroupName(submeshGrpName);
    aFamilies.push_back(anEdgesFamily);
  }
  if (!aFacesFamily->IsEmpty()) {
    aFacesFamily->SetType(SMDSAbs_Face);
    aFacesFamily->AddGroupName(submeshGrpName);
    aFamilies.push_back(aFacesFamily);
  }
  if (!aVolumesFamily->IsEmpty()) {
    aVolumesFamily->SetType(SMDSAbs_Volume);
    aVolumesFamily->AddGroupName(submeshGrpName);
    aFamilies.push_back(aVolumesFamily);
  }

  return aFamilies;
}

//=============================================================================
/*!
 *  Remove from <myElements> elements, common with <by>,
 *  Remove from <by> elements, common with <myElements>,
 *  Create family <common> from common elements, with combined groups list.
 */
//=============================================================================
void DriverMED_Family::Split (DriverMED_FamilyPtr by,
                              DriverMED_FamilyPtr common)
{
  // Elements
  set<const SMDS_MeshElement *>::iterator anIter = by->myElements.begin();
  for (; anIter != by->myElements.end(); anIter++)
  {
    if (myElements.find(*anIter) != myElements.end())
    {
      common->myElements.insert(*anIter);
      myElements.erase(*anIter);
      by->myElements.erase(*anIter);
    }
  }

  if (!common->IsEmpty())
  {
    // Groups list
    common->myGroupNames = myGroupNames;
    set<string>::iterator aGrNamesIter = by->myGroupNames.begin();
    for (; aGrNamesIter != by->myGroupNames.end(); aGrNamesIter++)
    {
      common->myGroupNames.insert(*aGrNamesIter);
    }

    // Type
    common->myType = myType;
  }
}
