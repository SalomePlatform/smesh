//  SMESH SMDS : implementaion of Salome mesh data structure
//
//  Copyright (C) 2003  OPEN CASCADE
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
//  See http://www.opencascade.org or email : webmaster@opencascade.org 
//
//
//
//  File   : SMDSControl_BoundaryFaces.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
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
	

