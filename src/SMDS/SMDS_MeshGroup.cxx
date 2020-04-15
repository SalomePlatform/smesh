// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMDS : implementation of Salome mesh data structure
//  File   : SMDS_MeshGroup.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_MeshGroup.hxx"

#include "SMDS_SetIterator.hxx"
#include "ObjectPool.hxx"

#include <utilities.h>

#include <boost/make_shared.hpp>

//=======================================================================
//function : SMDS_MeshGroup
//purpose  :
//=======================================================================

SMDS_MeshGroup::SMDS_MeshGroup(const SMDS_Mesh *         theMesh,
                               const SMDSAbs_ElementType theType)
  : SMDS_ElementHolder( theMesh ), myType( theType ), myTic( 0 )
{
}

//=======================================================================
//function : Clear
//purpose  :
//=======================================================================

void SMDS_MeshGroup::Clear()
{
  clearVector( myElements );
  myType = SMDSAbs_All;
  ++myTic;
}

//=======================================================================
//function : Add
//purpose  : 
//=======================================================================

bool SMDS_MeshGroup::Add(const SMDS_MeshElement * theElem)
{
  // the type of the group is determined by the first element added
  if ( myElements.empty() ) {
    myType = theElem->GetType();
  }
  else if ( theElem->GetType() != myType ) {
    MESSAGE("SMDS_MeshGroup::Add : Type Mismatch "<<theElem->GetType()<<"!="<<myType);
    return false;
  }

  bool added = myElements.insert( theElem ).second;

  ++myTic;

  return added;
}

//=======================================================================
//function : Remove
//purpose  :
//=======================================================================

bool SMDS_MeshGroup::Remove( const SMDS_MeshElement * theElem )
{
  TElementSet::iterator found = myElements.find(theElem);
  if ( found != myElements.end() ) {
    myElements.erase( found );
    if ( myElements.empty() ) myType = SMDSAbs_All;
    ++myTic;
    return true;
  }
  return false;
}

//=======================================================================
//function : Contains
//purpose  :
//=======================================================================

bool SMDS_MeshGroup::Contains(const SMDS_MeshElement * theElem) const
{
  return myElements.find( theElem ) != myElements.end();
}

//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::SetType(const SMDSAbs_ElementType theType)
{
  if (IsEmpty())
    myType = theType;
}

//=======================================================================
//function : GetElements
//purpose  : 
//=======================================================================

SMDS_ElemIteratorPtr SMDS_MeshGroup::GetElements() const
{
  typedef SMDS_SetIterator< const SMDS_MeshElement*, TIterator > TSetIterator;
  return boost::make_shared< TSetIterator >( myElements.begin(), myElements.end() );
}

//=======================================================================
//function : Move contents of another group
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::operator=( SMDS_MeshGroup && other )
{
  myMesh = other.myMesh;
  myType = other.myType;
  myElements = std::move( other.myElements );
  ++myTic;
}

//=======================================================================
//function : tmpClear
//purpose  : temporary remove its elements before mesh compacting
//=======================================================================

void SMDS_MeshGroup::tmpClear()
{
  myElements.clear();
}
