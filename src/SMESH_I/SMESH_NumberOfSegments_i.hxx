//=============================================================================
// File      : SMESH_NumberOfSegments_i.hxx
// Created   : jeu mai 16 13:09:38 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_NUMBEROFSEGMENTS_I_HXX_
#define _SMESH_NUMBEROFSEGMENTS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "SMESH_NumberOfSegments.hxx"

class SMESH_NumberOfSegments_i:
  public POA_SMESH::SMESH_NumberOfSegments,
  public SMESH_Hypothesis_i
{
public:
  SMESH_NumberOfSegments_i(const char* anHyp,
			   int studyId,
			   ::SMESH_Gen* genImpl);
  virtual ~SMESH_NumberOfSegments_i();

  void SetNumberOfSegments(CORBA::Long segmentsNumber)
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetNumberOfSegments();

protected:
  ::SMESH_NumberOfSegments* _impl;
};

#endif
