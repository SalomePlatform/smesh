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
//  File   : DriverMED_W_SMESHDS_Mesh.cxx
//  Module : SMESH

#include <sstream>

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_W_SMDS_Mesh.h"
#include "DriverMED_Family.h"

#include "SMESHDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

#include "MEDA_Wrapper.hxx"
#include "MED_Utilities.hxx"

#define _EDF_NODE_IDS_
//#define _ELEMENTS_BY_DIM_

using namespace std;

DriverMED_W_SMESHDS_Mesh::DriverMED_W_SMESHDS_Mesh():
  myAllSubMeshes (false),
  myDoGroupOfNodes (false),
  myDoGroupOfEdges (false),
  myDoGroupOfFaces (false),
  myDoGroupOfVolumes (false)
{}

void DriverMED_W_SMESHDS_Mesh::SetMeshName(const std::string& theMeshName)
{
  myMeshName = theMeshName;
}

void DriverMED_W_SMESHDS_Mesh::AddGroup(SMESHDS_GroupBase* theGroup)
{
  myGroups.push_back(theGroup);
}

void DriverMED_W_SMESHDS_Mesh::AddAllSubMeshes()
{
  myAllSubMeshes = true;
}

void DriverMED_W_SMESHDS_Mesh::AddSubMesh(SMESHDS_SubMesh* theSubMesh, int theID)
{
  mySubMeshes[theID] = theSubMesh;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfNodes()
{
  myDoGroupOfNodes = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfEdges()
{
  myDoGroupOfEdges = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfFaces()
{
  myDoGroupOfFaces = true;
}

void DriverMED_W_SMESHDS_Mesh::AddGroupOfVolumes()
{
  myDoGroupOfVolumes = true;
}

typedef double (SMDS_MeshNode::* TGetCoord)() const;
typedef const char* TName;
typedef const char* TUnit;

static TUnit aUnit[3] = {"m","m","m"};

static TGetCoord aXYZGetCoord[3] = {
  &SMDS_MeshNode::X, 
  &SMDS_MeshNode::Y, 
  &SMDS_MeshNode::Z
};
static TName aXYZName[3] = {"x","y","z"};


static TGetCoord aXYGetCoord[2] = {
  &SMDS_MeshNode::X, 
  &SMDS_MeshNode::Y
};
static TName aXYName[2] = {"x","y"};

static TGetCoord aYZGetCoord[2] = {
  &SMDS_MeshNode::Y, 
  &SMDS_MeshNode::Z
};
static TName aYZName[2] = {"y","z"};

static TGetCoord aXZGetCoord[2] = {
  &SMDS_MeshNode::X, 
  &SMDS_MeshNode::Z
};
static TName aXZName[2] = {"x","z"};


static TGetCoord aXGetCoord[1] = {
  &SMDS_MeshNode::X
};
static TName aXName[1] = {"x"};

static TGetCoord aYGetCoord[1] = {
  &SMDS_MeshNode::Y
};
static TName aYName[1] = {"y"};

static TGetCoord aZGetCoord[1] = {
  &SMDS_MeshNode::Z
};
static TName aZName[1] = {"z"};


class TCoordHelper{
  SMDS_NodeIteratorPtr myNodeIter;
  const SMDS_MeshNode* myCurrentNode;
  TGetCoord* myGetCoord;
  TName* myName;
  TUnit* myUnit;
public:
  TCoordHelper(const SMDS_NodeIteratorPtr& theNodeIter,
	       TGetCoord* theGetCoord,
	       TName* theName,
	       TUnit* theUnit = aUnit):
    myNodeIter(theNodeIter),
    myGetCoord(theGetCoord),
    myName(theName),
    myUnit(theUnit)
  {}
  virtual ~TCoordHelper(){}
  bool Next(){ 
    return myNodeIter->more() && 
      (myCurrentNode = myNodeIter->next());
  }
  const SMDS_MeshNode* GetNode(){
    return myCurrentNode;
  }
  MED::TIntVector::value_type GetID(){
    return myCurrentNode->GetID();
  }
  MED::TFloatVector::value_type GetCoord(med_int theCoodId){
    return (myCurrentNode->*myGetCoord[theCoodId])();
  }
  MED::TStringVector::value_type GetName(med_int theDimId){
    return myName[theDimId];
  }
  MED::TStringVector::value_type GetUnit(med_int theDimId){
    return myUnit[theDimId];
  }
};
typedef boost::shared_ptr<TCoordHelper> TCoordHelperPtr;


Driver_Mesh::Status DriverMED_W_SMESHDS_Mesh::Perform()
{
  Status aResult = DRS_OK;
  if (myMesh->hasConstructionEdges() || myMesh->hasConstructionFaces()) {
    INFOS("SMDS_MESH with hasConstructionEdges() or hasConstructionFaces() do not supports!!!");
    return DRS_FAIL;
  }
  try{
    using namespace MEDA;
    using namespace boost;

    MESSAGE("Perform - myFile : "<<myFile);
    TWrapper aMed(myFile);

    // Creating the MED mesh for corresponding SMDS structure
    //-------------------------------------------------------
    string aMeshName;
    if (myMeshId != -1) {
      ostringstream aMeshNameStr;
      aMeshNameStr<<myMeshId;
      aMeshName = aMeshNameStr.str();
    } else {
      aMeshName = myMeshName;
    }

    // Mesh dimension definition
    med_int aMeshDimension;
    TCoordHelperPtr aCoordHelperPtr;
    {  
      bool anIsXDimension = false;
      bool anIsYDimension = false;
      bool anIsZDimension = false;
      {
	SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
	double aBounds[6];
	if(aNodesIter->more()){
	  const SMDS_MeshNode* aNode = aNodesIter->next();
	  aBounds[0] = aBounds[1] = aNode->X();
	  aBounds[2] = aBounds[3] = aNode->Y();
	  aBounds[4] = aBounds[5] = aNode->Z();
	}
	while(aNodesIter->more()){
	  const SMDS_MeshNode* aNode = aNodesIter->next();
	  aBounds[0] = min(aBounds[0],aNode->X());
	  aBounds[1] = max(aBounds[1],aNode->X());
	  
	  aBounds[2] = min(aBounds[2],aNode->Y());
	  aBounds[3] = max(aBounds[3],aNode->Y());
	  
	  aBounds[4] = min(aBounds[4],aNode->Z());
	  aBounds[5] = max(aBounds[5],aNode->Z());
	}

	double EPS = 1.0E-7;
	anIsXDimension = (aBounds[1] - aBounds[0]) + abs(aBounds[1]) + abs(aBounds[0]) > EPS;
	anIsYDimension = (aBounds[3] - aBounds[2]) + abs(aBounds[3]) + abs(aBounds[2]) > EPS;
	anIsZDimension = (aBounds[5] - aBounds[4]) + abs(aBounds[5]) + abs(aBounds[4]) > EPS;

	aMeshDimension = anIsXDimension + anIsYDimension + anIsZDimension;
	if(!aMeshDimension)
	  aMeshDimension = 3;
      }

      SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
      switch(aMeshDimension){
      case 3:
	aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXYZGetCoord,aXYZName));
	break;
      case 2:
	if(anIsXDimension && anIsYDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXYGetCoord,aXYName));
	if(anIsYDimension && anIsZDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aYZGetCoord,aYZName));
	if(anIsXDimension && anIsZDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXZGetCoord,aXZName));
	break;
      case 1:
	if(anIsXDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aXGetCoord,aXName));
	if(anIsYDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aYGetCoord,aYName));
	if(anIsZDimension)
	  aCoordHelperPtr.reset(new TCoordHelper(aNodesIter,aZGetCoord,aZName));
	break;
      }
    }

    
    PMeshInfo aMeshInfo = TWrapper::CrMeshInfo(aMeshDimension,aMeshName);
    MESSAGE("Add - aMeshName : "<<aMeshName<<"; "<<aMeshInfo->GetName());
    aMed.SetMeshInfo(aMeshInfo);

    // Storing SMDS groups and sub-meshes
    //-----------------------------------
    int myNodesDefaultFamilyId = 0;
    int myEdgesDefaultFamilyId = 0;
    int myFacesDefaultFamilyId = 0;
    int myVolumesDefaultFamilyId = 0;
    if (myDoGroupOfNodes)
      myNodesDefaultFamilyId = REST_NODES_FAMILY;
    if (myDoGroupOfEdges)
      myEdgesDefaultFamilyId = REST_EDGES_FAMILY;
    if (myDoGroupOfFaces)
      myFacesDefaultFamilyId = REST_FACES_FAMILY;
    if (myDoGroupOfVolumes)
      myVolumesDefaultFamilyId = REST_VOLUMES_FAMILY;

    MESSAGE("Perform - aFamilyInfo");
    map<const SMDS_MeshElement *, int> anElemFamMap;
    list<DriverMED_FamilyPtr> aFamilies;
    if (myAllSubMeshes) {
      aFamilies = DriverMED_Family::MakeFamilies
        (myMesh->SubMeshes(), myGroups,
         myDoGroupOfNodes, myDoGroupOfEdges, myDoGroupOfFaces, myDoGroupOfVolumes);
    } else {
      aFamilies = DriverMED_Family::MakeFamilies
        (mySubMeshes, myGroups,
         myDoGroupOfNodes, myDoGroupOfEdges, myDoGroupOfFaces, myDoGroupOfVolumes);
    }
    list<DriverMED_FamilyPtr>::iterator aFamsIter = aFamilies.begin();

    for (; aFamsIter != aFamilies.end(); aFamsIter++)
    {
      PFamilyInfo aFamilyInfo = (*aFamsIter)->GetFamilyInfo(aMeshInfo);
      aMed.SetFamilyInfo(aFamilyInfo);
      int aFamId = (*aFamsIter)->GetId();

      const set<const SMDS_MeshElement *>& anElems = (*aFamsIter)->GetElements();
      set<const SMDS_MeshElement *>::iterator anElemsIter = anElems.begin();
      for (; anElemsIter != anElems.end(); anElemsIter++)
      {
        anElemFamMap[*anElemsIter] = aFamId;
      }
//      delete (*aFamsIter);
    }

    // Storing SMDS nodes to the MED file for the MED mesh
    //----------------------------------------------------
#ifdef _EDF_NODE_IDS_
    typedef map<med_int,med_int> TNodeIdMap;
    TNodeIdMap aNodeIdMap;
#endif
    med_int aNbElems = myMesh->NbNodes();
    MED::TIntVector anElemNums(aNbElems);
    MED::TIntVector aFamilyNums(aNbElems);
    MED::TFloatVector aCoordinates(aNbElems*aMeshDimension);
    for(med_int iNode = 0, aStartId = 0; aCoordHelperPtr->Next(); iNode++, aStartId += aMeshDimension){
      for(med_int iCoord = 0; iCoord < aMeshDimension; iCoord++){
	aCoordinates[aStartId+iCoord] = aCoordHelperPtr->GetCoord(iCoord);
      }
      int aNodeID = aCoordHelperPtr->GetID();
      anElemNums[iNode] = aNodeID;
#ifdef _EDF_NODE_IDS_
      aNodeIdMap[aNodeID] = iNode+1;
#endif
      const SMDS_MeshNode* aNode = aCoordHelperPtr->GetNode();
      if (anElemFamMap.find(aNode) != anElemFamMap.end())
	aFamilyNums[iNode] = anElemFamMap[aNode];
      else
	aFamilyNums[iNode] = myNodesDefaultFamilyId;
    }

    MED::TStringVector aCoordNames(aMeshDimension);
    MED::TStringVector aCoordUnits(aMeshDimension);
    for(med_int iCoord = 0; iCoord < aMeshDimension; iCoord++){
      aCoordNames[iCoord] = aCoordHelperPtr->GetName(iCoord);
      aCoordUnits[iCoord] = aCoordHelperPtr->GetUnit(iCoord);
    }

    const med_repere SMDS_COORDINATE_SYSTEM = MED_CART;

    PNodeInfo aNodeInfo = TWrapper::CrNodeInfo(aMeshInfo,
					       SMDS_COORDINATE_SYSTEM,
					       aCoordinates,
					       aCoordNames,
					       aCoordUnits,
					       aFamilyNums,
					       anElemNums);
    MESSAGE("Perform - aNodeInfo->GetNbElem() = "<<aNbElems);
    aMed.SetNodeInfo(aNodeInfo);


    // Storing others SMDS elements to the MED file for the MED mesh
    //--------------------------------------------------------------
    med_entite_maillage SMDS_MED_ENTITY = MED_MAILLE;
    const med_connectivite SMDS_MED_CONNECTIVITY = MED_NOD;

    // Storing SMDS Edges
    if(med_int aNbElems = myMesh->NbEdges()){
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = MED_ARETE;
#endif
      SMDS_EdgeIteratorPtr anIter = myMesh->edgesIterator();
      med_int aNbConnectivity = MED::GetNbConn(SMDS_MED_ENTITY,MED_SEG2,aMeshDimension);
      MED::TIntVector anElemNums(aNbElems);
      MED::TIntVector aFamilyNums(aNbElems);
      MED::TIntVector aConnectivity(aNbElems*aNbConnectivity);

      for(med_int iElem = 0, iConn = 0; anIter->more(); iElem++, iConn+=aNbConnectivity){
	const SMDS_MeshEdge* anElem = anIter->next();
	SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	for(med_int iNode = 0; iNode < aNbConnectivity && aNodesIter->more(); iNode++){
	  const SMDS_MeshElement* aNode = aNodesIter->next();
#ifdef _EDF_NODE_IDS_
	  aConnectivity[iConn+iNode] = aNodeIdMap[aNode->GetID()];
#else
	  aConnectivity[iConn+iNode] = aNode->GetID();
#endif
	}
	anElemNums[iElem] = anElem->GetID();

        if (anElemFamMap.find(anElem) != anElemFamMap.end())
          aFamilyNums[iElem] = anElemFamMap[anElem];
        else
          aFamilyNums[iElem] = myEdgesDefaultFamilyId;
      }
      
      PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						 SMDS_MED_ENTITY,
						 MED_SEG2,
						 SMDS_MED_CONNECTIVITY,
						 aConnectivity,
						 aFamilyNums,
						 anElemNums);
      aMed.SetCellInfo(aCellInfo);
    }

    // Storing SMDS Faces
    if(med_int aNbElems = myMesh->NbFaces()){
      SMDS_FaceIteratorPtr anIter = myMesh->facesIterator();
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = MED_FACE;
#endif
      med_int aNbTriaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_TRIA3,aMeshDimension);
      MED::TIntVector anTriaElemNums; 
      anTriaElemNums.reserve(aNbElems);
      MED::TIntVector aTriaFamilyNums;
      aTriaFamilyNums.reserve(aNbElems);
      MED::TIntVector aTriaConn;
      aTriaConn.reserve(aNbElems*aNbTriaConn);

      med_int aNbQuadConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_QUAD4,aMeshDimension);
      MED::TIntVector aQuadElemNums;
      aQuadElemNums.reserve(aNbElems);
      MED::TIntVector aQuadFamilyNums;
      aQuadFamilyNums.reserve(aNbElems);
      MED::TIntVector aQuadConn;
      aQuadConn.reserve(aNbElems*aNbQuadConn);

      for(med_int iElem = 0; iElem < aNbElems && anIter->more(); iElem++){
	const SMDS_MeshFace* anElem = anIter->next();
	med_int aNbNodes = anElem->NbNodes();
	SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	med_int aNbConnectivity;
	MED::TIntVector* anElemNums;
        MED::TIntVector* aFamilyNums;
	MED::TIntVector* aConnectivity;
	switch(aNbNodes){
	case 3:
	  aNbConnectivity = aNbTriaConn;
	  anElemNums = &anTriaElemNums;
	  aFamilyNums = &aTriaFamilyNums;
	  aConnectivity = &aTriaConn;
	  break;
	case 4:
	  aNbConnectivity = aNbQuadConn;
	  anElemNums = &aQuadElemNums;
	  aFamilyNums = &aQuadFamilyNums;
	  aConnectivity = &aQuadConn;
	  break;
	}
	MED::TIntVector aVector(aNbNodes);
	for(med_int iNode = 0; aNodesIter->more(); iNode++){
	  const SMDS_MeshElement* aNode = aNodesIter->next();
	  aVector[iNode] = aNode->GetID();
	}

	med_int aSize = aConnectivity->size();
	aConnectivity->resize(aSize+aNbConnectivity);
	// There is some differnce between SMDS and MED in cells mapping
#ifdef _EDF_NODE_IDS_
	switch(aNbNodes){
	case 4:
	  (*aConnectivity)[aSize+0] = aNodeIdMap[aVector[0]];
	  (*aConnectivity)[aSize+1] = aNodeIdMap[aVector[1]];
	  (*aConnectivity)[aSize+2] = aNodeIdMap[aVector[3]];  
	  (*aConnectivity)[aSize+3] = aNodeIdMap[aVector[2]];  
	default:
	  for(med_int iNode = 0; iNode < aNbNodes; iNode++) 
	    (*aConnectivity)[aSize+iNode] = aNodeIdMap[aVector[iNode]];
	}
#else
	switch(aNbNodes){
	case 4:
	  (*aConnectivity)[aSize+0] = aVector[0];
	  (*aConnectivity)[aSize+1] = aVector[1];
	  (*aConnectivity)[aSize+2] = aVector[3];  
	  (*aConnectivity)[aSize+3] = aVector[2];  
	default:
	  for(med_int iNode = 0; iNode < aNbNodes; iNode++) 
	    (*aConnectivity)[aSize+iNode] = aVector[iNode];
	}
#endif
	anElemNums->push_back(anElem->GetID());

        if (anElemFamMap.find(anElem) != anElemFamMap.end())
          aFamilyNums->push_back(anElemFamMap[anElem]);
        else
          aFamilyNums->push_back(myFacesDefaultFamilyId);
      }
      if(med_int aNbElems = anTriaElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_TRIA3,
						   SMDS_MED_CONNECTIVITY,
						   aTriaConn,
						   aTriaFamilyNums,
						   anTriaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_TRIA3<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
      if(med_int aNbElems = aQuadElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_QUAD4,
						   SMDS_MED_CONNECTIVITY,
						   aQuadConn,
						   aQuadFamilyNums,
						   aQuadElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_QUAD4<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
    }

    // Storing SMDS Volumes
    if(med_int aNbElems = myMesh->NbVolumes()){
      SMDS_VolumeIteratorPtr anIter = myMesh->volumesIterator();
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = MED_MAILLE;
#endif
      med_int aNbTetraConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_TETRA4,aMeshDimension);
      MED::TIntVector anTetraElemNums; 
      anTetraElemNums.reserve(aNbElems);
      MED::TIntVector aTetraFamilyNums;
      aTetraFamilyNums.reserve(aNbElems);
      MED::TIntVector aTetraConn;
      aTetraConn.reserve(aNbElems*aNbTetraConn);

      med_int aNbPyraConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_PYRA5,aMeshDimension);
      MED::TIntVector anPyraElemNums; 
      anPyraElemNums.reserve(aNbElems);
      MED::TIntVector aPyraFamilyNums;
      aPyraFamilyNums.reserve(aNbElems);
      MED::TIntVector aPyraConn;
      aPyraConn.reserve(aNbElems*aNbPyraConn);

      med_int aNbPentaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_PENTA6,aMeshDimension);
      MED::TIntVector anPentaElemNums; 
      anPentaElemNums.reserve(aNbElems);
      MED::TIntVector aPentaFamilyNums;
      aPentaFamilyNums.reserve(aNbElems);
      MED::TIntVector aPentaConn;
      aPentaConn.reserve(aNbElems*aNbPentaConn);

      med_int aNbHexaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_HEXA8,aMeshDimension);
      MED::TIntVector aHexaElemNums;
      aHexaElemNums.reserve(aNbElems);
      MED::TIntVector aHexaFamilyNums;
      aHexaFamilyNums.reserve(aNbElems);
      MED::TIntVector aHexaConn;
      aHexaConn.reserve(aNbElems*aNbHexaConn);

      for(med_int iElem = 0; iElem < aNbElems && anIter->more(); iElem++){
	const SMDS_MeshVolume* anElem = anIter->next();
	med_int aNbNodes = anElem->NbNodes();
	SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	med_int aNbConnectivity;
	MED::TIntVector* anElemNums;
	MED::TIntVector* aFamilyNums;
	MED::TIntVector* aConnectivity;
	switch(aNbNodes){
	case 4:
	  aNbConnectivity = aNbTetraConn;
	  anElemNums = &anTetraElemNums;
	  aFamilyNums = &aTetraFamilyNums;
	  aConnectivity = &aTetraConn;
	  break;
	case 5:
	  aNbConnectivity = aNbPyraConn;
	  anElemNums = &anPyraElemNums;
	  aFamilyNums = &aPyraFamilyNums;
	  aConnectivity = &aPyraConn;
	  break;
	case 6:
	  aNbConnectivity = aNbPentaConn;
	  anElemNums = &anPentaElemNums;
	  aFamilyNums = &aPentaFamilyNums;
	  aConnectivity = &aPentaConn;
	  break;
	case 8:
	  aNbConnectivity = aNbHexaConn;
	  anElemNums = &aHexaElemNums;
	  aFamilyNums = &aHexaFamilyNums;
	  aConnectivity = &aHexaConn;
	}

	MED::TIntVector aVector(aNbNodes);
	for(med_int iNode = 0; aNodesIter->more(); iNode++){
	  const SMDS_MeshElement* aNode = aNodesIter->next();
	  aVector[iNode] = aNode->GetID();
	}
	med_int aSize = aConnectivity->size();
	aConnectivity->resize(aSize+aNbConnectivity);
	// There is some difference between SMDS and MED in cells mapping
#ifdef _EDF_NODE_IDS_
	switch(aNbNodes){
	case 5:
	  (*aConnectivity)[aSize+0] = aNodeIdMap[aVector[0]];
	  (*aConnectivity)[aSize+1] = aNodeIdMap[aVector[3]];
	  (*aConnectivity)[aSize+2] = aNodeIdMap[aVector[2]];  
	  (*aConnectivity)[aSize+3] = aNodeIdMap[aVector[1]];  
	  (*aConnectivity)[aSize+4] = aNodeIdMap[aVector[4]];  
	default:
	  for(med_int iNode = 0; iNode < aNbNodes; iNode++) 
	    (*aConnectivity)[aSize+iNode] = aNodeIdMap[aVector[iNode]];
	}
#else
	switch(aNbNodes){
	case 5:
	  (*aConnectivity)[aSize+0] = aVector[0];
	  (*aConnectivity)[aSize+1] = aVector[3];
	  (*aConnectivity)[aSize+2] = aVector[2];  
	  (*aConnectivity)[aSize+3] = aVector[1];  
	  (*aConnectivity)[aSize+4] = aVector[4];  
	default:
	  for(med_int iNode = 0; iNode < aNbNodes; iNode++) 
	    (*aConnectivity)[aSize+iNode] = aVector[iNode];
	}
#endif
	anElemNums->push_back(anElem->GetID());

        if (anElemFamMap.find(anElem) != anElemFamMap.end())
          aFamilyNums->push_back(anElemFamMap[anElem]);
        else
          aFamilyNums->push_back(myVolumesDefaultFamilyId);
      }

      if(med_int aNbElems = anTetraElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_TETRA4,
						   SMDS_MED_CONNECTIVITY,
						   aTetraConn,
						   aTetraFamilyNums,
						   anTetraElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_TETRA4<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
      if(med_int aNbElems = anPyraElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_PYRA5,
						   SMDS_MED_CONNECTIVITY,
						   aPyraConn,
						   aPyraFamilyNums,
						   anPyraElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_PYRA5<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
      if(med_int aNbElems = anPentaElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_PENTA6,
						   SMDS_MED_CONNECTIVITY,
						   aPentaConn,
						   aPentaFamilyNums,
						   anPentaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_PENTA6<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
      if(med_int aNbElems = aHexaElemNums.size()){
	PCellInfo aCellInfo = TWrapper::CrCellInfo(aMeshInfo,
						   SMDS_MED_ENTITY,
						   MED_HEXA8,
						   SMDS_MED_CONNECTIVITY,
						   aHexaConn,
						   aHexaFamilyNums,
						   aHexaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_HEXA8<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
    }
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }

  myMeshId = -1;
  myGroups.clear();
  mySubMeshes.clear();
  return aResult;
}
