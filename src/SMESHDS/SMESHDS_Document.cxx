using namespace std;
//=============================================================================
// File      : SMESHDS_Document.cxx
// Created   : 
// Author    : Yves FRICAUD, OCC
// Project   : SALOME
// Copyright : OCC 2002
// $Header: 
//=============================================================================

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


