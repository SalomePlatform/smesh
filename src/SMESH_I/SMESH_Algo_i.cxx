using namespace std;
//=============================================================================
// File      : SMESH_Algo_i.cxx
// Created   : sam mai 18 09:21:02 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_Algo_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <string>
#include <vector>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo_i::SMESH_Algo_i()
{
  MESSAGE("SMESH_Algo_i::SMESH_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo_i::~SMESH_Algo_i()
{
  MESSAGE("SMESH_Algo_i::~SMESH_Algo_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::ListOfHypothesisName* SMESH_Algo_i::GetCompatibleHypothesis()
{
  MESSAGE("SMESH_Algo_i::GetCompatibleHypothesis");
  SMESH::ListOfHypothesisName_var listOfHypothesis 
    = new SMESH::ListOfHypothesisName;
  const vector<string> & hypList = _impl->GetCompatibleHypothesis();
  int nbHyp = hypList.size();
  listOfHypothesis->length(nbHyp);
  for (int i=0; i<nbHyp; i++)
    {
      listOfHypothesis[i] = hypList[i].c_str();
    }
  return listOfHypothesis._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Algo_i::SetImpl(::SMESH_Algo* impl)
{
  MESSAGE("SMESH_Algo_i::SetImpl");
  //SMESH_Algo_i::SetImpl(impl);
  _impl = impl;
}
