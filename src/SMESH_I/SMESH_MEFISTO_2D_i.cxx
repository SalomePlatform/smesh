using namespace std;
//=============================================================================
// File      : SMESH_MEFISTO_2D_i.cxx
// Created   : jeu mai 16 13:26:18 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_MEFISTO_2D_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MEFISTO_2D_i::SMESH_MEFISTO_2D_i(const char* anHyp,
				       int studyId,
				       ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_MEFISTO_2D_i::SMESH_MEFISTO_2D_i");
  _genImpl = genImpl;
  ::SMESH_MEFISTO_2D* impl 
      = new ::SMESH_MEFISTO_2D(_genImpl->_hypothesisFactory.GetANewId(),
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

SMESH_MEFISTO_2D_i::~SMESH_MEFISTO_2D_i()
{
  MESSAGE("SMESH_MEFISTO_2D_i::~SMESH_MEFISTO_2D_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MEFISTO_2D_i::SetImpl(::SMESH_MEFISTO_2D* impl)
{
  MESSAGE("SMESH_MEFISTO_2D_i::SetImpl");
  SMESH_2D_Algo_i::SetImpl(impl);
  _impl = impl;
}
