using namespace std;
//=============================================================================
// File      : SMESH_2D_Algo_i.cxx
// Created   : sam mai 18 09:23:51 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_2D_Algo_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_2D_Algo_i::SMESH_2D_Algo_i()
{
  MESSAGE("SMESH_2D_Algo_i::SMESH_2D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_2D_Algo_i::~SMESH_2D_Algo_i()
{
  MESSAGE("SMESH_2D_Algo_i::~SMESH_2D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_2D_Algo_i::SetImpl(::SMESH_2D_Algo* impl)
{
  MESSAGE("SMESH_2D_Algo_i::SetImpl");
  SMESH_Algo_i::SetImpl(impl);
  _impl = impl;
}
