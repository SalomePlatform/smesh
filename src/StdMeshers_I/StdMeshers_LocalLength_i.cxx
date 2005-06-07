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
//  File   : StdMeshers_LocalLength_i.cxx
//           Moved here from SMESH_LocalLength_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_LocalLength_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

//=============================================================================
/*!
 *  StdMeshers_LocalLength_i::StdMeshers_LocalLength_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_LocalLength_i::StdMeshers_LocalLength_i( PortableServer::POA_ptr thePOA,
					  int                     theStudyId,
					  ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_LocalLength_i::StdMeshers_LocalLength_i" );
  myBaseImpl = new ::StdMeshers_LocalLength( theGenImpl->GetANewId(),
				        theStudyId,
				        theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_LocalLength_i::~StdMeshers_LocalLength_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_LocalLength_i::~StdMeshers_LocalLength_i()
{
  MESSAGE( "StdMeshers_LocalLength_i::~StdMeshers_LocalLength_i" );
}

//=============================================================================
/*!
 *  StdMeshers_LocalLength_i::SetLength
 *
 *  Set length
 */
//=============================================================================

void StdMeshers_LocalLength_i::SetLength( CORBA::Double theLength )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_LocalLength_i::SetLength" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetLength( theLength );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
				  SALOME::BAD_PARAM );
  }

  // Update Python script
  TCollection_AsciiString aStr, aStrLen ((double)theLength);
  SMESH_Gen_i::AddObject(aStr, _this()) += ".SetLength(";
  aStr += aStrLen + ")";

  SMESH_Gen_i::AddToCurrentPyScript(aStr);
}

//=============================================================================
/*!
 *  StdMeshers_LocalLength_i::GetLength
 *
 *  Get length
 */
//=============================================================================

CORBA::Double StdMeshers_LocalLength_i::GetLength()
{
  MESSAGE( "StdMeshers_LocalLength_i::GetLength" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetLength();
}

//=============================================================================
/*!
 *  StdMeshers_LocalLength_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_LocalLength* StdMeshers_LocalLength_i::GetImpl()
{
  MESSAGE( "StdMeshers_LocalLength_i::GetImpl" );
  return ( ::StdMeshers_LocalLength* )myBaseImpl;
}
