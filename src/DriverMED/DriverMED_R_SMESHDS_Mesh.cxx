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
//  File   : DriverMED_R_SMESHDS_Mesh.cxx
//  Module : SMESH

#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_R_SMDS_Mesh.h"
#include "SMESHDS_Mesh.hxx"
#include "utilities.h"

#include "DriverMED_Family.h"

#include "SMESHDS_Group.hxx"

#include "MEDA_Wrapper.hxx"
#include "MED_Utilities.hxx"

#include <stdlib.h>

#define _EDF_NODE_IDS_

void DriverMED_R_SMESHDS_Mesh::SetMeshName(string theMeshName)
{
  myMeshName = theMeshName;
}

static const SMDS_MeshNode* 
FindNode(const SMDS_Mesh* theMesh, med_int theId){
  const SMDS_MeshNode* aNode = theMesh->FindNode(theId);
  if(aNode) return aNode;
  EXCEPTION(runtime_error,"SMDS_Mesh::FindNode - cannot find a SMDS_MeshNode for ID = "<<theId);
}


enum ECoordName{eX, eY, eZ, eNone};
typedef med_float (*TGetCoord)(MEDA::PNodeInfo&, med_int);

template<ECoordName TheCoordId>
med_float GetCoord(MEDA::PNodeInfo& thePNodeInfo, med_int theElemId){
  return thePNodeInfo->GetNodeCoord(theElemId,TheCoordId);
}

template<>
med_float GetCoord<eNone>(MEDA::PNodeInfo& thePNodeInfo, med_int theElemId){
  return 0.0;
}


static TGetCoord aXYZGetCoord[3] = {
  &GetCoord<eX>, 
  &GetCoord<eY>, 
  &GetCoord<eZ>
};


static TGetCoord aXYGetCoord[3] = {
  &GetCoord<eX>, 
  &GetCoord<eY>, 
  &GetCoord<eNone>
};

static TGetCoord aYZGetCoord[3] = {
  &GetCoord<eNone>,
  &GetCoord<eX>, 
  &GetCoord<eY>
};

static TGetCoord aXZGetCoord[3] = {
  &GetCoord<eX>, 
  &GetCoord<eNone>,
  &GetCoord<eY>
};


static TGetCoord aXGetCoord[3] = {
  &GetCoord<eX>, 
  &GetCoord<eNone>,
  &GetCoord<eNone>
};

static TGetCoord aYGetCoord[3] = {
  &GetCoord<eNone>,
  &GetCoord<eX>, 
  &GetCoord<eNone>
};

static TGetCoord aZGetCoord[3] = {
  &GetCoord<eNone>,
  &GetCoord<eNone>,
  &GetCoord<eX>
};


class TCoordHelper{
  MEDA::PNodeInfo myPNodeInfo;
  TGetCoord* myGetCoord;
public:
  TCoordHelper(const MEDA::PNodeInfo& thePNodeInfo,
	       TGetCoord* theGetCoord):
    myPNodeInfo(thePNodeInfo),
    myGetCoord(theGetCoord)
  {}
  virtual ~TCoordHelper(){}
  med_float GetCoord(med_int theElemId, med_int theCoodId){
    return (*myGetCoord[theCoodId])(myPNodeInfo,theElemId);
  }
};
typedef boost::shared_ptr<TCoordHelper> TCoordHelperPtr;


Driver_Mesh::Status DriverMED_R_SMESHDS_Mesh::Perform()
{
  Status aResult = DRS_FAIL;
  try{
    using namespace MEDA;

    myFamilies.clear();
    MESSAGE("Perform - myFile : "<<myFile);
    TWrapper aMed(myFile);

    aResult = DRS_EMPTY;
    if(med_int aNbMeshes = aMed.GetNbMeshes()){
      for(int iMesh = 0; iMesh < aNbMeshes; iMesh++){
	// Reading the MED mesh
	//---------------------
	PMeshInfo aMeshInfo = aMed.GetMeshInfo(iMesh);
        string aMeshName;
        if (myMeshId != -1) {
          ostringstream aMeshNameStr;
          aMeshNameStr<<myMeshId;
          aMeshName = aMeshNameStr.str();
        } else {
          aMeshName = myMeshName;
        }
	MESSAGE("Perform - aMeshName : "<<aMeshName<<"; "<<aMeshInfo->GetName());
	if(aMeshName != aMeshInfo->GetName()) continue;
        aResult = DRS_OK;
	med_int aMeshDim = aMeshInfo->GetDim();
	
        // Reading MED families to the temporary structure
	//------------------------------------------------
        med_int aNbFams = aMed.GetNbFamilies(aMeshInfo);
        MESSAGE("Read " << aNbFams << " families");
        for (med_int iFam = 0; iFam < aNbFams; iFam++) {
          PFamilyInfo aFamilyInfo = aMed.GetFamilyInfo(aMeshInfo, iFam);
          med_int aFamId = aFamilyInfo->GetId();
          MESSAGE("Family " << aFamId << " :");

            DriverMED_FamilyPtr aFamily (new DriverMED_Family);

            med_int aNbGrp = aFamilyInfo->GetNbGroup();
            MESSAGE("belong to " << aNbGrp << " groups");
            for (med_int iGr = 0; iGr < aNbGrp; iGr++) {
              string aGroupName = aFamilyInfo->GetGroupName(iGr);
              MESSAGE(aGroupName);
              aFamily->AddGroupName(aGroupName);
            }
            myFamilies[aFamId] = aFamily;
        }

        // Reading MED nodes to the corresponding SMDS structure
	//------------------------------------------------------
	PNodeInfo aNodeInfo = aMed.GetNodeInfo(aMeshInfo);

	TCoordHelperPtr aCoordHelperPtr;
	{
	  med_int aMeshDimension = aMeshInfo->GetDim();
	  bool anIsDimPresent[3] = {false, false, false};
          for(med_int iDim = 0; iDim < aMeshDimension; iDim++){
	    string aDimName = aNodeInfo->GetCoordName(iDim);
	    if(aDimName == "x" || aDimName == "X")
	      anIsDimPresent[eX] = true;
	    else if(aDimName == "y" || aDimName == "Y")
	      anIsDimPresent[eY] = true;
	    else if(aDimName == "z" || aDimName == "Z")
	      anIsDimPresent[eZ] = true;
	  }
	  switch(aMeshDimension){
	  case 3:
	    aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aXYZGetCoord));
	    break;
	  case 2:
	    if(anIsDimPresent[eY] && anIsDimPresent[eZ])
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aYZGetCoord));
	    else if(anIsDimPresent[eX] && anIsDimPresent[eZ])
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aXZGetCoord));
	    else
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aXYGetCoord));
	    break;
	  case 1:
	    if(anIsDimPresent[eY])
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aYGetCoord));
	    else if(anIsDimPresent[eZ])
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aZGetCoord));
	    else
	      aCoordHelperPtr.reset(new TCoordHelper(aNodeInfo,aXGetCoord));
	    break;
	  }
	}

	med_booleen anIsNodeNum = aNodeInfo->IsElemNum();
	med_int aNbElems = aNodeInfo->GetNbElem();
	MESSAGE("Perform - aNodeInfo->GetNbElem() = "<<aNbElems<<"; anIsNodeNum = "<<anIsNodeNum);
        for(med_int iElem = 0; iElem < aNbElems; iElem++){
          double aCoords[3] = {0.0, 0.0, 0.0};
          for(med_int iDim = 0; iDim < 3; iDim++)
            aCoords[iDim] = aCoordHelperPtr->GetCoord(iElem,iDim);
          const SMDS_MeshNode* aNode;
          if(anIsNodeNum) {
	    aNode = myMesh->AddNodeWithID
              (aCoords[0],aCoords[1],aCoords[2],aNodeInfo->GetElemNum(iElem));
          } else {
	    aNode = myMesh->AddNode
              (aCoords[0],aCoords[1],aCoords[2]);
          }
          //cout<<aNode->GetID()<<": "<<aNode->X()<<", "<<aNode->Y()<<", "<<aNode->Z()<<endl;

          // Save reference to this node from its family
          med_int aFamNum = aNodeInfo->GetFamNum(iElem);
          if (myFamilies.find(aFamNum) != myFamilies.end())
          {
            myFamilies[aFamNum]->AddElement(aNode);
            myFamilies[aFamNum]->SetType(SMDSAbs_Node);
          }
        }

	// Reading pre information about all MED cells
	//--------------------------------------------
        bool takeNumbers = true;  // initially we trust the numbers from file
	MED::TEntityInfo aEntityInfo = aMed.GetEntityInfo(aMeshInfo);
	MED::TEntityInfo::iterator anEntityIter = aEntityInfo.begin();
	for(; anEntityIter != aEntityInfo.end(); anEntityIter++){
	  const med_entite_maillage& anEntity = anEntityIter->first;
	  if(anEntity == MED_NOEUD) continue;
	  // Reading MED cells to the corresponding SMDS structure
	  //------------------------------------------------------
	  const MED::TGeom& aTGeom = anEntityIter->second;
	  MED::TGeom::const_iterator anTGeomIter = aTGeom.begin();
	  for(; anTGeomIter != aTGeom.end(); anTGeomIter++){
	    const med_geometrie_element& aGeom = anTGeomIter->first;
	    if(aGeom == MED_POINT1) continue;
	    PCellInfo aCellInfo = aMed.GetCellInfo(aMeshInfo,anEntity,aGeom);
	    med_booleen anIsElemNum = takeNumbers ? aCellInfo->IsElemNum() : MED_FAUX;
	    med_int aNbElems = aCellInfo->GetNbElem();
	    MESSAGE("Perform - anEntity = "<<anEntity<<"; anIsElemNum = "<<anIsElemNum);
	    MESSAGE("Perform - aGeom = "<<aGeom<<"; aNbElems = "<<aNbElems);

	    for(int iElem = 0; iElem < aNbElems; iElem++){
	      med_int aNbNodes = -1;
	      switch(aGeom){
	      case MED_SEG2:
	      case MED_SEG3:
		aNbNodes = 2;
		break;
	      case MED_TRIA3:
	      case MED_TRIA6:
		aNbNodes = 3;
		break;
		break;
	      case MED_QUAD4:
	      case MED_QUAD8:
		aNbNodes = 4;
                break;
              case MED_TETRA4:
	      case MED_TETRA10:
		aNbNodes = 4;
		break;
	      case MED_PYRA5:
	      case MED_PYRA13:
		aNbNodes = 5;
		break;
	      case MED_PENTA6:
	      case MED_PENTA15:
		aNbNodes = 6;
		break;
	      case MED_HEXA8:
	      case MED_HEXA20:
		aNbNodes = 8;
		break;
	      }
	      vector<med_int> aNodeIds(aNbNodes);
#ifdef _EDF_NODE_IDS_
	      if(anIsNodeNum) {
		for(int i = 0; i < aNbNodes; i++){
		  aNodeIds[i] = aNodeInfo->GetElemNum(aCellInfo->GetConn(iElem,i)-1);
		}
	      }else{
		for(int i = 0; i < aNbNodes; i++){
		  aNodeIds[i] = aCellInfo->GetConn(iElem,i);
		}
	      }
#else
	      for(int i = 0; i < aNbNodes; i++){
		aNodeIds[i] = aCellInfo->GetConn(iElem,i);
	      }
#endif

	      bool isRenum = false;
	      SMDS_MeshElement* anElement = NULL;
	      med_int aFamNum = aCellInfo->GetFamNum(iElem);
	      try{
		switch(aGeom){
		case MED_SEG2:
		case MED_SEG3:
		  if(anIsElemNum)
		    anElement = myMesh->AddEdgeWithID(aNodeIds[0],
						      aNodeIds[1],
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddEdge(FindNode(myMesh,aNodeIds[0]),
						FindNode(myMesh,aNodeIds[1]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_TRIA3:
		case MED_TRIA6:
		  aNbNodes = 3;
		  if(anIsElemNum)
		    anElement = myMesh->AddFaceWithID(aNodeIds[0],
						      aNodeIds[1],
						      aNodeIds[2],
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddFace(FindNode(myMesh,aNodeIds[0]),
						FindNode(myMesh,aNodeIds[1]),
						FindNode(myMesh,aNodeIds[2]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_QUAD4:
		case MED_QUAD8:
		  aNbNodes = 4;
		  // There is some differnce between SMDS and MED
		  if(anIsElemNum)
		    anElement = myMesh->AddFaceWithID(aNodeIds[0],
						      aNodeIds[1],
						      aNodeIds[2],
						      aNodeIds[3],
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddFace(FindNode(myMesh,aNodeIds[0]),
						FindNode(myMesh,aNodeIds[1]),
						FindNode(myMesh,aNodeIds[2]),
						FindNode(myMesh,aNodeIds[3]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_TETRA4:
		case MED_TETRA10:
		  aNbNodes = 4;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds[0],
							aNodeIds[1],
							aNodeIds[2],
							aNodeIds[3],
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds[0]),
						  FindNode(myMesh,aNodeIds[1]),
						  FindNode(myMesh,aNodeIds[2]),
						  FindNode(myMesh,aNodeIds[3]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_PYRA5:
		case MED_PYRA13:
		  aNbNodes = 5;
		  // There is some differnce between SMDS and MED
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds[0],
							aNodeIds[1],
							aNodeIds[2],
							aNodeIds[3],
							aNodeIds[4],
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds[0]),
						  FindNode(myMesh,aNodeIds[1]),
						  FindNode(myMesh,aNodeIds[2]),
						  FindNode(myMesh,aNodeIds[3]),
						  FindNode(myMesh,aNodeIds[4]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_PENTA6:
		case MED_PENTA15:
		  aNbNodes = 6;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds[0],
							aNodeIds[1],
							aNodeIds[2],
							aNodeIds[3],
							aNodeIds[4],
							aNodeIds[5],
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds[0]),
						  FindNode(myMesh,aNodeIds[1]),
						  FindNode(myMesh,aNodeIds[2]),
						  FindNode(myMesh,aNodeIds[3]),
						  FindNode(myMesh,aNodeIds[4]),
						  FindNode(myMesh,aNodeIds[5]));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_HEXA8:
		case MED_HEXA20:
		  aNbNodes = 8;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds[0],
							aNodeIds[1],
							aNodeIds[2],
							aNodeIds[3],
							aNodeIds[4],
							aNodeIds[5],
							aNodeIds[6],
							aNodeIds[7],
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds[0]),
						  FindNode(myMesh,aNodeIds[1]),
						  FindNode(myMesh,aNodeIds[2]),
						  FindNode(myMesh,aNodeIds[3]),
						  FindNode(myMesh,aNodeIds[4]),
						  FindNode(myMesh,aNodeIds[5]),
						  FindNode(myMesh,aNodeIds[6]),
						  FindNode(myMesh,aNodeIds[7]));
		    isRenum = anIsElemNum;
		  }
		  break;
		}
	      }catch(const std::exception& exc){
		//INFOS("Follow exception was cought:\n\t"<<exc.what());
		aResult = DRS_FAIL;
	      }catch(...){
		//INFOS("Unknown exception was cought !!!");
		aResult = DRS_FAIL;
	      }
		
              if (!anElement) {
                aResult = DRS_WARN_SKIP_ELEM;
              }
              else {
                if (isRenum) {
                  anIsElemNum = MED_FAUX;
                  takeNumbers = false;
                  if (aResult < DRS_WARN_RENUMBER)
                    aResult = DRS_WARN_RENUMBER;
                }
                if (myFamilies.find(aFamNum) != myFamilies.end()) {
                  // Save reference to this element from its family
                  myFamilies[aFamNum]->AddElement(anElement);
                  myFamilies[aFamNum]->SetType(anElement->GetType());
                }
              }
            }
	  }
	}
	break;
      }
    }
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
    aResult = DRS_FAIL;
  }catch(...){
    INFOS("Unknown exception was cought !!!");
    aResult = DRS_FAIL;
  }
  MESSAGE("Perform - aResult status = "<<aResult);
  return aResult;
}

list<string> DriverMED_R_SMESHDS_Mesh::GetMeshNames(Status& theStatus)
{
  list<string> aMeshNames;

  try {
    using namespace MEDA;

    MESSAGE("GetMeshNames - myFile : " << myFile);
    theStatus = DRS_OK;
    TWrapper aMed (myFile);

    if (med_int aNbMeshes = aMed.GetNbMeshes()) {
      for (int iMesh = 0; iMesh < aNbMeshes; iMesh++) {
	// Reading the MED mesh
	//---------------------
	PMeshInfo aMeshInfo = aMed.GetMeshInfo(iMesh);
	aMeshNames.push_back(aMeshInfo->GetName());
      }
    }
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
    theStatus = DRS_FAIL;
  }catch(...){
    INFOS("Unknown exception was cought !!!");
    theStatus = DRS_FAIL;
  }

  return aMeshNames;
}

list<string> DriverMED_R_SMESHDS_Mesh::GetGroupNames()
{
  list<string> aResult;
  set<string> aResGroupNames;

  map<int, DriverMED_FamilyPtr>::iterator aFamsIter = myFamilies.begin();
  for (; aFamsIter != myFamilies.end(); aFamsIter++)
  {
    DriverMED_FamilyPtr aFamily = (*aFamsIter).second;
    const MED::TStringSet& aGroupNames = aFamily->GetGroupNames();
    set<string>::iterator aGrNamesIter = aGroupNames.begin();
    for (; aGrNamesIter != aGroupNames.end(); aGrNamesIter++)
    {
      string aName = *aGrNamesIter;
      // Check, if this is a Group or SubMesh name
//if (aName.substr(0, 5) == string("Group")) {
        if (aResGroupNames.find(aName) == aResGroupNames.end()) {
          aResGroupNames.insert(aName);
          aResult.push_back(aName);
        }
//    }
    }
  }

  return aResult;
}

void DriverMED_R_SMESHDS_Mesh::GetGroup(SMESHDS_Group* theGroup)
{
  string aGroupName (theGroup->GetStoreName());
  MESSAGE("Get Group " << aGroupName);

  map<int, DriverMED_FamilyPtr>::iterator aFamsIter = myFamilies.begin();
  for (; aFamsIter != myFamilies.end(); aFamsIter++)
  {
    DriverMED_FamilyPtr aFamily = (*aFamsIter).second;
    if (aFamily->MemberOf(aGroupName))
    {
      const set<const SMDS_MeshElement *>& anElements = aFamily->GetElements();
      set<const SMDS_MeshElement *>::iterator anElemsIter = anElements.begin();
      const SMDS_MeshElement * element = 0;
      for (; anElemsIter != anElements.end(); anElemsIter++)
      {
        element = *anElemsIter;
        theGroup->SMDSGroup().Add(element);
      }
      if ( element )
        theGroup->SetType( element->GetType() );
    }
  }
}

void DriverMED_R_SMESHDS_Mesh::GetSubMesh (SMESHDS_SubMesh* theSubMesh,
                                           const int theId)
{
  char submeshGrpName[ 30 ];
  sprintf( submeshGrpName, "SubMesh %d", theId );
  string aName (submeshGrpName);
  map<int, DriverMED_FamilyPtr>::iterator aFamsIter = myFamilies.begin();
  for (; aFamsIter != myFamilies.end(); aFamsIter++)
  {
    DriverMED_FamilyPtr aFamily = (*aFamsIter).second;
    if (aFamily->MemberOf(aName))
    {
      const set<const SMDS_MeshElement *>& anElements = aFamily->GetElements();
      set<const SMDS_MeshElement *>::iterator anElemsIter = anElements.begin();
      if (aFamily->GetType() == SMDSAbs_Node)
      {
        for (; anElemsIter != anElements.end(); anElemsIter++)
        {
          const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>(*anElemsIter);
          theSubMesh->AddNode(node);
        }
      }
      else
      {
        for (; anElemsIter != anElements.end(); anElemsIter++)
        {
          theSubMesh->AddElement(*anElemsIter);
        }
      }
    }
  }
}

void DriverMED_R_SMESHDS_Mesh::CreateAllSubMeshes ()
{
  map<int, DriverMED_FamilyPtr>::iterator aFamsIter = myFamilies.begin();
  for (; aFamsIter != myFamilies.end(); aFamsIter++)
  {
    DriverMED_FamilyPtr aFamily = (*aFamsIter).second;
    MED::TStringSet aGroupNames = aFamily->GetGroupNames();
    set<string>::iterator aGrNamesIter = aGroupNames.begin();
    for (; aGrNamesIter != aGroupNames.end(); aGrNamesIter++)
    {
      string aName = *aGrNamesIter;
      // Check, if this is a Group or SubMesh name
      if (aName.substr(0, 7) == string("SubMesh"))
      {
        int Id = atoi(string(aName).substr(7).c_str());
        set<const SMDS_MeshElement *> anElements = aFamily->GetElements();
        set<const SMDS_MeshElement *>::iterator anElemsIter = anElements.begin();
        if (aFamily->GetType() == SMDSAbs_Node)
        {
          for (; anElemsIter != anElements.end(); anElemsIter++)
          {
            SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>
              ( static_cast<const SMDS_MeshNode*>( *anElemsIter ));
            // find out a shape type
            TopoDS_Shape aShape = myMesh->IndexToShape( Id );
            int aShapeType = ( aShape.IsNull() ? -1 : aShape.ShapeType() );
            switch ( aShapeType ) {
            case TopAbs_FACE:
              myMesh->SetNodeOnFace(node, Id); break;
            case TopAbs_EDGE:
              myMesh->SetNodeOnEdge(node, Id); break;
            case TopAbs_VERTEX:
              myMesh->SetNodeOnVertex(node, Id); break;
            default:
              myMesh->SetNodeInVolume(node, Id);
            }
          }
        }
        else
        {
          for (; anElemsIter != anElements.end(); anElemsIter++)
          {
            myMesh->SetMeshElementOnShape(*anElemsIter, Id);
          }
        }
      }
    }
  }
}
