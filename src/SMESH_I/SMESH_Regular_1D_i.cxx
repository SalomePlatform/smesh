using namespace std;
//=============================================================================
// File      : SMESH_Regular_1D_i.cxx
// Created   : jeu mai 16 13:25:47 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_Regular_1D_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Regular_1D_i::SMESH_Regular_1D_i(const char* anHyp,
				       int studyId,
				       ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_Regular_1D_i::SMESH_Regular_1D_i");
  _genImpl = genImpl;
  ::SMESH_Regular_1D* impl 
      = new ::SMESH_Regular_1D(_genImpl->_hypothesisFactory.GetANewId(),
			       studyId,
			       genImpl);
  SetImpl(impl);
  _baseImpl = _impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Regular_1D_i::~SMESH_Regular_1D_i()
{
  MESSAGE("SMESH_Regular_1D_i::~SMESH_Regular_1D_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Regular_1D_i::SetImpl(::SMESH_Regular_1D* impl)
{
  MESSAGE("SMESH_Regular_1D_i::SetImpl");
  SMESH_1D_Algo_i::SetImpl(impl);
  _impl = impl;
}
