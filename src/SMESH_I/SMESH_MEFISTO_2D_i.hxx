//=============================================================================
// File      : SMESH_MEFISTO_2D_i.hxx
// Created   : jeu mai 16 13:26:22 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_MEFISTO_2D_I_HXX_
#define _SMESH_MEFISTO_2D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_2D_Algo_i.hxx"

#include "SMESH_MEFISTO_2D.hxx"

class SMESH_MEFISTO_2D_i:
  public POA_SMESH::SMESH_MEFISTO_2D,
  public SMESH_2D_Algo_i
{
public:
  SMESH_MEFISTO_2D_i(const char* anHyp,
		     int studyId,
		     ::SMESH_Gen* genImpl);

  virtual ~SMESH_MEFISTO_2D_i();

protected:
  virtual void SetImpl(::SMESH_MEFISTO_2D* impl);

  ::SMESH_MEFISTO_2D* _impl;
};

#endif
