using namespace std;
//=============================================================================
// File      : SMESH_MaxElementArea_i.cxx
// Created   : sam mai 18 23:14:24 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_MaxElementArea_i.hxx"
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

SMESH_MaxElementArea_i::SMESH_MaxElementArea_i(const char* anHyp,
					       int studyId,
					       ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_MaxElementArea_i::SMESH_MaxElementArea_i");
  _impl = new ::SMESH_MaxElementArea(genImpl->_hypothesisFactory.GetANewId(),
				     studyId,
				     genImpl);
  _baseImpl = _impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MaxElementArea_i::~SMESH_MaxElementArea_i()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MaxElementArea_i::SetMaxElementArea(CORBA::Double area)
  throw (SALOME::SALOME_Exception)
{
  ASSERT(_impl);
  try
    {
      _impl->SetMaxArea(area);
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

CORBA::Double SMESH_MaxElementArea_i::GetMaxElementArea()
{
  ASSERT(_impl);
  return _impl->GetMaxArea();
}

