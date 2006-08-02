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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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
#include "SMDS_PolyhedralVolumeOfNodes.hxx"

#include "utilities.h"

#include "MED_Utilities.hxx"

#define _EDF_NODE_IDS_
//#define _ELEMENTS_BY_DIM_

using namespace std;
using namespace MED;


DriverMED_W_SMESHDS_Mesh::DriverMED_W_SMESHDS_Mesh():
  myAllSubMeshes (false),
  myDoGroupOfNodes (false),
  myDoGroupOfEdges (false),
  myDoGroupOfFaces (false),
  myDoGroupOfVolumes (false)
{}

void DriverMED_W_SMESHDS_Mesh::SetFile(const std::string& theFileName, 
				       MED::EVersion theId)
{
  myMed = CrWrapper(theFileName,theId);
  Driver_SMESHDS_Mesh::SetFile(theFileName);
}

void DriverMED_W_SMESHDS_Mesh::SetFile(const std::string& theFileName)
{
  return SetFile(theFileName,MED::eV2_2);
}

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

namespace{
  typedef double (SMDS_MeshNode::* TGetCoord)() const;
  typedef const char* TName;
  typedef const char* TUnit;

  // name length in a mesh must be equal to 16 :
  //         1234567890123456
  TName M = "m               ";
  TName X = "x               ";
  TName Y = "y               ";
  TName Z = "z               ";

  TUnit aUnit[3] = {M,M,M};

  // 3 dim
  TGetCoord aXYZGetCoord[3] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Y, 
    &SMDS_MeshNode::Z
  };
  TName aXYZName[3] = {X,Y,Z};
  
  // 2 dim
  TGetCoord aXYGetCoord[2] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Y
  };
  TName aXYName[2] = {X,Y};

  TGetCoord aYZGetCoord[2] = {
    &SMDS_MeshNode::Y, 
    &SMDS_MeshNode::Z
  };
  TName aYZName[2] = {Y,Z};

  TGetCoord aXZGetCoord[2] = {
    &SMDS_MeshNode::X, 
    &SMDS_MeshNode::Z
  };
  TName aXZName[2] = {X,Z};

  // 1 dim
  TGetCoord aXGetCoord[1] = {
    &SMDS_MeshNode::X
  };
  TName aXName[1] = {X};

  TGetCoord aYGetCoord[1] = {
    &SMDS_MeshNode::Y
  };
  TName aYName[1] = {Y};

  TGetCoord aZGetCoord[1] = {
    &SMDS_MeshNode::Z
  };
  TName aZName[1] = {Z};


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
    MED::TFloatVector::value_type GetCoord(TInt theCoodId){
      return (myCurrentNode->*myGetCoord[theCoodId])();
    }
    MED::TStringVector::value_type GetName(TInt theDimId){
      return myName[theDimId];
    }
    MED::TStringVector::value_type GetUnit(TInt theDimId){
      return myUnit[theDimId];
    }
  };
  typedef boost::shared_ptr<TCoordHelper> TCoordHelperPtr;
  
}

  
Driver_Mesh::Status DriverMED_W_SMESHDS_Mesh::Perform()
{
  Status aResult = DRS_OK;
  if (myMesh->hasConstructionEdges() || myMesh->hasConstructionFaces()) {
    INFOS("SMDS_MESH with hasConstructionEdges() or hasConstructionFaces() do not supports!!!");
    return DRS_FAIL;
  }
  try{
    MESSAGE("Perform - myFile : "<<myFile);

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
    TInt aMeshDimension;
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

    
    PMeshInfo aMeshInfo = myMed->CrMeshInfo(aMeshDimension,aMeshName);
    MESSAGE("Add - aMeshName : "<<aMeshName<<"; "<<aMeshInfo->GetName());
    myMed->SetMeshInfo(aMeshInfo);

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
      PFamilyInfo aFamilyInfo = (*aFamsIter)->GetFamilyInfo(myMed,aMeshInfo);
      myMed->SetFamilyInfo(aFamilyInfo);
      int aFamId = (*aFamsIter)->GetId();

      const set<const SMDS_MeshElement *>& anElems = (*aFamsIter)->GetElements();
      set<const SMDS_MeshElement *>::const_iterator anElemsIter = anElems.begin();
      for (; anElemsIter != anElems.end(); anElemsIter++)
      {
        anElemFamMap[*anElemsIter] = aFamId;
      }
    }

    // Storing SMDS nodes to the MED file for the MED mesh
    //----------------------------------------------------
#ifdef _EDF_NODE_IDS_
    typedef map<TInt,TInt> TNodeIdMap;
    TNodeIdMap aNodeIdMap;
#endif
    TInt aNbElems = myMesh->NbNodes();
    MED::TIntVector anElemNums(aNbElems);
    MED::TIntVector aFamilyNums(aNbElems);
    MED::TFloatVector aCoordinates(aNbElems*aMeshDimension);
    for(TInt iNode = 0, aStartId = 0; aCoordHelperPtr->Next(); iNode++, aStartId += aMeshDimension){
      for(TInt iCoord = 0; iCoord < aMeshDimension; iCoord++){
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
    for(TInt iCoord = 0; iCoord < aMeshDimension; iCoord++){
      aCoordNames[iCoord] = aCoordHelperPtr->GetName(iCoord);
      aCoordUnits[iCoord] = aCoordHelperPtr->GetUnit(iCoord);
    }

    const ERepere SMDS_COORDINATE_SYSTEM = eCART;

    PNodeInfo aNodeInfo = myMed->CrNodeInfo(aMeshInfo,
					    aCoordinates,
					    eFULL_INTERLACE,
					    SMDS_COORDINATE_SYSTEM,
					    aCoordNames,
					    aCoordUnits,
					    aFamilyNums,
					    anElemNums);
    MESSAGE("Perform - aNodeInfo->GetNbElem() = "<<aNbElems);
    myMed->SetNodeInfo(aNodeInfo);


    // Storing others SMDS elements to the MED file for the MED mesh
    //--------------------------------------------------------------
    EEntiteMaillage SMDS_MED_ENTITY = eMAILLE;
    const EConnectivite SMDS_MED_CONNECTIVITY = eNOD;

    // Storing SMDS Edges
    if(TInt aNbElems = myMesh->NbEdges()){
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = eARETE;
#endif
      // count edges of diff types
      int aNbSeg3 = 0, aNbSeg2 = 0;
      SMDS_EdgeIteratorPtr anIter = myMesh->edgesIterator();
      while ( anIter->more() )
        if ( anIter->next()->NbNodes() == 3 )
          ++aNbSeg3;
      aNbSeg2 = aNbElems - aNbSeg3;

      TInt aNbSeg2Conn = MED::GetNbNodes(eSEG2);
      MED::TIntVector aSeg2ElemNums, aSeg2FamilyNums, aSeg2Conn;
      aSeg2ElemNums  .reserve( aNbSeg2 );
      aSeg2FamilyNums.reserve( aNbSeg2 );
      aSeg2Conn      .reserve( aNbSeg2*aNbSeg2Conn );

      TInt aNbSeg3Conn = MED::GetNbNodes(eSEG3);
      MED::TIntVector aSeg3ElemNums, aSeg3FamilyNums, aSeg3Conn;
      aSeg3ElemNums  .reserve( aNbSeg3 );
      aSeg3FamilyNums.reserve( aNbSeg3 );
      aSeg3Conn      .reserve( aNbSeg3*aNbSeg3Conn );

      anIter = myMesh->edgesIterator();
      while ( anIter->more() ) {
	const SMDS_MeshEdge* anElem = anIter->next();
	TInt aNbNodes = anElem->NbNodes();

	TInt aNbConnectivity;
	MED::TIntVector* anElemNums;
        MED::TIntVector* aFamilyNums;
	MED::TIntVector* aConnectivity;
        switch(aNbNodes){
        case 2:
          aNbConnectivity = aNbSeg2Conn;
          anElemNums      = &aSeg2ElemNums;
          aFamilyNums     = &aSeg2FamilyNums;
          aConnectivity   = &aSeg2Conn;
          break;
        case 3:
          aNbConnectivity = aNbSeg3Conn;
          anElemNums      = &aSeg3ElemNums;
          aFamilyNums     = &aSeg3FamilyNums;
          aConnectivity   = &aSeg3Conn;
          break;
        default:
          break;
        }

        for(TInt iNode = 0; iNode < aNbNodes; iNode++) {
	  const SMDS_MeshElement* aNode = anElem->GetNode( iNode );
#ifdef _EDF_NODE_IDS_
	  aConnectivity->push_back( aNodeIdMap[aNode->GetID()] );
#else
	  aConnectivity->push_back( aNode->GetID() );
#endif
        }

	anElemNums->push_back(anElem->GetID());

        map<const SMDS_MeshElement*,int>::iterator edge_fam = anElemFamMap.find( anElem );
        if ( edge_fam != anElemFamMap.end() )
          aFamilyNums->push_back( edge_fam->second );
        else
          aFamilyNums->push_back( myEdgesDefaultFamilyId );
      }
      
      if ( aNbSeg2 ) {
        PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
                                                SMDS_MED_ENTITY,
                                                eSEG2,
                                                aSeg2Conn,
                                                SMDS_MED_CONNECTIVITY,
                                                aSeg2FamilyNums,
                                                aSeg2ElemNums);
        myMed->SetCellInfo(aCellInfo);
      }
      if ( aNbSeg3 ) {
        PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
                                                SMDS_MED_ENTITY,
                                                eSEG3,
                                                aSeg3Conn,
                                                SMDS_MED_CONNECTIVITY,
                                                aSeg3FamilyNums,
                                                aSeg3ElemNums);
        myMed->SetCellInfo(aCellInfo);
      }
    }

    // Storing SMDS Faces
    if(TInt aNbElems = myMesh->NbFaces()){
      SMDS_FaceIteratorPtr anIter = myMesh->facesIterator();
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = eFACE;
#endif
      TInt aNbTriaConn = MED::GetNbNodes(eTRIA3);
      MED::TIntVector anTriaElemNums; 
      anTriaElemNums.reserve(aNbElems);
      MED::TIntVector aTriaFamilyNums;
      aTriaFamilyNums.reserve(aNbElems);
      MED::TIntVector aTriaConn;
      aTriaConn.reserve(aNbElems*aNbTriaConn);

      TInt aNbTria6Conn = MED::GetNbNodes(eTRIA6);
      MED::TIntVector anTria6ElemNums; 
      anTria6ElemNums.reserve(aNbElems);
      MED::TIntVector aTria6FamilyNums;
      aTria6FamilyNums.reserve(aNbElems);
      MED::TIntVector aTria6Conn;
      aTria6Conn.reserve(aNbElems*aNbTria6Conn);

      TInt aNbQuadConn = MED::GetNbNodes(eQUAD4);
      MED::TIntVector aQuadElemNums;
      aQuadElemNums.reserve(aNbElems);
      MED::TIntVector aQuadFamilyNums;
      aQuadFamilyNums.reserve(aNbElems);
      MED::TIntVector aQuadConn;
      aQuadConn.reserve(aNbElems*aNbQuadConn);

      TInt aNbQuad8Conn = MED::GetNbNodes(eQUAD8);
      MED::TIntVector aQuad8ElemNums;
      aQuad8ElemNums.reserve(aNbElems);
      MED::TIntVector aQuad8FamilyNums;
      aQuad8FamilyNums.reserve(aNbElems);
      MED::TIntVector aQuad8Conn;
      aQuad8Conn.reserve(aNbElems*aNbQuad8Conn);

      MED::TIntVector aPolygoneElemNums;
      aPolygoneElemNums.reserve(aNbElems);
      MED::TIntVector aPolygoneInds;
      aPolygoneInds.reserve(aNbElems + 1);
      aPolygoneInds.push_back(1); // reference on the first element in the connectivities
      MED::TIntVector aPolygoneFamilyNums;
      aPolygoneFamilyNums.reserve(aNbElems);
      MED::TIntVector aPolygoneConn;
      aPolygoneConn.reserve(aNbElems*aNbQuadConn);

      for(TInt iElem = 0; iElem < aNbElems && anIter->more(); iElem++){
	const SMDS_MeshFace* anElem = anIter->next();
	TInt aNbNodes = anElem->NbNodes();
	SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	TInt aNbConnectivity;
	MED::TIntVector* anElemNums;
        MED::TIntVector* aFamilyNums;
	MED::TIntVector* aConnectivity;
        if (anElem->IsPoly()) {
	  aNbConnectivity = aNbNodes;
          anElemNums = &aPolygoneElemNums;
          aFamilyNums = &aPolygoneFamilyNums;
          aConnectivity = &aPolygoneConn;
        }
        else {
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
          case 6:
            aNbConnectivity = aNbTria6Conn;
            anElemNums = &anTria6ElemNums;
            aFamilyNums = &aTria6FamilyNums;
            aConnectivity = &aTria6Conn;
            break;
          case 8:
            aNbConnectivity = aNbQuad8Conn;
            anElemNums = &aQuad8ElemNums;
            aFamilyNums = &aQuad8FamilyNums;
            aConnectivity = &aQuad8Conn;
            break;
          default:
            break;
          }
        }
	MED::TIntVector aVector(aNbNodes);
	for(TInt iNode = 0; aNodesIter->more(); iNode++){
	  const SMDS_MeshElement* aNode = aNodesIter->next();
#ifdef _EDF_NODE_IDS_
	  aVector[iNode] = aNodeIdMap[aNode->GetID()];
#else
	  aVector[iNode] = aNode->GetID();
#endif
	}

	TInt aSize = aConnectivity->size();
	aConnectivity->resize(aSize+aNbConnectivity);
	// There is some differences between SMDS and MED in cells mapping
	switch(aNbNodes){
	case 4:
	  (*aConnectivity)[aSize+0] = aVector[0];
	  (*aConnectivity)[aSize+1] = aVector[1];
	  (*aConnectivity)[aSize+2] = aVector[3];  
	  (*aConnectivity)[aSize+3] = aVector[2];  
	default:
	  for(TInt iNode = 0; iNode < aNbNodes; iNode++) 
	    (*aConnectivity)[aSize+iNode] = aVector[iNode];
	}

        if (anElem->IsPoly()) {
          // fill indices for polygonal element
          TInt aPrevPos = aPolygoneInds.back();
          aPolygoneInds.push_back(aPrevPos + aNbNodes);
        }

	anElemNums->push_back(anElem->GetID());

        if (anElemFamMap.find(anElem) != anElemFamMap.end())
          aFamilyNums->push_back(anElemFamMap[anElem]);
        else
          aFamilyNums->push_back(myFacesDefaultFamilyId);
      }
      if(TInt aNbElems = anTriaElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eTRIA3,
						aTriaConn,
						SMDS_MED_CONNECTIVITY,
						aTriaFamilyNums,
						anTriaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eTRIA3<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = aQuadElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eQUAD4,
						aQuadConn,
						SMDS_MED_CONNECTIVITY,
						aQuadFamilyNums,
						aQuadElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eQUAD4<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anTria6ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eTRIA6,
						aTria6Conn,
						SMDS_MED_CONNECTIVITY,
						aTria6FamilyNums,
						anTria6ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eTRIA6<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = aQuad8ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eQUAD8,
						aQuad8Conn,
						SMDS_MED_CONNECTIVITY,
						aQuad8FamilyNums,
						aQuad8ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eQUAD8<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = aPolygoneElemNums.size()){
        // add one element in connectivities,
        // referenced by the last element in indices
        aPolygoneConn.push_back(0);

	PPolygoneInfo aCellInfo = myMed->CrPolygoneInfo(aMeshInfo,
                                                        SMDS_MED_ENTITY,
                                                        ePOLYGONE,
                                                        aPolygoneInds,
                                                        aPolygoneConn,
                                                        SMDS_MED_CONNECTIVITY,
                                                        aPolygoneFamilyNums,
                                                        aPolygoneElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePOLYGONE<<"; aNbElems = "<<aNbElems);
	myMed->SetPolygoneInfo(aCellInfo);
      }
    }

    // Storing SMDS Volumes
    if(TInt aNbElems = myMesh->NbVolumes()){
      SMDS_VolumeIteratorPtr anIter = myMesh->volumesIterator();
#ifdef _ELEMENTS_BY_DIM_
      SMDS_MED_ENTITY = eMAILLE;
#endif
      TInt aNbTetraConn = MED::GetNbNodes(eTETRA4);
      MED::TIntVector anTetraElemNums; 
      anTetraElemNums.reserve(aNbElems);
      MED::TIntVector aTetraFamilyNums;
      aTetraFamilyNums.reserve(aNbElems);
      MED::TIntVector aTetraConn;
      aTetraConn.reserve(aNbElems*aNbTetraConn);

      TInt aNbPyraConn = MED::GetNbNodes(ePYRA5);
      MED::TIntVector anPyraElemNums; 
      anPyraElemNums.reserve(aNbElems);
      MED::TIntVector aPyraFamilyNums;
      aPyraFamilyNums.reserve(aNbElems);
      MED::TIntVector aPyraConn;
      aPyraConn.reserve(aNbElems*aNbPyraConn);

      TInt aNbPentaConn = MED::GetNbNodes(ePENTA6);
      MED::TIntVector anPentaElemNums; 
      anPentaElemNums.reserve(aNbElems);
      MED::TIntVector aPentaFamilyNums;
      aPentaFamilyNums.reserve(aNbElems);
      MED::TIntVector aPentaConn;
      aPentaConn.reserve(aNbElems*aNbPentaConn);

      TInt aNbHexaConn = MED::GetNbNodes(eHEXA8);
      MED::TIntVector aHexaElemNums;
      aHexaElemNums.reserve(aNbElems);
      MED::TIntVector aHexaFamilyNums;
      aHexaFamilyNums.reserve(aNbElems);
      MED::TIntVector aHexaConn;
      aHexaConn.reserve(aNbElems*aNbHexaConn);

      TInt aNbTetra10Conn = MED::GetNbNodes(eTETRA10);
      MED::TIntVector anTetra10ElemNums; 
      anTetra10ElemNums.reserve(aNbElems);
      MED::TIntVector aTetra10FamilyNums;
      aTetra10FamilyNums.reserve(aNbElems);
      MED::TIntVector aTetra10Conn;
      aTetra10Conn.reserve(aNbElems*aNbTetra10Conn);

      TInt aNbPyra13Conn = MED::GetNbNodes(ePYRA13);
      MED::TIntVector anPyra13ElemNums; 
      anPyra13ElemNums.reserve(aNbElems);
      MED::TIntVector aPyra13FamilyNums;
      aPyra13FamilyNums.reserve(aNbElems);
      MED::TIntVector aPyra13Conn;
      aPyra13Conn.reserve(aNbElems*aNbPyra13Conn);

      TInt aNbPenta15Conn = MED::GetNbNodes(ePENTA15);
      MED::TIntVector anPenta15ElemNums; 
      anPenta15ElemNums.reserve(aNbElems);
      MED::TIntVector aPenta15FamilyNums;
      aPenta15FamilyNums.reserve(aNbElems);
      MED::TIntVector aPenta15Conn;
      aPenta15Conn.reserve(aNbElems*aNbPenta15Conn);

      TInt aNbHexa20Conn = MED::GetNbNodes(eHEXA20);
      MED::TIntVector aHexa20ElemNums;
      aHexa20ElemNums.reserve(aNbElems);
      MED::TIntVector aHexa20FamilyNums;
      aHexa20FamilyNums.reserve(aNbElems);
      MED::TIntVector aHexa20Conn;
      aHexa20Conn.reserve(aNbElems*aNbHexa20Conn);

      MED::TIntVector aPolyedreElemNums;
      aPolyedreElemNums.reserve(aNbElems);
      MED::TIntVector aPolyedreInds;
      aPolyedreInds.reserve(aNbElems + 1);
      aPolyedreInds.push_back(1); // reference on the first element in the faces
      MED::TIntVector aPolyedreFaces;
      aPolyedreFaces.reserve(aNbElems + 1);
      aPolyedreFaces.push_back(1); // reference on the first element in the connectivities
      MED::TIntVector aPolyedreFamilyNums;
      aPolyedreFamilyNums.reserve(aNbElems);
      MED::TIntVector aPolyedreConn;
      aPolyedreConn.reserve(aNbElems*aNbHexaConn);

      for(TInt iElem = 0; iElem < aNbElems && anIter->more(); iElem++){
	const SMDS_MeshVolume* anElem = anIter->next();

        MED::TIntVector* anElemNums;
        MED::TIntVector* aFamilyNums;

        if (anElem->IsPoly()) {
          const SMDS_PolyhedralVolumeOfNodes* aPolyedre =
            (const SMDS_PolyhedralVolumeOfNodes*) anElem;
          if (!aPolyedre) {
            MESSAGE("Warning: bad volumic element");
            continue;
          }

          anElemNums = &aPolyedreElemNums;
          aFamilyNums = &aPolyedreFamilyNums;

          TInt aNodeId, aNbFaces = aPolyedre->NbFaces();
          for (int iface = 1; iface <= aNbFaces; iface++) {
            int aNbFaceNodes = aPolyedre->NbFaceNodes(iface);
            for (int inode = 1; inode <= aNbFaceNodes; inode++) {
              aNodeId = aPolyedre->GetFaceNode(iface, inode)->GetID();
#ifdef _EDF_NODE_IDS_
              aPolyedreConn.push_back(aNodeIdMap[aNodeId]);
#else
              aPolyedreConn.push_back(aNodeId);
#endif
            }
            TInt aPrevPos = aPolyedreFaces.back();
            aPolyedreFaces.push_back(aPrevPos + aNbFaceNodes);
          }
          TInt aPrevPos = aPolyedreInds.back();
          aPolyedreInds.push_back(aPrevPos + aNbFaces);

        }
        else {
          TInt aNbNodes = anElem->NbNodes();
          SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
          TInt aNbConnectivity;
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
	    break;
          case 10:
            aNbConnectivity = aNbTetra10Conn;
            anElemNums = &anTetra10ElemNums;
            aFamilyNums = &aTetra10FamilyNums;
            aConnectivity = &aTetra10Conn;
            break;
          case 13:
            aNbConnectivity = aNbPyra13Conn;
            anElemNums = &anPyra13ElemNums;
            aFamilyNums = &aPyra13FamilyNums;
            aConnectivity = &aPyra13Conn;
            break;
          case 15:
            aNbConnectivity = aNbPenta15Conn;
            anElemNums = &anPenta15ElemNums;
            aFamilyNums = &aPenta15FamilyNums;
            aConnectivity = &aPenta15Conn;
            break;
          case 20:
            aNbConnectivity = aNbHexa20Conn;
            anElemNums = &aHexa20ElemNums;
            aFamilyNums = &aHexa20FamilyNums;
            aConnectivity = &aHexa20Conn;
          }

          TInt aSize = aConnectivity->size();
          aConnectivity->resize(aSize + aNbConnectivity);

          MED::TIntVector aVector(aNbNodes);
          for(TInt iNode = 0; aNodesIter->more(); iNode++){
            const SMDS_MeshElement* aNode = aNodesIter->next();
#ifdef _EDF_NODE_IDS_
            aVector[iNode] = aNodeIdMap[aNode->GetID()];
#else
            aVector[iNode] = aNode->GetID();
#endif
          }
          // There is some difference between SMDS and MED in cells mapping
          switch(aNbNodes){
          case 5:
            (*aConnectivity)[aSize+0] = aVector[0];
            (*aConnectivity)[aSize+1] = aVector[3];
            (*aConnectivity)[aSize+2] = aVector[2];  
            (*aConnectivity)[aSize+3] = aVector[1];  
            (*aConnectivity)[aSize+4] = aVector[4];  
          default:
            for(TInt iNode = 0; iNode < aNbNodes; iNode++) 
              (*aConnectivity)[aSize+iNode] = aVector[iNode];
          }
        }

        anElemNums->push_back(anElem->GetID());

        if (anElemFamMap.find(anElem) != anElemFamMap.end())
          aFamilyNums->push_back(anElemFamMap[anElem]);
        else
          aFamilyNums->push_back(myVolumesDefaultFamilyId);
      }

      if(TInt aNbElems = anTetraElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eTETRA4,
						aTetraConn,
						SMDS_MED_CONNECTIVITY,
						aTetraFamilyNums,
						anTetraElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eTETRA4<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anPyraElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						ePYRA5,
						aPyraConn,
						SMDS_MED_CONNECTIVITY,
						aPyraFamilyNums,
						anPyraElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePYRA5<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anPentaElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						ePENTA6,
						aPentaConn,
						SMDS_MED_CONNECTIVITY,
						aPentaFamilyNums,
						anPentaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePENTA6<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = aHexaElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eHEXA8,
						aHexaConn,
						SMDS_MED_CONNECTIVITY,
						aHexaFamilyNums,
						aHexaElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eHEXA8<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anTetra10ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eTETRA10,
						aTetra10Conn,
						SMDS_MED_CONNECTIVITY,
						aTetra10FamilyNums,
						anTetra10ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eTETRA10<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anPyra13ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						ePYRA13,
						aPyra13Conn,
						SMDS_MED_CONNECTIVITY,
						aPyra13FamilyNums,
						anPyra13ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePYRA13<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = anPenta15ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						ePENTA15,
						aPenta15Conn,
						SMDS_MED_CONNECTIVITY,
						aPenta15FamilyNums,
						anPenta15ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePENTA15<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }
      if(TInt aNbElems = aHexa20ElemNums.size()){
	PCellInfo aCellInfo = myMed->CrCellInfo(aMeshInfo,
						SMDS_MED_ENTITY,
						eHEXA20,
						aHexa20Conn,
						SMDS_MED_CONNECTIVITY,
						aHexa20FamilyNums,
						aHexa20ElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<eHEXA20<<"; aNbElems = "<<aNbElems);
	myMed->SetCellInfo(aCellInfo);
      }

      if(TInt aNbElems = aPolyedreElemNums.size()){
        // add one element in connectivities,
        // referenced by the last element in faces
        aPolyedreConn.push_back(0);

	PPolyedreInfo aCellInfo = myMed->CrPolyedreInfo(aMeshInfo,
                                                        SMDS_MED_ENTITY,
                                                        ePOLYEDRE,
                                                        aPolyedreInds,
                                                        aPolyedreFaces,
                                                        aPolyedreConn,
                                                        SMDS_MED_CONNECTIVITY,
                                                        aPolyedreFamilyNums,
                                                        aPolyedreElemNums);
	MESSAGE("Perform - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<ePOLYEDRE<<"; aNbElems = "<<aNbElems);
	myMed->SetPolyedreInfo(aCellInfo);
      }
    }
  }
  catch(const std::exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }
  catch(...) {
    INFOS("Unknown exception was cought !!!");
  }

  myMeshId = -1;
  myGroups.clear();
  mySubMeshes.clear();
  return aResult;
}
