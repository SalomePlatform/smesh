//=============================================================================
// File      : SMESH_3D_Algo_i.hxx
// Created   : sam mai 18 09:25:05 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_3D_ALGO_I_HXX_
#define _SMESH_3D_ALGO_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Algo_i.hxx"

class SMESH_3D_Algo;

class SMESH_3D_Algo_i:
  public POA_SMESH::SMESH_3D_Algo,
  public SMESH_Algo_i
{
public:
  SMESH_3D_Algo_i();

  virtual ~SMESH_3D_Algo_i();

protected:
  virtual void SetImpl(::SMESH_3D_Algo* impl);

  ::SMESH_3D_Algo* _impl;
};

#endif
