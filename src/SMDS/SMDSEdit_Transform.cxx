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
//  File   : SMDSEdit_Transform.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
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
 
