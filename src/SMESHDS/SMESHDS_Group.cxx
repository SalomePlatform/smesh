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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHDS_Group.cxx
//  Author : Michael Sazonov, OCC
//  Module : SMESH
//  $Header$

#include <SMESHDS_Group.hxx>
#include <SMESHDS_Mesh.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESHDS_Group::SMESHDS_Group (const SMESHDS_Mesh*       theMesh,
                              const SMDSAbs_ElementType theType)
     : SMDS_MeshGroup(theMesh,theType), myStoreName(""),
       myCurIndex(0), myCurID(0)
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Contains (const int theID) const
{
  const SMDS_MeshElement* aElem = findInMesh (theID);
  if (aElem)
    return SMDS_MeshGroup::Contains(aElem);
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Add (const int theID)
{
  const SMDS_MeshElement* aElem = findInMesh (theID);
  if (!aElem || SMDS_MeshGroup::Contains(aElem))
    return false;
  SMDS_MeshGroup::Add (aElem);
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Remove (const int theID)
{
  const SMDS_MeshElement* aElem = findInMesh (theID);
  if (!aElem || !SMDS_MeshGroup::Contains(aElem))
    return false;
  SMDS_MeshGroup::Remove (aElem);
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESHDS_Group::GetID (const int theIndex)
{
  if (theIndex < 1 || theIndex > Extent())
    return -1;
  if (myCurIndex < 1 || myCurIndex > theIndex) {
    InitIterator();
    myCurIndex = 0;
    myCurID = -1;
  }
  while (myCurIndex < theIndex && More()) {
    myCurIndex++;
    myCurID = Next()->GetID();
  }
  return myCurID;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

const SMDS_MeshElement* SMESHDS_Group::findInMesh (const int theID) const
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
