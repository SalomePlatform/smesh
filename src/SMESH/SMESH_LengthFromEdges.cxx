using namespace std;
//=============================================================================
// File      : SMESH_LengthFromEdges.cxx
// Created   : mar jun 11 22:42:30 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_LengthFromEdges.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LengthFromEdges::SMESH_LengthFromEdges(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _mode =1;
  _name = "LengthFromEdges";
//   SCRUTE(_name);
//   SCRUTE(&_name);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LengthFromEdges::~SMESH_LengthFromEdges()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_LengthFromEdges::SetMode(int mode)
  throw (SALOME_Exception)
{
  int oldMode = _mode;
  if (mode <= 0) 
    throw SALOME_Exception(LOCALIZED("mode must be positive"));
  _mode = mode;
  if (oldMode != _mode)
    NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_LengthFromEdges::GetMode()
{
  return _mode;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_LengthFromEdges::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_LengthFromEdges::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, SMESH_LengthFromEdges & hyp)
{
  save << hyp._mode;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, SMESH_LengthFromEdges & hyp)
{
  bool isOK = true;
  int a;
  isOK = (load >> a);
  if (isOK) hyp._mode = a;
  else load.clear(ios::badbit | load.rdstate());
  return load;
}

