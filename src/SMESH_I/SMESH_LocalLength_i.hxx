//=============================================================================
// File      : SMESH_LocalLength_i.hxx
// Created   : mar mai 14 13:07:47 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_LOCALLENGTH_I_HXX_
#define _SMESH_LOCALLENGTH_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "SMESH_LocalLength.hxx"

class SMESH_LocalLength_i:
  public POA_SMESH::SMESH_LocalLength,
  public SMESH_Hypothesis_i
{
public:
  SMESH_LocalLength_i(const char* anHyp,
		      int studyId,
		      ::SMESH_Gen* genImpl);
  virtual ~SMESH_LocalLength_i();

  void SetLength(CORBA::Double length)
    throw (SALOME::SALOME_Exception);

  CORBA::Double GetLength();

protected:
  ::SMESH_LocalLength* _impl;
};

#endif

