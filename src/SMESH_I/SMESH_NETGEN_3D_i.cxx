//=============================================================================
// File      : SMESH_NETGEN_3D_i.cxx
// Created   : Jeudi 31 Janvier 2003
// Author    : Nadir Bouhamou CEA
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_NETGEN_3D_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NETGEN_3D_i::SMESH_NETGEN_3D_i(const char* anHyp,
				   int studyId,
				   ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_NETGEN_3D_i::SMESH_NETGEN_3D_i");
  _genImpl = genImpl;
  ::SMESH_NETGEN_3D* impl 
      = new ::SMESH_NETGEN_3D(_genImpl->_hypothesisFactory.GetANewId(),
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

SMESH_NETGEN_3D_i::~SMESH_NETGEN_3D_i()
{
  MESSAGE("SMESH_NETGEN_3D_i::~SMESH_NETGEN_3D_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_NETGEN_3D_i::SetImpl(::SMESH_NETGEN_3D* impl)
{
  MESSAGE("SMESH_NETGEN_3D_i::SetImpl");
  SMESH_3D_Algo_i::SetImpl(impl);
  _impl = impl;
}
