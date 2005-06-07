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
//  File   : StdMeshers_NumberOfSegments_i.cxx
//           Moved here from SMESH_NumberOfSegments_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_NumberOfSegments_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::StdMeshers_NumberOfSegments_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_NumberOfSegments_i::StdMeshers_NumberOfSegments_i( PortableServer::POA_ptr thePOA,
						    int                     theStudyId,
						    ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::StdMeshers_NumberOfSegments_i" );
  myBaseImpl = new ::StdMeshers_NumberOfSegments( theGenImpl->GetANewId(),
					     theStudyId,
					     theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::~StdMeshers_NumberOfSegments_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_NumberOfSegments_i::~StdMeshers_NumberOfSegments_i()
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::~StdMeshers_NumberOfSegments_i" );
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::SetNumberOfSegments
 *
 *  Set number of segments
 */
//=============================================================================

void StdMeshers_NumberOfSegments_i::SetNumberOfSegments( CORBA::Long theSegmentsNumber )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::SetNumberOfSegments" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetNumberOfSegments( theSegmentsNumber );
  }
  catch (SALOME_Exception& S_ex) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
				  SALOME::BAD_PARAM );
  }

  // Update Python script
  TCollection_AsciiString aStr, aStrNb ((int)theSegmentsNumber);
  SMESH_Gen_i::AddObject(aStr, _this()) += ".SetNumberOfSegments(";
  aStr += aStrNb + ")";

  SMESH_Gen_i::AddToCurrentPyScript(aStr);
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::GetNumberOfSegments
 *
 *  Get number of segments
 */
//=============================================================================

CORBA::Long StdMeshers_NumberOfSegments_i::GetNumberOfSegments()
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::GetNumberOfSegments" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetNumberOfSegments();
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::SetScaleFactor
 *
 *  Set scalar factor
 */
//=============================================================================

void StdMeshers_NumberOfSegments_i::SetScaleFactor( CORBA::Double theScaleFactor )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::SetScaleFactor" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetScaleFactor( theScaleFactor );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
				  SALOME::BAD_PARAM );
  }
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::GetScaleFactor
 *
 *  Get scalar factor
 */
//=============================================================================

CORBA::Double StdMeshers_NumberOfSegments_i::GetScaleFactor()
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::GetScaleFactor" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetScaleFactor();
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfSegments_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_NumberOfSegments* StdMeshers_NumberOfSegments_i::GetImpl()
{
  MESSAGE( "StdMeshers_NumberOfSegments_i::GetImpl" );
  return ( ::StdMeshers_NumberOfSegments* )myBaseImpl;
}

