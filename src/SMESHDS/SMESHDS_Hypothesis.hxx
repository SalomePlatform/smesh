//=============================================================================
// File      : SMESHDS_Hypothesis.hxx
// Created   : sam mai 18 08:07:54 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESHDS_HYPOTHESIS_HXX_
#define _SMESHDS_HYPOTHESIS_HXX_

#if (__GNUC__>2)
#include <string>
#include <istream>
#include <ostream>
#else 
#include <string>
#include <istream.h>
#include <ostream.h>
#endif
  
// class istream;
// class ostream;

class SMESHDS_Hypothesis
{
public:
  SMESHDS_Hypothesis(int hypId);
  virtual ~SMESHDS_Hypothesis();

  const char* GetName();
  int GetID();
  int GetType();

  virtual ostream & SaveTo(ostream & save)=0;
  virtual istream & LoadFrom(istream & load)=0;

enum hypothesis_type {PARAM_ALGO, ALGO, ALGO_1D, ALGO_2D, ALGO_3D};

protected:
  string _name;
  int _hypId;
  int _type;
};

#endif
