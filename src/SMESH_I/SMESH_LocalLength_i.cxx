using namespace std;
//=============================================================================
// File      : SMESH_LocalLength_i.cxx
// Created   : mar mai 14 13:07:42 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_LocalLength_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  Constructor: 
 *  _name is related to the class name: prefix = SMESH_ ; suffix = _i .
 */
//=============================================================================

SMESH_LocalLength_i::SMESH_LocalLength_i(const char* anHyp,
					 int studyId,
					 ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_LocalLength_i::SMESH_LocalLength_i");
  _impl = new ::SMESH_LocalLength(genImpl->_hypothesisFactory.GetANewId(),
				  studyId,
				  genImpl);
  _baseImpl = _impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LocalLength_i::~SMESH_LocalLength_i()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_LocalLength_i::SetLength(CORBA::Double length)
    throw (SALOME::SALOME_Exception)
{
  ASSERT(_impl);
  try
    {
      _impl->SetLength(length);
    }
  catch (SALOME_Exception& S_ex)
    {
      THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), \
				   SALOME::BAD_PARAM);
    }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Double SMESH_LocalLength_i::GetLength()
{
  ASSERT(_impl);
  return _impl->GetLength();
}

