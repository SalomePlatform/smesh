using namespace std;
//  File      : SMESH_MeshEditor_i.cxx
//  Created   : Wed Jun 19 18:43:26 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_MeshEdgesIterator.hxx"
#include "SMDS_MeshFacesIterator.hxx"
#include "SMDS_MeshVolumesIterator.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"

#include "utilities.h"

#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>



//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(const Handle(SMESHDS_Mesh)& theMesh) {
  _myMeshDS = theMesh;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveElements(const SMESH::long_array& IDsOfElements) {
  for (int i = 0 ; i< IDsOfElements.length(); i++) {
    CORBA::Long index = IDsOfElements[i];
    _myMeshDS->RemoveElement(index);
    MESSAGE ( "Element "<< index  << " was removed" )
  }
 return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::long_array& IDsOfNodes) {

  // Here we try to collect all 1D, 2D and 3D elements which contain at least one 
  // of <IDsOfNodes> in order to remove such elements. 
  // This seems correct since e.g a triangle without 1 vertex looks nonsense.
  TColStd_MapOfInteger elemsToRemove;

  for (int i = 0 ; i< IDsOfNodes.length(); i++) {

    CORBA::Long ID = IDsOfNodes[i];

    SMDS_MeshEdgesIterator edgeIt(_myMeshDS);
    for (; edgeIt.More(); edgeIt.Next()) {
      Handle(SMDS_MeshEdge) anEdge = Handle(SMDS_MeshEdge)::DownCast(edgeIt.Value());
      for (Standard_Integer i = 0; i < anEdge->NbNodes(); i++) {
	if (anEdge->GetConnection(i) == ID) {
	  Standard_Integer elemID = anEdge->GetID();
	  if (!elemsToRemove.Contains(elemID)) elemsToRemove.Add(elemID);
	}
      }
    }

    SMDS_MeshFacesIterator faceIt(_myMeshDS);
    for (; faceIt.More(); faceIt.Next()) {
      Handle(SMDS_MeshFace) aFace = Handle(SMDS_MeshFace)::DownCast(faceIt.Value());
      for (Standard_Integer i = 0; i < aFace->NbNodes(); i++) {
	if (aFace->GetConnection(i) == ID) {
	  Standard_Integer elemID = aFace->GetID();
	  if (!elemsToRemove.Contains(elemID)) elemsToRemove.Add(elemID);
	}
      }
    }
  
    SMDS_MeshVolumesIterator volIt(_myMeshDS);
    for (; volIt.More(); volIt.Next()) {
      Handle(SMDS_MeshVolume) aVol = Handle(SMDS_MeshVolume)::DownCast(volIt.Value());
      for (Standard_Integer i = 0; i < aVol->NbNodes(); i++) {
	if (aVol->GetConnection(i) == ID) {
	  Standard_Integer elemID = aVol->GetID();
	  if (!elemsToRemove.Contains(elemID)) elemsToRemove.Add(elemID);
	}
      }
    }
  }

  // Now remove them!
  TColStd_MapIteratorOfMapOfInteger it(elemsToRemove);
  for (; it.More(); it.Next()) {
    Standard_Integer elemID = it.Key();
    _myMeshDS->RemoveElement(elemID);
    MESSAGE("RemoveNodes(): element removed: " << elemID)
  }
  
  // It's nodes' turn to die
  for (int i = 0 ; i< IDsOfNodes.length(); i++) {
    CORBA::Long index = IDsOfNodes[i];
    _myMeshDS->RemoveNode(index);
    MESSAGE ( "Node "<< index  << " was removed" )
  }
 return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddEdge(const SMESH::long_array& IDsOfNodes) {
  int NbNodes = IDsOfNodes.length();
  if ( NbNodes == 2 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    int idTri = _myMeshDS->AddEdge(index1,index2);
  }
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddNode(CORBA::Double x,
					   CORBA::Double y,
					   CORBA::Double z) {
  MESSAGE( " AddNode " << x << " , " << y << " , " << z )
  int idNode = _myMeshDS->AddNode(x,y,z);
  MESSAGE( " idNode " << idNode )
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddFace(const SMESH::long_array& IDsOfNodes) {
  int NbNodes = IDsOfNodes.length();
  if ( NbNodes == 3 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    int idTri = _myMeshDS->AddFace(index1,index2,index3);
  } else  if ( NbNodes == 4 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    CORBA::Long index4 = IDsOfNodes[3];
    int idTri = _myMeshDS->AddFace(index1,index2,index3,index4);
  }
 return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddVolume(const SMESH::long_array& IDsOfNodes) {
  int NbNodes = IDsOfNodes.length();
  if ( NbNodes == 4 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    CORBA::Long index4 = IDsOfNodes[3];
    int idTetra = _myMeshDS->AddVolume(index1,index2,index3,index4);
  } else if ( NbNodes == 5 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    CORBA::Long index4 = IDsOfNodes[3];
    CORBA::Long index5 = IDsOfNodes[4];
    int idPyramid = _myMeshDS->AddVolume(index1,index2,index3,index4,index5);
  } else if ( NbNodes == 6 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    CORBA::Long index4 = IDsOfNodes[3];
    CORBA::Long index5 = IDsOfNodes[4];
    CORBA::Long index6 = IDsOfNodes[5];
    int idPrism = _myMeshDS->AddVolume(index1,index2,index3,index4,index5,index6);
  } else if ( NbNodes == 8 ) {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    CORBA::Long index3 = IDsOfNodes[2];
    CORBA::Long index4 = IDsOfNodes[3];
    CORBA::Long index5 = IDsOfNodes[4];
    CORBA::Long index6 = IDsOfNodes[5];
    CORBA::Long index7 = IDsOfNodes[6];
    CORBA::Long index8 = IDsOfNodes[7];
    int idHexa = _myMeshDS->AddVolume(index1,index2,index3,index4,index5,index6,index7,index8);
  } 
  return true;
};
