using namespace std;
// File:	SMDS_MapOfMeshElement.cxx
// Created:	Fri Jan 25 11:47:26 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MapOfMeshElement.ixx"
#include <Standard_NoSuchObject.hxx>


#ifndef _Standard_DomainError_HeaderFile
#include <Standard_DomainError.hxx>
#endif
#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_MeshElementMapHasher_HeaderFile
#include "SMDS_MeshElementMapHasher.hxx"
#endif
#ifndef _SMDS_StdMapNodeOfExtendedMap_HeaderFile
#include "SMDS_StdMapNodeOfExtendedMap.hxx"
#endif
#ifndef _SMDS_MapIteratorOfExtendedMap_HeaderFile
#include "SMDS_MapIteratorOfExtendedMap.hxx"
#endif
 

#define TheKey Handle_SMDS_MeshElement
#define TheKey_hxx <SMDS_MeshElement.hxx>
#define Hasher SMDS_MeshElementMapHasher
#define Hasher_hxx <SMDS_MeshElementMapHasher.hxx>
#define TCollection_StdMapNode SMDS_StdMapNodeOfExtendedMap
#define TCollection_StdMapNode_hxx <SMDS_StdMapNodeOfExtendedMap.hxx>
#define TCollection_MapIterator SMDS_MapIteratorOfExtendedMap
#define TCollection_MapIterator_hxx <SMDS_MapIteratorOfExtendedMap.hxx>
#define Handle_TCollection_StdMapNode Handle_SMDS_StdMapNodeOfExtendedMap
#define TCollection_StdMapNode_Type_() SMDS_StdMapNodeOfExtendedMap_Type_()
#define TCollection_Map SMDS_ExtendedMap
#define TCollection_Map_hxx <SMDS_ExtendedMap.hxx>



//=======================================================================
//function : Find
//purpose  : 
//=======================================================================

const Handle(SMDS_MeshElement)& SMDS_MapOfMeshElement::Find(const TheKey& K) const
{
  if (IsEmpty()) {
    Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::Find");
  }
//  Standard_NoSuchObject_Raise_if(IsEmpty(),"SMDS_MapOfMeshElement::Find");
  TCollection_StdMapNode** data = (TCollection_StdMapNode**) myData1;
  TCollection_StdMapNode* p = data[Hasher::HashCode(K,NbBuckets())];
  while (p) {
    if (Hasher::IsEqual(p->Key(),K)) {
      return p->Key();
      }
    p = (TCollection_StdMapNode*) p->Next();
  }
  Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::Find");
  return p->Key();
}

//=======================================================================
//function : ChangeFind
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement)& SMDS_MapOfMeshElement::ChangeFind(const TheKey& K)
{
  if (IsEmpty()) {
    Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::ChangeFind");
  }
//Standard_NoSuchObject_Raise_if(IsEmpty(),"SMDS_MapOfMeshElement::ChangeFind");
  TCollection_StdMapNode** data = (TCollection_StdMapNode**) myData1;
  TCollection_StdMapNode* p = data[Hasher::HashCode(K,NbBuckets())];
  while (p) {
    if (Hasher::IsEqual(p->Key(),K)) {
      return p->Key();
      }
    p = (TCollection_StdMapNode*)  p->Next();
  }
  Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::ChangeFind");
  return p->Key();
}


//=======================================================================
//function : FindID
//purpose  : 
//=======================================================================

const Handle(SMDS_MeshElement)& SMDS_MapOfMeshElement::FindID(const Standard_Integer ID) const
{
  if (IsEmpty()) {
    Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::FindID");
  }
//  Standard_NoSuchObject_Raise_if(IsEmpty(),"SMDS_MapOfMeshElement::Find");
  TCollection_StdMapNode** data = (TCollection_StdMapNode**) myData1;
  TCollection_StdMapNode* p = data[ID % NbBuckets()];
  while (p) {
    if (p->Key()->GetID() == ID) {
      return p->Key();
      }
    p = (TCollection_StdMapNode*) p->Next();
  }
  Standard_NoSuchObject::Raise("SMDS_MapOfMeshElement::Find");
  return  p->Key();
}

//=======================================================================
//function : ContainsID
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_MapOfMeshElement::ContainsID(const Standard_Integer ID) const
{
  if (IsEmpty()) return Standard_False;
  TCollection_StdMapNode** data = (TCollection_StdMapNode**) myData1;
  TCollection_StdMapNode* p = data[ID % NbBuckets()];
  while (p) {
    if (p->Key()->GetID() == ID) {
      return Standard_True;
      }
    p = (TCollection_StdMapNode*)p->Next();
  }
  return Standard_False;
}
