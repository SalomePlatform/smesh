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

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_W_SMDS_Mesh.h"
#include "DriverMED_Family.h"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

#include "MEDA_Wrapper.hxx"
#include <sstream>	

#include "MED_Utilities.hxx"

DriverMED_W_SMESHDS_Mesh::DriverMED_W_SMESHDS_Mesh()
     :
       myMesh (NULL),
       myFile (""),
       myFileId (-1),
       myMeshId (-1),
       myAllSubMeshes (false),
       myDoGroupOfNodes (false),
       myDoGroupOfEdges (false),
       myDoGroupOfFaces (false),
       myDoGroupOfVolumes (false)
{
}

DriverMED_W_SMESHDS_Mesh::~DriverMED_W_SMESHDS_Mesh()
{
}

void DriverMED_W_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
  myMesh = aMesh;
}

void DriverMED_W_SMESHDS_Mesh::SetFile(string aFile)
{
  myFile = aFile;
}

void DriverMED_W_SMESHDS_Mesh::SetFileId(med_idt aFileId)
{
  myFileId = aFileId;
}

void DriverMED_W_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
  myMeshId = aMeshId;
}

void DriverMED_W_SMESHDS_Mesh::SetMeshName(string theMeshName)
{
  myMeshName = theMeshName;
}

void DriverMED_W_SMESHDS_Mesh::AddGroup(SMESHDS_Group* theGroup)
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

void DriverMED_W_SMESHDS_Mesh::Write()
{
  string myClass = string("SMDS_Mesh");
  string myExtension = string("MED");

  DriverMED_W_SMDS_Mesh *myWriter = new DriverMED_W_SMDS_Mesh;

  myWriter->SetMesh(myMesh);
  //  myWriter->SetFile(myFile);
  myWriter->SetMeshId(myMeshId);
  myWriter->SetFileId(myFileId);

  myWriter->Write();
}

void DriverMED_W_SMESHDS_Mesh::Add()
{
  if (myMesh->hasConstructionEdges() || myMesh->hasConstructionFaces()) {
    INFOS("SMDS_MESH with hasConstructionEdges() or hasConstructionFaces() do not supports!!!");
    return;
  }
  try{
    using namespace MEDA;
    using namespace boost;

    MESSAGE("Add - myFile : "<<myFile);
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
    const int SMDS_MESH_DIM = 3;
    PMeshInfo aMeshInfo = TWrapper::CrMeshInfo(SMDS_MESH_DIM,aMeshName);
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

    MESSAGE("Add - aFamilyInfo");
    map<const SMDS_MeshElement *, int> anElemFamMap;
    list<DriverMED_FamilyPtr> aFamilies;
    if (myAllSubMeshes) {
      SMESHDS_Mesh* aSMESHDSMesh = dynamic_cast<SMESHDS_Mesh*>(myMesh);
      if (!aSMESHDSMesh) {
        EXCEPTION(runtime_error,"Can not cast SMDS_Mesh to SMESHDS_Mesh");
      }
      aFamilies = DriverMED_Family::MakeFamilies
        (aSMESHDSMesh->SubMeshes(), myGroups,
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
    typedef map<med_int,med_int> TNodeIdMap;
    TNodeIdMap aNodeIdMap;

    med_int aNbElems = myMesh->NbNodes();
    MED::TIntVector anElemNums(aNbElems);
    MED::TIntVector aFamilyNums(aNbElems);
    MED::TFloatVector aCoordinates(aNbElems*SMDS_MESH_DIM);
    SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
    for(med_int iNode = 0, iCoord = 0; aNodesIter->more(); iNode++, iCoord+=SMDS_MESH_DIM){
      const SMDS_MeshNode* aNode = aNodesIter->next();
      aCoordinates[iCoord] = aNode->X();
      aCoordinates[iCoord+1] = aNode->Y();
      aCoordinates[iCoord+2] = aNode->Z();
      TNodeIdMap::key_type aNodeId = aNode->GetID();
      anElemNums[iNode] = aNodeId;
      aNodeIdMap[aNodeId] = iNode+1;
      //cout<<aNode->GetID()<<": "<<aNode->X()<<", "<<aNode->Y()<<", "<<aNode->Z()<<endl;

      if (anElemFamMap.find(aNode) != anElemFamMap.end())
        aFamilyNums[iNode] = anElemFamMap[aNode];
      else
        aFamilyNums[iNode] = myNodesDefaultFamilyId;
    }

    MED::TStringVector aCoordNames(3);
    aCoordNames[0] = "x";  aCoordNames[1] = "y";  aCoordNames[2] = "z";

    MED::TStringVector aCoordUnits(3);
    aCoordUnits[0] = "m";  aCoordUnits[1] = "m";  aCoordUnits[2] = "m";

    const med_repere SMDS_COORDINATE_SYSTEM = MED_CART;

    PNodeInfo aNodeInfo = TWrapper::CrNodeInfo(aMeshInfo,
					       SMDS_COORDINATE_SYSTEM,
					       aCoordinates,
					       aCoordNames,
					       aCoordUnits,
					       aFamilyNums,
					       anElemNums);
    MESSAGE("Add - aNodeInfo->GetNbElem() = "<<aNbElems);
    aMed.SetNodeInfo(aNodeInfo);


    // Storing others SMDS elements to the MED file for the MED mesh
    //--------------------------------------------------------------
    const med_entite_maillage SMDS_MED_ENTITY = MED_MAILLE;
    const med_connectivite SMDS_MED_CONNECTIVITY = MED_NOD;

    // Storing SMDS Edges
    if(med_int aNbElems = myMesh->NbEdges()){
      SMDS_EdgeIteratorPtr anIter = myMesh->edgesIterator();
      med_int aNbConnectivity = MED::GetNbConn(SMDS_MED_ENTITY,MED_SEG2,SMDS_MESH_DIM);
      MED::TIntVector anElemNums(aNbElems);
      MED::TIntVector aFamilyNums(aNbElems);
      MED::TIntVector aConnectivity(aNbElems*aNbConnectivity);

      for(med_int iElem = 0, iConn = 0; anIter->more(); iElem++, iConn+=aNbConnectivity){
	const SMDS_MeshEdge* anElem = anIter->next();
	SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	for(med_int iNode = 0; iNode < aNbConnectivity && aNodesIter->more(); iNode++){
	  const SMDS_MeshElement* aNode = aNodesIter->next();
	  aConnectivity[iConn+iNode] = aNodeIdMap[aNode->GetID()];
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

      med_int aNbTriaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_TRIA3,SMDS_MESH_DIM);
      MED::TIntVector anTriaElemNums; 
      anTriaElemNums.reserve(aNbElems);
      MED::TIntVector aTriaFamilyNums;
      aTriaFamilyNums.reserve(aNbElems);
      MED::TIntVector aTriaConn;
      aTriaConn.reserve(aNbElems*aNbTriaConn);

      med_int aNbQuadConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_QUAD4,SMDS_MESH_DIM);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_TRIA3<<"; aNbElems = "<<aNbElems);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_QUAD4<<"; aNbElems = "<<aNbElems);
	aMed.SetCellInfo(aCellInfo);
      }
    }

    // Storing SMDS Volumes
    if(med_int aNbElems = myMesh->NbVolumes()){
      SMDS_VolumeIteratorPtr anIter = myMesh->volumesIterator();

      med_int aNbTetraConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_TETRA4,SMDS_MESH_DIM);
      MED::TIntVector anTetraElemNums; 
      anTetraElemNums.reserve(aNbElems);
      MED::TIntVector aTetraFamilyNums;
      aTetraFamilyNums.reserve(aNbElems);
      MED::TIntVector aTetraConn;
      aTetraConn.reserve(aNbElems*aNbTetraConn);

      med_int aNbPyraConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_PYRA5,SMDS_MESH_DIM);
      MED::TIntVector anPyraElemNums; 
      anPyraElemNums.reserve(aNbElems);
      MED::TIntVector aPyraFamilyNums;
      aPyraFamilyNums.reserve(aNbElems);
      MED::TIntVector aPyraConn;
      aPyraConn.reserve(aNbElems*aNbPyraConn);

      med_int aNbPentaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_PENTA6,SMDS_MESH_DIM);
      MED::TIntVector anPentaElemNums; 
      anPentaElemNums.reserve(aNbElems);
      MED::TIntVector aPentaFamilyNums;
      aPentaFamilyNums.reserve(aNbElems);
      MED::TIntVector aPentaConn;
      aPentaConn.reserve(aNbElems*aNbPentaConn);

      med_int aNbHexaConn = MED::GetNbConn(SMDS_MED_ENTITY,MED_HEXA8,SMDS_MESH_DIM);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_TETRA4<<"; aNbElems = "<<aNbElems);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_PYRA5<<"; aNbElems = "<<aNbElems);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_PENTA6<<"; aNbElems = "<<aNbElems);
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
	MESSAGE("Add - anEntity = "<<SMDS_MED_ENTITY<<"; aGeom = "<<MED_HEXA8<<"; aNbElems = "<<aNbElems);
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
}
