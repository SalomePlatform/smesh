using namespace std;
// File:	SMDSControl_BoundaryEdges.cxx
// Created:	Wed Feb 20 19:28:42 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


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
