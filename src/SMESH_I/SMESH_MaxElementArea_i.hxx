//=============================================================================
// File      : SMESH_MaxElementArea_i.hxx
// Created   : sam mai 18 23:14:29 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_MAXELEMENTAREA_I_HXX_
#define _SMESH_MAXELEMENTAREA_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "SMESH_MaxElementArea.hxx"

class SMESH_MaxElementArea_i:
  public POA_SMESH::SMESH_MaxElementArea,
  public SMESH_Hypothesis_i
{
public:
  SMESH_MaxElementArea_i(const char* anHyp,
			 int studyId,
			 ::SMESH_Gen* genImpl);
  virtual ~SMESH_MaxElementArea_i();

  void SetMaxElementArea(CORBA::Double area)
    throw (SALOME::SALOME_Exception);

  CORBA::Double GetMaxElementArea();

protected:
  ::SMESH_MaxElementArea* _impl;
};

#endif
