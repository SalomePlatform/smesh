using namespace std;
//=============================================================================
// File      : SMESH_MaxElementArea.cxx
// Created   : sam mai 18 23:14:08 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_MaxElementArea.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MaxElementArea::SMESH_MaxElementArea(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _maxArea =1.;
  _name = "MaxElementArea";
//   SCRUTE(_name);
//   SCRUTE(&_name);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MaxElementArea::~SMESH_MaxElementArea()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MaxElementArea::SetMaxArea(double maxArea)
  throw (SALOME_Exception)
{
  double oldArea = _maxArea;
  if (maxArea <= 0) 
    throw SALOME_Exception(LOCALIZED("maxArea must be positive"));
  _maxArea = maxArea;
  if (_maxArea != oldArea)
    NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double SMESH_MaxElementArea::GetMaxArea()
{
  return _maxArea;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_MaxElementArea::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_MaxElementArea::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, SMESH_MaxElementArea & hyp)
{
  save << hyp._maxArea;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, SMESH_MaxElementArea & hyp)
{
  bool isOK = true;
  double a;
  isOK = (load >> a);
  if (isOK) hyp._maxArea = a;
  else load.clear(ios::badbit | load.rdstate());
  return load;
}

