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
//  File   : SMDSControl_BoundaryEdges.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
#include "SMDSControl_BoundaryEdges.ixx"
#include "SMDSControl.hxx"

#include "SMDS_MeshFacesIterator.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MapIteratorOfExtendedMap.hxx"

//=======================================================================
//function : SMDSControl_BoundaryEdges
//purpose  : 
//=======================================================================

SMDSControl_BoundaryEdges::SMDSControl_BoundaryEdges(const Handle(SMDS_Mesh)& M)
  :SMDSControl_MeshBoundary(M)
{
}

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

void SMDSControl_BoundaryEdges::Compute()
{
  Standard_Integer idnode1;
  Standard_Integer idnode2;
  Standard_Integer nbedg;

  myBoundaryMesh = myMesh->AddSubMesh();
  SMDS_MeshFacesIterator itface(myMesh);

  for (;itface.More();itface.Next()) {
    Handle(SMDS_MeshElement) ME = itface.Value();

    nbedg = ME->NbEdges();

    for (Standard_Integer iedge=1; iedge<=nbedg; ++iedge) {
      ME->GetEdgeDefinedByNodes(iedge,idnode1,idnode2);
      Handle(SMDS_MeshElement) edge = new SMDS_MeshEdge(0,idnode1,idnode2);
      if (!myBoundaryEdges.Add(edge))
	myBoundaryEdges.Remove(edge);
    }
  }
  
  SMDS_MapIteratorOfExtendedMap itbound(myBoundaryEdges);

  for (;itbound.More();itbound.Next()) {
    const Handle(SMDS_MeshElement)& edge = itbound.Key();
    myBoundaryMesh->AddEdge(edge->GetConnection(1),edge->GetConnection(2));
  }
}
