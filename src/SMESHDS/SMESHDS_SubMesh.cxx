//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESH_SubMesh.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_SubMesh.ixx"
#include "SMDS_MapIteratorOfExtendedMap.hxx"

//=======================================================================
//function : SMESHDS_SubMesh
//purpose  : 
//=======================================================================
SMESHDS_SubMesh::SMESHDS_SubMesh(const Handle(SMDS_Mesh)& M) : myMesh(M)
{
  myListOfEltIDIsUpdate  = Standard_False;
  myListOfNodeIDIsUpdate = Standard_False;
}

//=======================================================================
//function : AddElement
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddElement (const Handle(SMDS_MeshElement)& ME) 
{
  myElements.Add(ME);
  myListOfEltIDIsUpdate = Standard_False;
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::RemoveElement(const Handle(SMDS_MeshElement)& ME) 
{
  myElements.Remove(ME);
  myListOfEltIDIsUpdate = Standard_False;
}
//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddNode (const Handle(SMDS_MeshNode)& N) 
{
  myNodes.Add(N);
  myListOfNodeIDIsUpdate = Standard_False;
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::RemoveNode (const Handle(SMDS_MeshNode)& N) 
{
  myNodes.Remove(N);
  myListOfNodeIDIsUpdate = Standard_False;
}

//=======================================================================
//function : NbElements
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_SubMesh::NbElements() 
{
  return myElements.Extent();
}

//=======================================================================
//function : GetElements
//purpose  : 
//=======================================================================
const SMDS_MapOfMeshElement& SMESHDS_SubMesh::GetElements() 
{
  return myElements;
}
//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_SubMesh::NbNodes() 
{
  return myNodes.Extent();
}

//=======================================================================
//function : GetNodes
//purpose  : 
//=======================================================================
const SMDS_MapOfMeshElement& SMESHDS_SubMesh::GetNodes() 
{
  return myNodes;
}

//=======================================================================
//function : GetIDElements
//purpose  : 
//=======================================================================
const TColStd_ListOfInteger& SMESHDS_SubMesh::GetIDElements()
{
  if (!myListOfEltIDIsUpdate) {
    myListOfEltID.Clear();
    for (SMDS_MapIteratorOfExtendedMap it(myElements); it.More(); it.Next()) {
      myListOfEltID.Append(it.Key()->GetID());
    }
    myListOfEltIDIsUpdate = Standard_True;
  }
  return myListOfEltID;
}

//=======================================================================
//function : GetIDNodes
//purpose  : 
//=======================================================================
const TColStd_ListOfInteger& SMESHDS_SubMesh::GetIDNodes()
{
  if (!myListOfNodeIDIsUpdate) {
    myListOfNodeID.Clear();
    for (SMDS_MapIteratorOfExtendedMap it(myNodes); it.More(); it.Next()) {
      myListOfNodeID.Append(it.Key()->GetID());
    }
    myListOfNodeIDIsUpdate = Standard_True;
  }
  return myListOfNodeID;
}
