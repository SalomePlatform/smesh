//=============================================================================
// File      : SMESH_1D_Algo_i.hxx
// Created   : sam mai 18 09:23:09 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_1D_ALGO_I_HXX_
#define _SMESH_1D_ALGO_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Algo_i.hxx"

#include "SMESH_1D_Algo.hxx"

class SMESH_1D_Algo_i:
  public POA_SMESH::SMESH_1D_Algo,
  public SMESH_Algo_i
{
public:
  SMESH_1D_Algo_i();

  virtual ~SMESH_1D_Algo_i();

protected:
  virtual void SetImpl(::SMESH_1D_Algo* impl);

  ::SMESH_1D_Algo* _impl;
};

#endif
