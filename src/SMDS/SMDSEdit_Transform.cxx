using namespace std;
// File:	SMDSEdit_Transform.cxx
// Created:	Wed May 15 21:57:17 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDSEdit_Transform.ixx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNodesIterator.hxx"
#include "SMDS_ListIteratorOfListOfMeshElement.hxx"
#include "TColStd_MapOfInteger.hxx"


//=======================================================================
//function : SMDSEdit_Transform
//purpose  : 
//=======================================================================

SMDSEdit_Transform::SMDSEdit_Transform(const Handle(SMDS_Mesh)& aMesh,const gp_Trsf& aTrsf)
  :myMesh(aMesh),myTrsf(aTrsf)
{
}

//=======================================================================
//function : SMDSEdit_Transform
//purpose  : 
//=======================================================================

SMDSEdit_Transform::SMDSEdit_Transform(const Handle(SMDS_Mesh)& aMesh,
				       const SMDS_ListOfMeshElement& aListOfME,
				       const gp_Trsf& aTrsf)
  :myMesh(aMesh),myTrsf(aTrsf)
{
  myListOfME = aListOfME;
}

//=======================================================================
//function : SetTrsf
//purpose  : 
//=======================================================================

void SMDSEdit_Transform::SetTrsf(const gp_Trsf& aTrsf)
{
  myTrsf = aTrsf;
}

//=======================================================================
//function : GetTrsf
//purpose  : 
//=======================================================================

gp_Trsf SMDSEdit_Transform::GetTrsf() const
{
  return myTrsf;
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================

void SMDSEdit_Transform::Perform()
{
  if (myListOfME.IsEmpty()) { 
    // transform the whole mesh
    SMDS_MeshNodesIterator itNodes(myMesh);
    
    for (;itNodes.More();itNodes.Next()) {
      const Handle(SMDS_MeshElement)& elem = itNodes.Value();
      Handle(SMDS_MeshNode) node = *((Handle(SMDS_MeshNode)*)&elem);
      
      gp_Pnt P = node->Pnt();
      P.Transform(myTrsf);
      node->SetPnt(P);
    }
    
  } else {
    TColStd_MapOfInteger mapnode;
    Standard_Integer nbnodes,inode;

    SMDS_ListIteratorOfListOfMeshElement itME(myListOfME);
    for (;itME.More();itME.Next()) {
      const Handle(SMDS_MeshElement)& elem = itME.Value();
      nbnodes = elem->NbNodes();

      for (inode = 1; inode <= nbnodes; ++inode) {
	const Handle(SMDS_MeshNode)& node = myMesh->GetNode(inode,elem);
	if (mapnode.Add(node->GetID())) {
	  gp_Pnt P = node->Pnt();
	  P.Transform(myTrsf);
	  node->SetPnt(P);
	}
      }
      
    }
  }
}
 
