//=============================================================================
// File      : SMESH_NETGEN_3D_i.hxx
// Created   : Jeudi 31 Janvier 2003
// Author    : Nadir Bouhamou CEA
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================
#ifndef _SMESH_NETGEN_3D_I_HXX_
#define _SMESH_NETGEN_3D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_3D_Algo_i.hxx"

#include "SMESH_NETGEN_3D.hxx"

class SMESH_NETGEN_3D_i:
  public POA_SMESH::SMESH_NETGEN_3D,
  public SMESH_3D_Algo_i
{
public:
  SMESH_NETGEN_3D_i(const char* anHyp,
		   int studyId,
		   ::SMESH_Gen* genImpl);

  virtual ~SMESH_NETGEN_3D_i();

protected:
  virtual void SetImpl(::SMESH_NETGEN_3D* impl);

  ::SMESH_NETGEN_3D* _impl;
};

#endif
