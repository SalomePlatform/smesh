//=============================================================================
// File      : SMESH_Hypothesis_i.hxx
// Created   : lun mai  6 13:42:08 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_HYPOTHESIS_I_HXX_
#define _SMESH_HYPOTHESIS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

class SMESH_Gen;
class SMESH_Hypothesis;

class SMESH_Hypothesis_i:
  public POA_SMESH::SMESH_Hypothesis
{
public:
  SMESH_Hypothesis_i();
  virtual ~SMESH_Hypothesis_i();

  char* GetName();
  CORBA::Long GetId();

protected:
  ::SMESH_Hypothesis* _baseImpl;
  ::SMESH_Gen* _genImpl;
  int _id;
};

#endif
