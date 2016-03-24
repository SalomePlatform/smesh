// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : StdMeshers_StartEndLength_i.cxx
//           Moved here from SMESH_LocalLength_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "StdMeshers_StartEndLength_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::StdMeshers_StartEndLength_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_StartEndLength_i::StdMeshers_StartEndLength_i( PortableServer::POA_ptr thePOA,
                                                         int                     theStudyId,
                                                         ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_StartEndLength_i::StdMeshers_StartEndLength_i" );
  myBaseImpl = new ::StdMeshers_StartEndLength( theGenImpl->GetANewId(),
                                               theStudyId,
                                               theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::~StdMeshers_StartEndLength_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_StartEndLength_i::~StdMeshers_StartEndLength_i()
{
  MESSAGE( "StdMeshers_StartEndLength_i::~StdMeshers_StartEndLength_i" );
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::SetLength
 *
 *  Set length
 */
//=============================================================================

void StdMeshers_StartEndLength_i::SetLength(CORBA::Double theLength,
                                            CORBA::Boolean theIsStart )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_StartEndLength_i::SetLength" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetLength( theLength, theIsStart );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() <<
    _this() << ( theIsStart ? ".SetStartLength( " : ".SetEndLength( " ) <<
    SMESH::TVar(theLength) << " )";
}

//=============================================================================
/*!
 * Sets <start segment length> parameter value
 */
//=============================================================================

void StdMeshers_StartEndLength_i::SetStartLength( CORBA::Double length)
  throw (SALOME::SALOME_Exception)
{
  SetLength( length, true );
}

//=============================================================================
/*!
 * Sets <end segment length> parameter value
 */
//=============================================================================

void StdMeshers_StartEndLength_i::SetEndLength( CORBA::Double length)
  throw (SALOME::SALOME_Exception)
{
  SetLength( length, false );
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::GetLength
 *
 *  Get length
 */
//=============================================================================

CORBA::Double StdMeshers_StartEndLength_i::GetLength( CORBA::Boolean theIsStart)
{
  MESSAGE( "StdMeshers_StartEndLength_i::GetLength" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetLength( theIsStart );
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::SetReversedEdges
 *
 *  Set edges to reverse
 */
//=============================================================================

void StdMeshers_StartEndLength_i::SetReversedEdges( const SMESH::long_array& theIds )
{
  ASSERT( myBaseImpl );
  try {
    std::vector<int> ids( theIds.length() );
    CORBA::Long iEnd = theIds.length();
    for ( CORBA::Long i = 0; i < iEnd; i++ )
      ids[ i ] = theIds[ i ];

    this->GetImpl()->SetReversedEdges( ids );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetReversedEdges( " << theIds << " )";
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::SetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

void StdMeshers_StartEndLength_i::SetObjectEntry( const char* theEntry )
{
  ASSERT( myBaseImpl );
  string entry(theEntry); // actually needed as theEntry is spoiled by moment of dumping
  try {
    this->GetImpl()->SetObjectEntry( entry.c_str() );
    // Update Python script
    SMESH::TPythonDump() << _this() << ".SetObjectEntry( '" << entry.c_str() << "' )";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),SALOME::BAD_PARAM );
  }
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::GetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

char* StdMeshers_StartEndLength_i::GetObjectEntry()
{
  ASSERT( myBaseImpl );
  const char* entry;
  try {
    entry = this->GetImpl()->GetObjectEntry();
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  return CORBA::string_dup( entry );
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::GetReversedEdges
 *
 *  Get reversed edges
 */
//=============================================================================

SMESH::long_array* StdMeshers_StartEndLength_i::GetReversedEdges()
{
  ASSERT( myBaseImpl );
  SMESH::long_array_var anArray = new SMESH::long_array;
  std::vector<int> ids = this->GetImpl()->GetReversedEdges();
  anArray->length( ids.size() );
  for ( CORBA::ULong i = 0; i < ids.size(); i++)
    anArray [ i ] = ids [ i ];

  return anArray._retn();
}

//=============================================================================
/*!
 *  StdMeshers_StartEndLength_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_StartEndLength* StdMeshers_StartEndLength_i::GetImpl()
{
  MESSAGE( "StdMeshers_StartEndLength_i::GetImpl" );
  return ( ::StdMeshers_StartEndLength* )myBaseImpl;
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
CORBA::Boolean StdMeshers_StartEndLength_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_StartEndLength_i::getMethodOfParameter(const int paramIndex,
                                                              int       /*nbVars*/) const
{
  return paramIndex == 0 ? "SetStartLength" : "SetEndLength";
}
