using namespace std;
//=============================================================================
// File      : SMESH_NumberOfSegments_i.cxx
// Created   : jeu mai 16 13:09:33 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_NumberOfSegments_i.hxx"
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

SMESH_NumberOfSegments_i::SMESH_NumberOfSegments_i(const char* anHyp,
						   int studyId,
						   ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_NumberOfSegments_i::SMESH_NumberOfSegments_i");
  _impl= new ::SMESH_NumberOfSegments(genImpl->_hypothesisFactory.GetANewId(),
				      studyId,
				      genImpl);
  _baseImpl = _impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NumberOfSegments_i::~SMESH_NumberOfSegments_i()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void 
SMESH_NumberOfSegments_i::SetNumberOfSegments(CORBA::Long segmentsNumber)
  throw (SALOME::SALOME_Exception)
{
  ASSERT(_impl);
  try
    {
      _impl->SetNumberOfSegments(segmentsNumber);
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

CORBA::Long SMESH_NumberOfSegments_i::GetNumberOfSegments()
{
  ASSERT(_impl);
  return _impl->GetNumberOfSegments();
}

