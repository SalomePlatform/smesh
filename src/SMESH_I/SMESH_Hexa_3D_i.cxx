using namespace std;
//=============================================================================
// File      : SMESH_Hexa_3D_i.cxx
// Created   : sam mai 18 23:15:45 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_Hexa_3D_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hexa_3D_i::SMESH_Hexa_3D_i(const char* anHyp,
				 int studyId,
				 ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_Hexa_3D_i::SMESH_Hexa_3D_i");
  _genImpl = genImpl;
  ::SMESH_Hexa_3D* impl 
      = new ::SMESH_Hexa_3D(_genImpl->_hypothesisFactory.GetANewId(),
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

SMESH_Hexa_3D_i::~SMESH_Hexa_3D_i()
{
  MESSAGE("SMESH_Hexa_3D_i::~SMESH_Hexa_3D_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Hexa_3D_i::SetImpl(::SMESH_Hexa_3D* impl)
{
  MESSAGE("SMESH_Hexa_3D_i::SetImpl");
  SMESH_3D_Algo_i::SetImpl(impl);
  _impl = impl;
}
