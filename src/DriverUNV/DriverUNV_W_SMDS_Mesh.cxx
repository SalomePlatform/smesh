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

#include <algorithm>

#include "DriverUNV_W_SMDS_Mesh.h"

#include "SMDS_Mesh.hxx"

#include "utilities.h"

#include "UNV2411_Structure.hxx"
#include "UNV2412_Structure.hxx"
#include "UNV_Utilities.hxx"

using namespace std;

Driver_Mesh::Status DriverUNV_W_SMDS_Mesh::Perform()
{
  Status aResult = DRS_OK;
  std::ofstream out_stream(myFile.c_str());
  try{
    {
      using namespace UNV2411;
      TDataSet aDataSet2411;
      // Storing SMDS nodes to the UNV file
      //-----------------------------------
      MESSAGE("Perform - myMesh->NbNodes() = "<<myMesh->NbNodes());
      SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
      for(; aNodesIter->more();){
	const SMDS_MeshNode* aNode = aNodesIter->next();
	TRecord aRec;
	aRec.coord[0] = aNode->X();
	aRec.coord[1] = aNode->Y();
	aRec.coord[2] = aNode->Z();
	const TNodeLab& aLabel = aNode->GetID();
	aDataSet2411.insert(TDataSet::value_type(aLabel,aRec));
      }
      MESSAGE("Perform - aDataSet2411.size() = "<<aDataSet2411.size());
      UNV2411::Write(out_stream,aDataSet2411);
    }
    {
      using namespace UNV2412;
      TDataSet aDataSet2412;
      // Storing SMDS Edges
      MESSAGE("Perform - myMesh->NbEdges() = "<<myMesh->NbEdges());
      if(myMesh->NbEdges()){
	SMDS_EdgeIteratorPtr anIter = myMesh->edgesIterator();
	for(; anIter->more();){
	  const SMDS_MeshEdge* anElem = anIter->next();
	  const TElementLab& aLabel = anElem->GetID();
	  int aNbNodes = anElem->NbNodes();
	  TRecord aRec;
	  aRec.node_labels.reserve(aNbNodes);
	  SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	  for(; aNodesIter->more();){
	    const SMDS_MeshElement* aNode = aNodesIter->next();
	    aRec.node_labels.push_back(aNode->GetID());
	  }
	  aRec.fe_descriptor_id = 11;
	  aDataSet2412.insert(TDataSet::value_type(aLabel,aRec));
	}
	MESSAGE("Perform - aDataSet2412.size() = "<<aDataSet2412.size());
      }
      MESSAGE("Perform - myMesh->NbFaces() = "<<myMesh->NbFaces());
      if(myMesh->NbFaces()){
	SMDS_FaceIteratorPtr anIter = myMesh->facesIterator();
	for(; anIter->more();){
	  const SMDS_MeshFace* anElem = anIter->next();
	  const TElementLab& aLabel = anElem->GetID();
	  int aNbNodes = anElem->NbNodes();
	  TRecord aRec;
	  aRec.node_labels.reserve(aNbNodes);
	  SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	  for(; aNodesIter->more();){
	    const SMDS_MeshElement* aNode = aNodesIter->next();
	    aRec.node_labels.push_back(aNode->GetID());
	  }
	  switch(aNbNodes){
	  case 3:
	    aRec.fe_descriptor_id = 41;
	    break;
	  case 4:
	    aRec.fe_descriptor_id = 44;
	    break;
	  default:
	    continue;
	  }
	  aDataSet2412.insert(TDataSet::value_type(aLabel,aRec));
	}
	MESSAGE("Perform - aDataSet2412.size() = "<<aDataSet2412.size());
      }
      MESSAGE("Perform - myMesh->NbVolumes() = "<<myMesh->NbVolumes());
      if(myMesh->NbVolumes()){
	SMDS_VolumeIteratorPtr anIter = myMesh->volumesIterator();
	for(; anIter->more();){
	  const SMDS_MeshVolume* anElem = anIter->next();
	  const TElementLab& aLabel = anElem->GetID();
	  int aNbNodes = anElem->NbNodes();
	  TRecord aRec;
	  aRec.node_labels.reserve(aNbNodes);
	  SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
	  for(; aNodesIter->more();){
	    const SMDS_MeshElement* aNode = aNodesIter->next();
	    aRec.node_labels.push_back(aNode->GetID());
	  }
	  switch(aNbNodes){
	  case 4:
	    aRec.fe_descriptor_id = 111;
	    break;
	  case 6:
	    aRec.fe_descriptor_id = 112;
	    swap(aRec.node_labels[1],aRec.node_labels[2]);
	    swap(aRec.node_labels[4],aRec.node_labels[5]);
	    break;
	  case 8:
	    aRec.fe_descriptor_id = 115;
	    break;
	  default:
	    continue;
	  }
	  aDataSet2412.insert(TDataSet::value_type(aLabel,aRec));
	}
	MESSAGE("Perform - aDataSet2412.size() = "<<aDataSet2412.size());
      }
      UNV2412::Write(out_stream,aDataSet2412);
    }
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
  return aResult;
}
