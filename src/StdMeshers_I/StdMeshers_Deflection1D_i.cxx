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
//  File   : StdMeshers_Deflection1D_i.cxx
//           Moved here from SMESH_LocalLength_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_Deflection1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i( PortableServer::POA_ptr thePOA,
					  int                     theStudyId,
					  ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i" );
  myBaseImpl = new ::StdMeshers_Deflection1D( theGenImpl->GetANewId(),
				        theStudyId,
				        theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i()
{
  MESSAGE( "StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i" );
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::SetDeflection
 *
 *  Set deflection
 */
//=============================================================================

void StdMeshers_Deflection1D_i::SetDeflection( CORBA::Double theValue )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_Deflection1D_i::SetDeflection" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetDeflection( theValue );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
				  SALOME::BAD_PARAM );
  }

  // Update Python script
  TCollection_AsciiString aStr, aStrVal ((double)theValue);
  SMESH_Gen_i::AddObject(aStr, _this()) += ".SetDeflection(";
  aStr += aStrVal + ")";

  SMESH_Gen_i::AddToCurrentPyScript(aStr);
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::GetDeflection
 *
 *  Get deflection
 */
//=============================================================================

CORBA::Double StdMeshers_Deflection1D_i::GetDeflection()
{
  MESSAGE( "StdMeshers_Deflection1D_i::GetDeflection" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetDeflection();
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Deflection1D* StdMeshers_Deflection1D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Deflection1D_i::GetImpl" );
  return ( ::StdMeshers_Deflection1D* )myBaseImpl;
}
