//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_subMesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_subMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"
#include "OpUtil.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::SMESH_subMesh_i()
{
  MESSAGE("SMESH_subMesh_i::SMESH_subMesh_i default, not for use");
    ASSERT(0);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::SMESH_subMesh_i(SMESH_Gen_i* gen_i,
				 SMESH_Mesh_i* mesh_i,
				 int localId)
{
  MESSAGE("SMESH_subMesh_i::SMESH_subMesh_i");
  _gen_i = gen_i;
  _mesh_i = mesh_i;
  _localId = localId;
  // ****
}
//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::~SMESH_subMesh_i()
{
  MESSAGE("SMESH_subMesh_i::~SMESH_subMesh_i");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_subMesh_i::GetNumberOfElements()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_subMesh_i::GetNumberOfElements");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_subMesh_i::GetNumberOfNodes()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_subMesh_i::GetNumberOfNodes");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_subMesh_i::GetElementsId()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_subMesh_i::GetElementsId");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_subMesh_i::GetNodesId()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_subMesh_i::GetNodesId");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::SMESH_Mesh_ptr SMESH_subMesh_i::GetFather()
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_subMesh_i::GetFather");
  SMESH::SMESH_Mesh_var meshIor = _mesh_i->GetIor();
  return SMESH::SMESH_Mesh::_duplicate(meshIor);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
CORBA::Long SMESH_subMesh_i::GetId()
{
  MESSAGE("SMESH_subMesh_i::GetId");
  return _localId;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SALOME_MED::FAMILY_ptr SMESH_subMesh_i::GetFamily()
  throw (SALOME::SALOME_Exception)
{
  SALOME_MED::MESH_var MEDMesh = GetFather()->GetMEDMesh();

  SALOME_MED::Family_array_var families = 
    MEDMesh->getFamilies(SALOME_MED::MED_NODE);
    
  for ( int i = 0; i < families->length(); i++ ) {
    if ( families[i]->getIdentifier() == ( _localId ) )
      return families[i];
  }
}
