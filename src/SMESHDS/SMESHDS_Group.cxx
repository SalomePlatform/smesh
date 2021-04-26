// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHDS_Group.cxx
//  Module : SMESH
//

#include "SMESHDS_Group.hxx"
#include "SMESHDS_Mesh.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESHDS_Group::SMESHDS_Group (const int                 theID,
                              const SMESHDS_Mesh*       theMesh,
                              const SMDSAbs_ElementType theType)
  : SMESHDS_GroupBase(theID,theMesh,theType),
    myGroup(theMesh,theType)
{
}

//=======================================================================
//function : Extent
//purpose  : 
//=======================================================================

smIdType SMESHDS_Group::Extent() const
{
  return myGroup.Extent();
}

//=======================================================================
//function : IsEmpty
//purpose  : 
//=======================================================================

bool SMESHDS_Group::IsEmpty()
{
  return myGroup.IsEmpty();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Contains (const smIdType theID)
{
  const SMDS_MeshElement* aElem = findInMesh (theID);
  if (aElem)
    return myGroup.Contains(aElem);
  return false;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

bool SMESHDS_Group::Contains (const SMDS_MeshElement* elem)
{
  if (elem)
    return myGroup.Contains(elem);
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Add (const smIdType theID)
{
  return Add( findInMesh( theID ));
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Add (const SMDS_MeshElement* aElem )
{
  bool added = false;
  if ( aElem )
  {
    added = myGroup.Add (aElem);
    if ( added )
      resetIterator();
  }
  return added;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESHDS_Group::Remove (const smIdType theID)
{
  bool removed = false;
  if ( const SMDS_MeshElement* aElem = findInMesh( theID ))
  {
    removed = myGroup.Remove (aElem);
    if ( removed )
      resetIterator();
  }
  return removed;
}


//======================================================================
//function : Clear
//purpose  : 
//=======================================================================

void SMESHDS_Group::Clear()
{
  myGroup.Clear();
  resetIterator();
}
  
//=======================================================================
//function : GetElements
//purpose  : 
//=======================================================================

SMDS_ElemIteratorPtr SMESHDS_Group::GetElements() const
{
  return myGroup.GetElements();
}

//================================================================================
/*!
 * \brief Return a value allowing to find out if a group has changed or not
 */
//================================================================================

int SMESHDS_Group::GetTic() const
{
  return myGroup.Tic();
}

//=======================================================================
//function : SetType
//purpose  : 
//=======================================================================

void SMESHDS_Group::SetType(SMDSAbs_ElementType theType)
{
  if ( myGroup.IsEmpty() || GetType() == SMDSAbs_All )
  {
    SMESHDS_GroupBase::SetType( theType );
    myGroup.SetType ( theType );
  }
  else
  {
    SMESHDS_GroupBase::SetType( myGroup.GetType() );
  }
}

