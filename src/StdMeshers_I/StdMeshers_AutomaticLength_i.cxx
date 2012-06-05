// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_AutomaticLength_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$
//
#include "StdMeshers_AutomaticLength_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_AutomaticLength_i::StdMeshers_AutomaticLength_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_AutomaticLength_i::StdMeshers_AutomaticLength_i( PortableServer::POA_ptr thePOA,
                                                            int                     theStudyId,
                                                            ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_AutomaticLength_i::StdMeshers_AutomaticLength_i" );
  myBaseImpl = new ::StdMeshers_AutomaticLength( theGenImpl->GetANewId(),
                                                 theStudyId,
                                                 theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_AutomaticLength_i::~StdMeshers_AutomaticLength_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_AutomaticLength_i::~StdMeshers_AutomaticLength_i()
{
  MESSAGE( "StdMeshers_AutomaticLength_i::~StdMeshers_AutomaticLength_i" );
}

//=============================================================================
/*!
 *  StdMeshers_AutomaticLength_i::SetFineness
 *
 *  Set Fineness
 */
//=============================================================================

void StdMeshers_AutomaticLength_i::SetFineness( CORBA::Double theFineness )
     throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetFineness( theFineness );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetFineness( " << theFineness << " )";
}

//=============================================================================
/*!
 *  StdMeshers_AutomaticLength_i::GetFineness
 *
 *  Get Fineness
 */
//=============================================================================

CORBA::Double StdMeshers_AutomaticLength_i::GetFineness()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetFineness();
}

//=============================================================================
/*!
 *  StdMeshers_AutomaticLength_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_AutomaticLength* StdMeshers_AutomaticLength_i::GetImpl()
{
  return ( ::StdMeshers_AutomaticLength* )myBaseImpl;
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
CORBA::Boolean StdMeshers_AutomaticLength_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

