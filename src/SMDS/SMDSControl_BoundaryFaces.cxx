using namespace std;
// File:	SMDSControl_BoundaryFaces.cxx
// Created:	Tue Mar 12 23:46:24 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDSControl_BoundaryFaces.ixx"
#include "SMDSControl.hxx"

#include "SMDS_MeshVolumesIterator.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshTriangle.hxx"
#include "SMDS_MeshQuadrangle.hxx"
#include "SMDS_MapIteratorOfExtendedMap.hxx"

//=======================================================================
//function : SMDSControl_BoundaryFaces
//purpose  : 
//=======================================================================

SMDSControl_BoundaryFaces::SMDSControl_BoundaryFaces(const Handle(SMDS_Mesh)& M)
  :SMDSControl_MeshBoundary(M)
{
}

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

void SMDSControl_BoundaryFaces::Compute()
{
  myBoundaryMesh = myMesh->AddSubMesh();
  SMDS_MeshVolumesIterator itvol(myMesh);
  
  Standard_Integer idnode[4]; // max number of nodes for a face
  Standard_Integer nbnode;

  for (;itvol.More();itvol.Next()) {
    Handle(SMDS_MeshElement) ME = itvol.Value();


    Standard_Integer nbfaces = ME->NbFaces();

    for (Standard_Integer iface=1; iface<=nbfaces; ++iface) {

      ME->GetFaceDefinedByNodes(iface,idnode,nbnode);
      // Triangle
      if (nbnode == 3) {
	Handle(SMDS_MeshElement) face = new SMDS_MeshTriangle(0,idnode[0],idnode[1],idnode[2]);
	if (!myBoundaryFaces.Add(face))
	  myBoundaryFaces.Remove(face);

	  
      } else {
	// Quadrangle
	  
	Handle(SMDS_MeshElement) face = new SMDS_MeshQuadrangle(0,idnode[0],idnode[1],idnode[2],idnode[3]);
	if (!myBoundaryFaces.Add(face))
	  myBoundaryFaces.Remove(face);
	  
      }
    } // end iface

  } // end itvol

  SMDS_MapIteratorOfExtendedMap itbound(myBoundaryFaces);

  for (;itbound.More();itbound.Next()) {
    const Handle(SMDS_MeshElement)& face = itbound.Key();
    if (face->NbNodes() == 3)
      myBoundaryMesh->AddFace(face->GetConnection(1),face->GetConnection(2),face->GetConnection(3));
    else
      myBoundaryMesh->AddFace(face->GetConnection(1),face->GetConnection(2),face->GetConnection(3),
			      face->GetConnection(4));
  }

}
	

