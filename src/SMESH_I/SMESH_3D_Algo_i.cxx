using namespace std;
//=============================================================================
// File      : SMESH_3D_Algo_i.cxx
// Created   : sam mai 18 09:25:00 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_3D_Algo_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_3D_Algo_i::SMESH_3D_Algo_i()
{
  MESSAGE("SMESH_3D_Algo_i::SMESH_3D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_3D_Algo_i::~SMESH_3D_Algo_i()
{
  MESSAGE("SMESH_3D_Algo_i::~SMESH_3D_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_3D_Algo_i::SetImpl(::SMESH_3D_Algo* impl)
{
  MESSAGE("SMESH_3D_Algo_i::SetImpl");
  SMESH_Algo_i::SetImpl(impl);
  _impl = impl;
}
