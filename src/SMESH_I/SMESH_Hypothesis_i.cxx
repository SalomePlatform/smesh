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
//  File   : SMESH_Hypothesis_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Gen_i.hxx"

#include <utilities.h>
#include <SALOMEDS_wrap.hxx>

#include <iostream>
#include <sstream>

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
  myBaseImpl = 0;
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
  return myBaseImpl->GetID();
}

//=============================================================================
/*!
 *  SMESH_Hypothesis_i::IsPublished()
 *
 */
//=============================================================================
bool SMESH_Hypothesis_i::IsPublished()
{
  bool res = false;
  if ( SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen())
  {
    SALOMEDS::Study_var study = gen->GetCurrentStudy();
    SALOMEDS::SObject_wrap SO = SMESH_Gen_i::ObjectToSObject( study, _this());
    res = !SO->_is_nil();
  }
  return res;
}

//================================================================================
/*!
 * \brief Set the pramIndex-th parameter
 */
//================================================================================

void SMESH_Hypothesis_i::SetVarParameter (const char* theParameter,
                                          const char* theMethod)
{
  if ( SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen() )
  {
    SMESH::SMESH_Hypothesis_var varHolder;
    if ( myHolder->_is_nil() ) varHolder = _this();
    else                       varHolder = myHolder;
    gen->UpdateParameters( varHolder, theParameter );

    const std::vector< std::string >& pars = gen->GetLastParameters();
    if ( !pars.empty() )
      myMethod2VarParams[ theMethod ] = pars[0];
  }
}

//================================================================================
/*!
 * \brief Return the pramIndex-th variable parameter used for Hypothesis creation
 */
//================================================================================

char* SMESH_Hypothesis_i::GetVarParameter (const char* theMethod)
{
  if ( myMethod2VarParams.count("needs update by old study"))
  {
    // restore myMethod2VarParams by old study
    myMethod2VarParams.clear();
    if ( SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen() )
    {
      CORBA::String_var oldparVar = gen->GetParameters( _this() );
      setOldParameters( oldparVar.in() );
    }
  }
  std::map< std::string, std::string >::iterator meth_param = myMethod2VarParams.find( theMethod );
  if ( meth_param != myMethod2VarParams.end() )
    return CORBA::string_dup( meth_param->second.c_str() );

  return CORBA::string_dup("");
}

//================================================================================
/*!
 * \brief Store a hypothesis wrapping this not published one.
 *
 * This hyp, which has no own parameters but is published, is used to store variables
 * defining parameters of this hypothesis.
 */
//================================================================================

void SMESH_Hypothesis_i::SetHolderHypothesis(const SMESH::SMESH_Hypothesis_ptr hyp)
{
  myHolder = SMESH::SMESH_Hypothesis::_duplicate( hyp );
}

//================================================================================
/*!
 * \brief Restore myMethod2VarParams by parameters stored in an old study
 */
//================================================================================

void SMESH_Hypothesis_i::setOldParameters (const char* theParameters)
{
  if ( SMESH_Gen_i* gen = SMESH_Gen_i::GetSMESHGen() )
  {
    TCollection_AsciiString aOldParameters(theParameters);
    int pos = aOldParameters.SearchFromEnd("|");
    if ( pos >= 0 ) aOldParameters = aOldParameters.Split(pos);
    pos = aOldParameters.SearchFromEnd(";*=");
    if ( pos >= 0 ) aOldParameters.Split(pos-1);
    gen->UpdateParameters( CORBA::Object_var( _this() ).in(), aOldParameters.ToCString() );

    myMethod2VarParams.clear();
    const std::vector< std::string >& pars = gen->GetLastParameters();
    for ( size_t i = 0; i < pars.size(); ++i )
    {
      std::string meth = getMethodOfParameter( i, pars.size() );
      myMethod2VarParams[ meth ] = pars[i];
    }
    gen->UpdateParameters( CORBA::Object_var( _this() ).in(), "" ); // clear params
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
  return myBaseImpl;
}

//================================================================================
/*!
 * \brief Return true if a hypothesis has parameters
 */
//================================================================================

CORBA::Boolean SMESH_Hypothesis_i::HasParameters()
{
  std::ostringstream os;
  myBaseImpl->SaveTo( os );
  return ( !os.str().empty() );
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
  std::ostringstream os;

  // assure that parameters are loaded from an old study
  CORBA::String_var p = GetVarParameter("");

  os << "VARS " << myMethod2VarParams.size()  << " ";
  std::map< std::string, std::string >::iterator meth_param = myMethod2VarParams.begin();
  for ( ; meth_param != myMethod2VarParams.end(); ++meth_param )
    os << meth_param->first << " "
       << meth_param->second.size() << " "
       << meth_param->second << " ";

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
  std::istringstream is( theStream );
  if ( strncmp( theStream, "VARS", 4 ) == 0 )
  {
    int nbVars, len;
    char str[256];
    std::string meth;
    is >> str >> nbVars;
    for ( int i = 0; i < nbVars; ++i )
    {
      is >> meth >> len;
      if ( len < 256 )
      {
        is.get( str, len + 2 ); // 2 - to read at least 1 white space
        if ( len > 0 )
          myMethod2VarParams[ meth ] = std::string( str+1, len );
      }
    }
  }
  else
  {
    // we can't restore myMethod2VarParams by old study here because SObject
    // isn't yet bound to _this()
    myMethod2VarParams["needs update by old study"] = "yes";
  }

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
