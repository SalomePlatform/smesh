using namespace std;
//=============================================================================
// File      : SMESHDS_Hypothesis.cxx
// Created   : sam mai 18 08:08:50 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESHDS_Hypothesis.hxx"


//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESHDS_Hypothesis::SMESHDS_Hypothesis(int hypId)
{
//   MESSAGE("SMESHDS_Hypothesis::SMESHDS_Hypothesis");
  _hypId = hypId;
  _name = "generic";
//   SCRUTE(_name);
//   SCRUTE(_hypId);
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESHDS_Hypothesis::~SMESHDS_Hypothesis()
{
//   MESSAGE("SMESHDS_Hypothesis::~SMESHDS_Hypothesis");
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const char* SMESHDS_Hypothesis::GetName()
{
//   MESSAGE("SMESHDS_Hypothesis::GetName");
//   SCRUTE(_name);
//   SCRUTE(&_name);
  return _name.c_str();
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESHDS_Hypothesis::GetID()
{
//   MESSAGE("SMESHDS_Hypothesis::GetId");
//   SCRUTE(_hypId);
  return _hypId;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESHDS_Hypothesis::GetType()
{
//   MESSAGE("SMESHDS_Hypothesis::GetType");
//   SCRUTE(_type);
  return _type;
}

