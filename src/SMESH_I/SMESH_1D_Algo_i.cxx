using namespace std;
//=============================================================================
// File      : SMESH_1D_Algo_i.cxx
// Created   : sam mai 18 09:23:13 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_1D_Algo_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_1D_Algo_i::SMESH_1D_Algo_i()
{
  MESSAGE("SMESH_1D_Algo_i::SMESH_1D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_1D_Algo_i::~SMESH_1D_Algo_i()
{
  MESSAGE("SMESH_1D_Algo_i::~SMESH_1D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_1D_Algo_i::SetImpl(::SMESH_1D_Algo* impl)
{
  MESSAGE("SMESH_1D_Algo_i::SetImpl");
  SMESH_Algo_i::SetImpl(impl);
  _impl = impl;
}
