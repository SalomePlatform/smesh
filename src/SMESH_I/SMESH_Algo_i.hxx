//=============================================================================
// File      : SMESH_Algo_i.hxx
// Created   : sam mai 18 09:21:09 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_ALGO_I_HXX_
#define _SMESH_ALGO_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "SMESH_Algo.hxx"

class SMESH_Algo_i:
  public POA_SMESH::SMESH_Algo,
  public SMESH_Hypothesis_i
{
public:
  SMESH_Algo_i();

  virtual ~SMESH_Algo_i();

  SMESH::ListOfHypothesisName* GetCompatibleHypothesis();

protected:
  virtual void SetImpl(::SMESH_Algo* impl);

  ::SMESH_Algo* _impl;
};

#endif
