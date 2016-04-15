// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : StdMeshers_MaxLength_i.cxx
//  Module : SMESH
//
#include "StdMeshers_MaxLength_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::StdMeshers_MaxLength_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_MaxLength_i::StdMeshers_MaxLength_i( PortableServer::POA_ptr thePOA,
                                                int                     theStudyId,
                                                ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),
    SMESH_Hypothesis_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_MaxLength( theGenImpl->GetANewId(),
                                           theStudyId,
                                           theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::~StdMeshers_MaxLength_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_MaxLength_i::~StdMeshers_MaxLength_i()
{
}

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::SetLength
 *
 *  Set length
 */
//=============================================================================
void StdMeshers_MaxLength_i::SetLength( CORBA::Double theLength )
     throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetLength( theLength );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetLength( " << SMESH::TVar(theLength) << " )";
}

//=============================================================================
/*!
 *  Sets preestimation flag
 */
//=============================================================================
void StdMeshers_MaxLength_i::SetUsePreestimatedLength( CORBA::Boolean toUse )
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetUsePreestimatedLength( toUse );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // this is an internal kitchen call - no Python dump
  // Update Python script
  //SMESH::TPythonDump() << _this() << ".SetUsePreestimatedLength( " << toUse << " )";
}

//=============================================================================
/*!
 *  Sets preestimation length
 */
//=============================================================================
void StdMeshers_MaxLength_i::SetPreestimatedLength( CORBA::Double theLength )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetPreestimatedLength( theLength );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }
  // this is an internal kitchen call - no Python dump
  // Update Python script
  //SMESH::TPythonDump() << _this() << ".SetPreestimatedLength( " << toUse << " )";
}

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::GetLength
 *
 *  Get length
 */
//=============================================================================
CORBA::Double StdMeshers_MaxLength_i::GetLength()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetLength();
}

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::GetPreestimatedLength
 */
//=============================================================================
CORBA::Double StdMeshers_MaxLength_i::GetPreestimatedLength()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetPreestimatedLength();
}

//=============================================================================
/*!
 *  Returns preestimation flag
 */
//=============================================================================
CORBA::Boolean StdMeshers_MaxLength_i::GetUsePreestimatedLength()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetUsePreestimatedLength();
}

//================================================================================
/*!
 * \brief Returns true if preestemated length is defined
 */
//================================================================================

CORBA::Boolean StdMeshers_MaxLength_i::HavePreestimatedLength()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->HavePreestimatedLength();
}

//=============================================================================
/*!
 *  StdMeshers_MaxLength_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================
::StdMeshers_MaxLength* StdMeshers_MaxLength_i::GetImpl()
{
  return ( ::StdMeshers_MaxLength* )myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  
CORBA::Boolean StdMeshers_MaxLength_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_MaxLength_i::getMethodOfParameter(const int, int) const
{
  return "SetLength";
}
