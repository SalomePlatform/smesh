//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_Hypothesis_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#include <iostream>
#include <sstream>
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "utilities.h"

using namespace std;

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
  
  MESSAGE( "SMESH_Hypothesis_i::SMESH_Hypothesis_i / Fin" );
};

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
 *  SMESH_Hypothesis_i::IsPublished()
 *
 */
//=============================================================================
bool SMESH_Hypothesis_i::IsPublished(){
  bool res = false;
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  if(gen){
    SALOMEDS::SObject_var SO = 
      SMESH_Gen_i::ObjectToSObject(gen->GetCurrentStudy() , SMESH::SMESH_Hypothesis::_narrow(_this()));
    res = !SO->_is_nil();
  }
  return res;
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::SetParameters()
 *
 */
//=============================================================================
void SMESH_Hypothesis_i::SetParameters(const char* theParameters)
{
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  char * aParameters = CORBA::string_dup(theParameters);
  if(gen){
    if(IsPublished()) {
      SMESH_Gen_i::GetSMESHGen()->UpdateParameters(SMESH::SMESH_Hypothesis::_narrow(_this()),aParameters);
    }
    else {
      myBaseImpl->SetParameters(gen->ParseParameters(aParameters));
    }
  }
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetParameters()
 *
 */
//=============================================================================
char* SMESH_Hypothesis_i::GetParameters()
{
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  char* aResult;
  if(IsPublished()) {
    MESSAGE("SMESH_Hypothesis_i::GetParameters() : Get Parameters from SObject");
    aResult = gen->GetParameters(SMESH::SMESH_Hypothesis::_narrow(_this()));
  }
  else {
    MESSAGE("SMESH_Hypothesis_i::GetParameters() : Get local parameters");
    aResult = myBaseImpl->GetParameters(); 
  }
  return CORBA::string_dup(aResult);
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::GetLastParameters()
 *
 */
//=============================================================================
SMESH::ListOfParameters* SMESH_Hypothesis_i::GetLastParameters()
{
  SMESH::ListOfParameters_var aResult = new SMESH::ListOfParameters();
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  if(gen) {
    char *aParameters;
    if(IsPublished())
     aParameters = GetParameters();
    else
      aParameters = myBaseImpl->GetLastParameters();

    SALOMEDS::Study_ptr aStudy = gen->GetCurrentStudy();
    if(!aStudy->_is_nil()) {
      SALOMEDS::ListOfListOfStrings_var aSections = aStudy->ParseVariables(aParameters); 
      if(aSections->length() > 0) {
        SALOMEDS::ListOfStrings aVars = aSections[aSections->length()-1];
        aResult->length(aVars.length());
        for(int i = 0;i < aVars.length();i++)
          aResult[i] = CORBA::string_dup( aVars[i]);
      }
    }
  }
  return aResult._retn();
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::SetLastParameters()
 *
 */
//=============================================================================
void SMESH_Hypothesis_i::SetLastParameters(const char* theParameters)
{
  if(!IsPublished()) {
    myBaseImpl->SetLastParameters(theParameters);
  }
}
//=============================================================================
/*!
 *  SMESH_Hypothesis_i::ClearParameters()
 *
 */
//=============================================================================
void SMESH_Hypothesis_i::ClearParameters()
{
  if(!IsPublished()) {
    myBaseImpl->ClearParameters();
  }
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
  //MESSAGE( "SMESH_Hypothesis_i::GetImpl" );
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
  // let listeners know about loading (issue 0020918)
  myBaseImpl->NotifySubMeshesHypothesisModification();
}

//================================================================================
/*!
 * \brief This mesthod is called after completion of loading a study 
 */
//================================================================================

void SMESH_Hypothesis_i::UpdateAsMeshesRestored()
{
  // for hyps needing full data restored
}
