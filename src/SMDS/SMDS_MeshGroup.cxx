using namespace std;
// File:	SMDS_MeshGroup.cxx
// Created:	Mon Jun  3 12:15:55 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_MeshGroup.ixx"
#include "SMDS_ListIteratorOfListOfMeshGroup.hxx"

//=======================================================================
//function : SMDS_MeshGroup
//purpose  : 
//=======================================================================

SMDS_MeshGroup::SMDS_MeshGroup(const Handle(SMDS_Mesh)& aMesh)
  :myMesh(aMesh),myType(SMDSAbs_All)
{
}

//=======================================================================
//function : SMDS_MeshGroup
//purpose  : 
//=======================================================================

SMDS_MeshGroup::SMDS_MeshGroup(const Handle(SMDS_MeshGroup)& parent)
  :myMesh(parent->myMesh),myType(SMDSAbs_All),myParent(parent)
{
}

//=======================================================================
//function : AddSubGroup
//purpose  : 
//=======================================================================

Handle(SMDS_MeshGroup) SMDS_MeshGroup::AddSubGroup()
{
  Handle(SMDS_MeshGroup) subgroup = new SMDS_MeshGroup(this);
  if (!subgroup.IsNull()) {
    myChildren.Append(subgroup);
  }
  return subgroup;
}

//=======================================================================
//function : RemoveSubGroup
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_MeshGroup::RemoveSubGroup(const Handle(SMDS_MeshGroup)& aGroup)
{
  Standard_Boolean found = Standard_False;

  SMDS_ListIteratorOfListOfMeshGroup itgroup(myChildren);
  for (;itgroup.More() && !found; itgroup.Next()) {
    Handle(SMDS_MeshGroup) subgroup;
    subgroup = itgroup.Value();
    if (subgroup == aGroup) {
      found = Standard_True;
      myChildren.Remove(itgroup);
    }
  }

  return found;
}

//=======================================================================
//function : RemoveFromParent
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_MeshGroup::RemoveFromParent()
{
  if (myParent.IsNull())
    return Standard_False;

  return (myParent->RemoveSubGroup(this));

}

//=======================================================================
//function : Clear
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Clear()
{
  myElements.Clear();
  myType = SMDSAbs_All;
}

//=======================================================================
//function : IsEmpty
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_MeshGroup::IsEmpty() const
{
  return myElements.IsEmpty();
}

//=======================================================================
//function : Extent
//purpose  : 
//=======================================================================

Standard_Integer SMDS_MeshGroup::Extent() const
{
  return myElements.Extent();
}

//=======================================================================
//function : Add
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Add(const Handle(SMDS_MeshElement)& ME)
{
  // the type of the group is determined by the first element added
  if (myElements.IsEmpty()) {
    myType = ME->GetType();
  }

  if (ME->GetType() != myType) { 
    Standard_TypeMismatch::Raise("SMDS_MeshGroup::Add");
  }

  myElements.Add(ME);
}


//=======================================================================
//function : Remove
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Remove(const Handle(SMDS_MeshElement)& ME)
{
  myElements.Remove(ME);
  if (myElements.IsEmpty())
    myType = SMDSAbs_All;
}

//=======================================================================
//function : Type
//purpose  : 
//=======================================================================

SMDSAbs_ElementType SMDS_MeshGroup::Type() const
{
  return myType;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_MeshGroup::Contains(const Handle(SMDS_MeshElement)& ME) const
{
  return myElements.Contains(ME);
}
