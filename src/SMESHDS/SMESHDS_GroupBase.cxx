//  SMESH SMESHDS : idl implementation based on 'SMESH' unit's classes
//
//  Copyright (C) 2004  CEA
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHDS_Group.cxx
//  Module : SMESH
//  $Header$

#include "SMESHDS_GroupBase.hxx"
#include "SMESHDS_Mesh.hxx"

#include "utilities.h"

using namespace std;

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESHDS_GroupBase::SMESHDS_GroupBase (const int                 theID,
                                      const SMESHDS_Mesh*       theMesh,
                                      const SMDSAbs_ElementType theType):
       myID(theID), myMesh(theMesh), myType(theType), myStoreName(""),
       myCurIndex(0), myCurID(-1)
{
  myColor = Quantity_Color( 0.0, 0.0, 0.0, Quantity_TOC_RGB );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESHDS_GroupBase::GetID (const int theIndex)
{
  if (myCurIndex < 1 || myCurIndex > theIndex) {
    myIterator = GetElements();
    myCurIndex = 0;
    myCurID = -1;
  }
  while (myCurIndex < theIndex && myIterator->more()) {
    myCurIndex++;
    myCurID = myIterator->next()->GetID();
  }
  return myCurIndex == theIndex ? myCurID : -1;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

const SMDS_MeshElement* SMESHDS_GroupBase::findInMesh (const int theID) const
{
  SMDSAbs_ElementType aType = GetType();
  const SMDS_MeshElement* aElem = NULL;
  if (aType == SMDSAbs_Node) {
    aElem = GetMesh()->FindNode(theID);
  }
  else if (aType != SMDSAbs_All) {
    aElem = GetMesh()->FindElement(theID);
    if (aElem && aType != aElem->GetType())
      aElem = NULL;
  }
  return aElem;
}

//=============================================================================
/*!
 *  Internal method: resets cached iterator, should be called by ancestors
 *  when they are modified (ex: Add() or Remove() )
 */
//=============================================================================
void SMESHDS_GroupBase::resetIterator()
{
  myCurIndex = 0;
  myCurID = -1;
}

//=======================================================================
//function : Extent
//purpose  : 
//=======================================================================

int SMESHDS_GroupBase::Extent()
{
  SMDS_ElemIteratorPtr it = GetElements();
  int nb = 0;
  if ( it )
    for ( ; it->more(); it->next() ) 
      nb++;
  return nb;
}

//=======================================================================
//function : IsEmpty
//purpose  : 
//=======================================================================

bool SMESHDS_GroupBase::IsEmpty()
{
  SMDS_ElemIteratorPtr it = GetElements();
  return ( !it || !it->more() );
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

bool SMESHDS_GroupBase::Contains (const int theID)
{
  if ( SMDS_ElemIteratorPtr it = GetElements() ) {
    while ( it->more() )
      if ( it->next()->GetID() == theID )
        return true;
  }
  return false;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

bool SMESHDS_GroupBase::Contains (const SMDS_MeshElement* elem)
{
  if ( elem )
    return Contains( elem->GetID() );
  return false;
}

//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================

void SMESHDS_GroupBase::SetType(SMDSAbs_ElementType theType)
{
  myType = theType;
}

//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================

void SMESHDS_GroupBase::SetColorGroup(int theColorGroup)
{
  if( theColorGroup < 0 || theColorGroup > 360 )
  {
    MESSAGE("SMESHDS_GroupBase::SetColorGroup : Value must be in range [0,360]");
    return;
  }

  Quantity_Color aColor( (double)theColorGroup, 1.0, 1.0, Quantity_TOC_HLS );
  SetColor( aColor );
}
  
//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================

int SMESHDS_GroupBase::GetColorGroup() const
{
  Quantity_Color aColor = GetColor();
  double aHue = aColor.Hue();
  if( aHue < 0 )
    return 0;
  return (int)( aHue );
}
  
