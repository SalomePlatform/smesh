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
#include "utilities.h"

#include "DriverMED_Family.h"

#include "SMESHDS_Group.hxx"

#include "MEDA_Wrapper.hxx"
#include "MED_Utilities.hxx"

#include <stdlib.h>

DriverMED_R_SMESHDS_Mesh::DriverMED_R_SMESHDS_Mesh()
     :
       myMesh (NULL),
       myFile (""),
       myFileId (-1),
       myMeshId (-1)
{
}

DriverMED_R_SMESHDS_Mesh::~DriverMED_R_SMESHDS_Mesh()
{
//  map<int, DriverMED_FamilyPtr>::iterator aFamsIter = myFamilies.begin();
//  for (; aFamsIter != myFamilies.end(); aFamsIter++)
//  {
//    delete (*aFamsIter).second;
//  }
}

void DriverMED_R_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
  myMesh = aMesh;
}

void DriverMED_R_SMESHDS_Mesh::SetFile(string aFile)
{
  myFile = aFile;
}

void DriverMED_R_SMESHDS_Mesh::SetFileId(med_idt aFileId)
{
  myFileId = aFileId;
}

void DriverMED_R_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
  myMeshId = aMeshId;
}

void DriverMED_R_SMESHDS_Mesh::SetMeshName(string theMeshName)
{
  myMeshName = theMeshName;
}

void DriverMED_R_SMESHDS_Mesh::Read()
{

  string myClass = string("SMDS_Mesh");
  string myExtension = string("MED");

  DriverMED_R_SMDS_Mesh *myReader = new DriverMED_R_SMDS_Mesh;

  myReader->SetMesh(myMesh);
  myReader->SetMeshId(myMeshId);
  myReader->SetFile(myFile);
  myReader->SetFileId(-1);

  myReader->Read();
}

void DriverMED_R_SMESHDS_Mesh::Add()
{
  string myClass = string("SMDS_Mesh");
  string myExtension = string("MED");

  DriverMED_R_SMDS_Mesh *myReader = new DriverMED_R_SMDS_Mesh;

  myReader->SetMesh(myMesh);
  myReader->SetMeshId(myMeshId);

  SCRUTE(myFileId);
  myReader->SetFileId(myFileId);

  myReader->Read();
}


static const SMDS_MeshNode* 
FindNode(const SMDS_Mesh* theMesh, med_int theId){
  const SMDS_MeshNode* aNode = theMesh->FindNode(theId);
  if(aNode) return aNode;
  EXCEPTION(runtime_error,"SMDS_Mesh::FindNode - cannot find a SMDS_MeshNode for ID = "<<theId);
}


DriverMED_R_SMESHDS_Mesh::ReadStatus DriverMED_R_SMESHDS_Mesh::ReadMySelf()
{
  ReadStatus result = DRS_FAIL;
  try{
    using namespace MEDA;

    myFamilies.clear();
    MESSAGE("ReadMySelf - myFile : "<<myFile);
    TWrapper aMed(myFile);

    result = DRS_EMPTY;
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
	MESSAGE("ReadMySelf - aMeshName : "<<aMeshName<<"; "<<aMeshInfo->GetName());
	if(aMeshName != aMeshInfo->GetName()) continue;
        result = DRS_OK;
	med_int aMeshDim = aMeshInfo->GetDim();
	
        // Reading MED families to the temporary structure
	//------------------------------------------------
        med_int aNbFams = aMed.GetNbFamilies(aMeshInfo);
        MESSAGE("Read " << aNbFams << " families");
        for (med_int iFam = 0; iFam < aNbFams; iFam++) {
          PFamilyInfo aFamilyInfo = aMed.GetFamilyInfo(aMeshInfo, iFam);
          med_int aFamId = aFamilyInfo->GetId();
          MESSAGE("Family " << aFamId << " :");

//if (aFamId >= FIRST_VALID_FAMILY) {
            DriverMED_FamilyPtr aFamily (new DriverMED_Family);

            med_int aNbGrp = aFamilyInfo->GetNbGroup();
            MESSAGE("belong to " << aNbGrp << " groups");
            for (med_int iGr = 0; iGr < aNbGrp; iGr++) {
              string aGroupName = aFamilyInfo->GetGroupName(iGr);
              MESSAGE(aGroupName);
              aFamily->AddGroupName(aGroupName);
            }
//        aFamily->SetId(aFamId);
            myFamilies[aFamId] = aFamily;
//          }
        }

        // Reading MED nodes to the corresponding SMDS structure
	//------------------------------------------------------
	PNodeInfo aNodeInfo = aMed.GetNodeInfo(aMeshInfo);
	med_booleen anIsNodeNum = aNodeInfo->IsElemNum();
	med_int aNbElems = aNodeInfo->GetNbElem();
	MESSAGE("ReadMySelf - aNodeInfo->GetNbElem() = "<<aNbElems<<"; anIsNodeNum = "<<anIsNodeNum);
        for(med_int iElem = 0; iElem < aNbElems; iElem++){
          double aCoords[3] = {0.0, 0.0, 0.0};
          for(med_int iDim = 0; iDim < aMeshDim; iDim++)
            aCoords[iDim] = aNodeInfo->GetNodeCoord(iElem,iDim);
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
	    MESSAGE("ReadMySelf - anEntity = "<<anEntity<<"; anIsElemNum = "<<anIsElemNum);
	    MESSAGE("ReadMySelf - aGeom = "<<aGeom<<"; aNbElems = "<<aNbElems);

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
	      if(anIsNodeNum) {
		for(int i = 0; i < aNbNodes; i++){
		  aNodeIds.at(i) = aNodeInfo->GetElemNum(aCellInfo->GetConn(iElem,i)-1);
		}
	      }else{
		for(int i = 0; i < aNbNodes; i++){
		  aNodeIds.at(i) = aCellInfo->GetConn(iElem,i);
		}
	      }
	      //if(anIsElemNum)
	      //	cout<<aCellInfo->GetElemNum(iElem)<<": ";
	      //else
	      //	cout<<iElem<<": ";
	      //for(int i = 0; i < aNbNodes; i++){
	      //	cout<<aNodeIds.at(i)<<", ";
	      //}

	      bool isRenum = false;
	      SMDS_MeshElement* anElement = NULL;
	      med_int aFamNum = aCellInfo->GetFamNum(iElem);
	      try{
		switch(aGeom){
		case MED_SEG2:
		case MED_SEG3:
		  if(anIsElemNum)
		    anElement = myMesh->AddEdgeWithID(aNodeIds.at(0),
						      aNodeIds.at(1),
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddEdge(FindNode(myMesh,aNodeIds.at(0)),
						FindNode(myMesh,aNodeIds.at(1)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_TRIA3:
		case MED_TRIA6:
		  aNbNodes = 3;
		  if(anIsElemNum)
		    anElement = myMesh->AddFaceWithID(aNodeIds.at(0),
						      aNodeIds.at(1),
						      aNodeIds.at(2),
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddFace(FindNode(myMesh,aNodeIds.at(0)),
						FindNode(myMesh,aNodeIds.at(1)),
						FindNode(myMesh,aNodeIds.at(2)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_QUAD4:
		case MED_QUAD8:
		  aNbNodes = 4;
		  // There is some differnce between SMDS and MED
		  if(anIsElemNum)
		    anElement = myMesh->AddFaceWithID(aNodeIds.at(0),
						      aNodeIds.at(1),
						      aNodeIds.at(2),
						      aNodeIds.at(3),
						      aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddFace(FindNode(myMesh,aNodeIds.at(0)),
						FindNode(myMesh,aNodeIds.at(1)),
						FindNode(myMesh,aNodeIds.at(2)),
						FindNode(myMesh,aNodeIds.at(3)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_TETRA4:
		case MED_TETRA10:
		  aNbNodes = 4;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds.at(0),
							aNodeIds.at(1),
							aNodeIds.at(2),
							aNodeIds.at(3),
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds.at(0)),
						  FindNode(myMesh,aNodeIds.at(1)),
						  FindNode(myMesh,aNodeIds.at(2)),
						  FindNode(myMesh,aNodeIds.at(3)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_PYRA5:
		case MED_PYRA13:
		  aNbNodes = 5;
		  // There is some differnce between SMDS and MED
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds.at(0),
							aNodeIds.at(1),
							aNodeIds.at(2),
							aNodeIds.at(3),
							aNodeIds.at(4),
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds.at(0)),
						  FindNode(myMesh,aNodeIds.at(1)),
						  FindNode(myMesh,aNodeIds.at(2)),
						  FindNode(myMesh,aNodeIds.at(3)),
						  FindNode(myMesh,aNodeIds.at(4)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_PENTA6:
		case MED_PENTA15:
		  aNbNodes = 6;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds.at(0),
							aNodeIds.at(1),
							aNodeIds.at(2),
							aNodeIds.at(3),
							aNodeIds.at(4),
							aNodeIds.at(5),
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds.at(0)),
						  FindNode(myMesh,aNodeIds.at(1)),
						  FindNode(myMesh,aNodeIds.at(2)),
						  FindNode(myMesh,aNodeIds.at(3)),
						  FindNode(myMesh,aNodeIds.at(4)),
						  FindNode(myMesh,aNodeIds.at(5)));
		    isRenum = anIsElemNum;
		  }
		  break;
		case MED_HEXA8:
		case MED_HEXA20:
		  aNbNodes = 8;
		  if(anIsElemNum)
		    anElement = myMesh->AddVolumeWithID(aNodeIds.at(0),
							aNodeIds.at(1),
							aNodeIds.at(2),
							aNodeIds.at(3),
							aNodeIds.at(4),
							aNodeIds.at(5),
							aNodeIds.at(6),
							aNodeIds.at(7),
							aCellInfo->GetElemNum(iElem));
		  if (!anElement) {
		    anElement = myMesh->AddVolume(FindNode(myMesh,aNodeIds.at(0)),
						  FindNode(myMesh,aNodeIds.at(1)),
						  FindNode(myMesh,aNodeIds.at(2)),
						  FindNode(myMesh,aNodeIds.at(3)),
						  FindNode(myMesh,aNodeIds.at(4)),
						  FindNode(myMesh,aNodeIds.at(5)),
						  FindNode(myMesh,aNodeIds.at(6)),
						  FindNode(myMesh,aNodeIds.at(7)));
		    isRenum = anIsElemNum;
		  }
		  break;
		}
	      }catch(const std::exception& exc){
		//INFOS("Follow exception was cought:\n\t"<<exc.what());
		result = DRS_FAIL;
	      }catch(...){
		//INFOS("Unknown exception was cought !!!");
		result = DRS_FAIL;
	      }
		
              if (!anElement) {
                result = DRS_WARN_SKIP_ELEM;
              }
              else {
                if (isRenum) {
                  anIsElemNum = MED_FAUX;
                  takeNumbers = false;
                  if (result < DRS_WARN_RENUMBER)
                    result = DRS_WARN_RENUMBER;
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
    result = DRS_FAIL;
  }catch(...){
    INFOS("Unknown exception was cought !!!");
    result = DRS_FAIL;
  }
  MESSAGE("ReadMySelf - result status = "<<result);
  return result;
}

list<string> DriverMED_R_SMESHDS_Mesh::GetMeshNames()
{
  list<string> aMeshNames;

  try {
    using namespace MEDA;

    MESSAGE("GetMeshNames - myFile : " << myFile);
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
  }catch(...){
    INFOS("Unknown exception was cought !!!");
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
      for (; anElemsIter != anElements.end(); anElemsIter++)
      {
        theGroup->SMDS_MeshGroup::Add(*anElemsIter);
      }
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
  SMESHDS_Mesh* aSMESHDSMesh = dynamic_cast<SMESHDS_Mesh*>(myMesh);
  if (!aSMESHDSMesh) {
    EXCEPTION(runtime_error,"Can not cast SMDS_Mesh to SMESHDS_Mesh");
  }
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
            const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>(*anElemsIter);
            aSMESHDSMesh->SetNodeInVolume(node, Id);
//            aSMESHDSMesh->SetNodeOnFace(node, Id);
//            aSMESHDSMesh->SetNodeOnEdge(node, Id);
//            aSMESHDSMesh->SetNodeOnVertex(node, Id);
          }
        }
        else
        {
          for (; anElemsIter != anElements.end(); anElemsIter++)
          {
            aSMESHDSMesh->SetMeshElementOnShape(*anElemsIter, Id);
          }
        }
      }
    }
  }
}
