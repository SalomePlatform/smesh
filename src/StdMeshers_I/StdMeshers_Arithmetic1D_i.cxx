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
//  File   : StdMeshers_Arithmetic1D_i.cxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_Arithmetic1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

//=============================================================================
/*!
 *  StdMeshers_Arithmetic1D_i::StdMeshers_Arithmetic1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_Arithmetic1D_i::StdMeshers_Arithmetic1D_i( PortableServer::POA_ptr thePOA,
						    int                     theStudyId,
						    ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_Arithmetic1D_i::StdMeshers_Arithmetic1D_i" );
  myBaseImpl = new ::StdMeshers_Arithmetic1D( theGenImpl->GetANewId(),
					     theStudyId,
					     theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_Arithmetic1D_i::~StdMeshers_Arithmetic1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_Arithmetic1D_i::~StdMeshers_Arithmetic1D_i()
{
  MESSAGE( "StdMeshers_Arithmetic1D_i::~StdMeshers_Arithmetic1D_i" );
}

//=============================================================================
/*!
 *  StdMeshers_Arithmetic1D_i::SetLength
 *
 *  Set length
 */
//=============================================================================

void StdMeshers_Arithmetic1D_i::SetLength(CORBA::Double theLength,
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
  SMESH::TPythonDump() << _this() << ".SetLength( "
                       << theLength << ", " << theIsStart << " )";
}

//=============================================================================
/*!
 *  StdMeshers_Arithmetic1D_i::GetLength
 *
 *  Get length
 */
//=============================================================================

CORBA::Double StdMeshers_Arithmetic1D_i::GetLength( CORBA::Boolean theIsStart)
{
  MESSAGE( "StdMeshers_StartEndLength_i::GetLength" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetLength( theIsStart );
}

//=============================================================================
/*!
 *  StdMeshers_Arithmetic1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Arithmetic1D* StdMeshers_Arithmetic1D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Arithmetic1D_i::GetImpl" );
  return ( ::StdMeshers_Arithmetic1D* )myBaseImpl;
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
CORBA::Boolean StdMeshers_Arithmetic1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

