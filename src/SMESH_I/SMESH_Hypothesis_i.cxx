using namespace std;
//=============================================================================
// File      : SMESH_Hypothesis_i.cxx
// Created   : lun mai  6 13:42:11 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Gen.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis_i::SMESH_Hypothesis_i()
{
  MESSAGE("SMESH_Hypothesis_i");
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis_i::~SMESH_Hypothesis_i()
{
  MESSAGE("~SMESH_Hypothesis_i");
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

char* SMESH_Hypothesis_i::GetName()
{
  MESSAGE("GetName");
  return CORBA::string_dup(_baseImpl->GetName());
//   const char* name = _baseImpl->GetName();
//   SCRUTE(name);
//   return CORBA::string_dup(name);
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Hypothesis_i::GetId()
{
  MESSAGE("GetId");
  return _baseImpl->GetID();
}
