//=============================================================================
// File      : SMESH_Hexa_3D_i.hxx
// Created   : sam mai 18 23:15:38 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
#ifndef _SMESH_HEXA_3D_I_HXX_
#define _SMESH_HEXA_3D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_3D_Algo_i.hxx"

#include "SMESH_Hexa_3D.hxx"

class SMESH_Hexa_3D_i:
  public POA_SMESH::SMESH_Hexa_3D,
  public SMESH_3D_Algo_i
{
public:
  SMESH_Hexa_3D_i(const char* anHyp,
		  int studyId,
		  ::SMESH_Gen* genImpl);

  virtual ~SMESH_Hexa_3D_i();

protected:
  virtual void SetImpl(::SMESH_Hexa_3D* impl);

  ::SMESH_Hexa_3D* _impl;
};

#endif
