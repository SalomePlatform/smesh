using namespace std;
//=============================================================================
// File      : SMESH_subMesh_i.cxx
// Created   : jeu mai 30 10:01:12 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
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
