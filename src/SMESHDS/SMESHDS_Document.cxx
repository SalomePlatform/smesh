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
//  File   : SMESHDS_Document.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_Document.ixx"
#include "SMESHDS_Hypothesis.hxx"

#include <Standard_OutOfRange.hxx>

//=======================================================================
//function : Create
//purpose  : 
//=======================================================================
SMESHDS_Document::SMESHDS_Document(const Standard_Integer UserID): myUserID(UserID)
{}

//=======================================================================
//function : NewMesh
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Document::NewMesh()
{
  static Standard_Integer NewMeshID = 0;
  NewMeshID++;
  Handle (SMESHDS_Mesh) aNewMesh = new SMESHDS_Mesh (NewMeshID);
  myMeshes.Bind(NewMeshID ,aNewMesh);
  return NewMeshID;
}

//=======================================================================
//function : GetMesh
//purpose  : 
//=======================================================================
Handle(SMESHDS_Mesh) SMESHDS_Document::GetMesh(const Standard_Integer MeshID) 
{
  if (!myMeshes.IsBound(MeshID)) 
    Standard_OutOfRange::Raise("SMESHDS_Document::RemoveMesh");
  return myMeshes.Find(MeshID);
}

//=======================================================================
//function : RemoveMesh
//purpose  : 
//=======================================================================
void SMESHDS_Document::RemoveMesh(const Standard_Integer MeshID)
{
  if (!myMeshes.IsBound(MeshID)) 
    Standard_OutOfRange::Raise("SMESHDS_Document::RemoveMesh");
  myMeshes.UnBind(MeshID);
}

//=======================================================================
//function : AddHypothesis
//purpose  : 
//=======================================================================
void  SMESHDS_Document::AddHypothesis(const SMESHDS_PtrHypothesis& H)
{
  myHypothesis.Bind (H->GetID(), H);
}

//=======================================================================
//function : GetHypothesis
//purpose  : 
//=======================================================================
SMESHDS_PtrHypothesis  SMESHDS_Document::GetHypothesis(const Standard_Integer HypID) 
{
  if (!myHypothesis.IsBound(HypID)) 
    Standard_OutOfRange::Raise("SMESHDS_Document::GetHypothesis");
  return myHypothesis.Find(HypID);
}

//=======================================================================
//function : RemoveHypothesis
//purpose  : 
//=======================================================================
void SMESHDS_Document::RemoveHypothesis(const Standard_Integer HypID)
{
  if (!myHypothesis.IsBound(HypID)) 
    Standard_OutOfRange::Raise("SMESHDS_Document::RemoveHypothesis");
  myMeshes.UnBind(HypID);
}

//=======================================================================
//function : NbMeshes
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Document::NbMeshes() 
{
  return myMeshes.Extent();
}

//=======================================================================
//function : NbHypothesis
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Document::NbHypothesis() 
{
  return myHypothesis.Extent();
}

//=======================================================================
//function : InitMeshesIterator
//purpose  : 
//=======================================================================
void SMESHDS_Document::InitMeshesIterator() 
{
  myMeshesIt.Initialize(myMeshes);
}
//=======================================================================
//function : NextMesh
//purpose  : 
//=======================================================================
void SMESHDS_Document::NextMesh() 
{
  myMeshesIt.Next();
}
//=======================================================================
//function : MoreMesh
//purpose  : 
//=======================================================================
Standard_Boolean SMESHDS_Document::MoreMesh() 
{
  return myMeshesIt.More();
}
//=======================================================================
//function : CurrentMesh
//purpose  : 
//=======================================================================
Handle_SMESHDS_Mesh SMESHDS_Document::CurrentMesh() 
{
 return  myMeshesIt.Value();
}

//=======================================================================
//function : InitHypothesisIterator
//purpose  : 
//=======================================================================
void SMESHDS_Document::InitHypothesisIterator() 
{
  myHypothesisIt.Initialize(myHypothesis);
}
//=======================================================================
//function : NextMesh
//purpose  : 
//=======================================================================
void SMESHDS_Document::NextHypothesis() 
{
  myHypothesisIt.Next();
}
//=======================================================================
//function : MoreMesh
//purpose  : 
//=======================================================================
Standard_Boolean SMESHDS_Document::MoreHypothesis() 
{
  return myHypothesisIt.More();
}
//=======================================================================
//function : CurrentMesh
//purpose  : 
//=======================================================================
SMESHDS_PtrHypothesis SMESHDS_Document::CurrentHypothesis() 
{
 return  myHypothesisIt.Value();
}


