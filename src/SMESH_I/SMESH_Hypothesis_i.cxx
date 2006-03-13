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
//  File   : SMESH_Hypothesis_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include <iostream>
#include <sstream>
#include "SMESH_Hypothesis_i.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::SMESH_Hypothesis_i
 * 
 *  Constructor
 */
//=============================================================================

SMESH_Hypothesis_i::SMESH_Hypothesis_i( PortableServer::POA_ptr thePOA )
     : SALOME::GenericObj_i( thePOA )
{
  MESSAGE( "SMESH_Hypothesis_i::SMESH_Hypothesis_i / Début" );
  myBaseImpl = 0;
  // _CS_gbo This instruction fails
  // thePOA->activate_object( this );
  // _CS_gbo I keep the POA reference to activate the object in the future.
  myPOA = thePOA;
  
  MESSAGE( "SMESH_Hypothesis_i::SMESH_Hypothesis_i / Fin" );
};

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::Activate()
 *
 *  Activation of the object
 */
//=============================================================================
void SMESH_Hypothesis_i::Activate() {
  myPOA->activate_object( this );
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::~SMESH_Hypothesis_i
 *
 *  Destructor
 */
//=============================================================================

SMESH_Hypothesis_i::~SMESH_Hypothesis_i()
{
  MESSAGE( "SMESH_Hypothesis_i::~SMESH_Hypothesis_i" );
  if ( myBaseImpl )
    delete myBaseImpl;
};

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetName
 *
 *  Get type name of hypothesis
 */
//=============================================================================

char* SMESH_Hypothesis_i::GetName()
{
  //MESSAGE( "SMESH_Hypothesis_i::GetName" );
  return CORBA::string_dup( myBaseImpl->GetName() );
};

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetLibName
 *
 *  Get plugin library name of hypothesis (required by persistency mechanism)
 */
//=============================================================================

char* SMESH_Hypothesis_i::GetLibName()
{
  MESSAGE( "SMESH_Hypothesis_i::GetLibName" );
  return CORBA::string_dup( myBaseImpl->GetLibName() );
};

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::SetLibName
 *
 *  Set plugin library name of hypothesis (required by persistency mechanism)
 */
//=============================================================================

void SMESH_Hypothesis_i::SetLibName(const char* theLibName)
{
  MESSAGE( "SMESH_Hypothesis_i::SetLibName" );
  myBaseImpl->SetLibName( theLibName );
};

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetId
 *
 *  Get unique id of hypothesis
 */
//=============================================================================

CORBA::Long SMESH_Hypothesis_i::GetId()
{
  MESSAGE( "SMESH_Hypothesis_i::GetId" );
  return myBaseImpl->GetID();
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::SMESH_Hypothesis* SMESH_Hypothesis_i::GetImpl()
{
  MESSAGE( "SMESH_Hypothesis_i::GetImpl" );
  return myBaseImpl;
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::SaveTo
 *
 *  Persistence: Dumps parameters to the string stream
 */
//=============================================================================

char* SMESH_Hypothesis_i::SaveTo()
{
  MESSAGE( "SMESH_Hypothesis_i::SaveTo" );
  std::ostringstream os;
  myBaseImpl->SaveTo( os );
  return CORBA::string_dup( os.str().c_str() );
}

//=============================================================================
/*!
*  SMESH_Hypothesis_i::LoadFrom
*
*  Persistence: Restores parameters from string
*/
//=============================================================================

void SMESH_Hypothesis_i::LoadFrom( const char* theStream )
{
  MESSAGE( "SMESH_Hypothesis_i::LoadFrom" );
  std::istringstream is( theStream );
  myBaseImpl->LoadFrom( is );
}
