//=============================================================================
// File      : SMESH_Regular_1D_i.hxx
// Created   : jeu mai 16 13:25:50 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_REGULAR_1D_I_HXX_
#define _SMESH_REGULAR_1D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_1D_Algo_i.hxx"

#include "SMESH_Regular_1D.hxx"

class SMESH_Regular_1D_i:
  public POA_SMESH::SMESH_Regular_1D,
  public SMESH_1D_Algo_i
{
public:
  SMESH_Regular_1D_i(const char* anHyp,
		     int studyId,
		     ::SMESH_Gen* genImpl);

  virtual ~SMESH_Regular_1D_i();
 
protected:
  virtual void SetImpl(::SMESH_Regular_1D* impl);

  ::SMESH_Regular_1D* _impl;
};

#endif
