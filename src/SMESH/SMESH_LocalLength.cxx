using namespace std;
//=============================================================================
// File      : SMESH_LocalLength.cxx
// Created   : sam mai 18 08:10:23 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_LocalLength.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LocalLength::SMESH_LocalLength(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _length =1.;
  _name = "LocalLength";
//   SCRUTE(_name);
//   SCRUTE(&_name);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LocalLength::~SMESH_LocalLength()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_LocalLength::SetLength(double length)
  throw (SALOME_Exception)
{
  double oldLength = _length;
  if (length <= 0) 
    throw SALOME_Exception(LOCALIZED("length must be positive"));
  _length = length;
  if (oldLength != _length)
    NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double SMESH_LocalLength::GetLength()
{
  return _length;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_LocalLength::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_LocalLength::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, SMESH_LocalLength & hyp)
{
  save << hyp._length;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, SMESH_LocalLength & hyp)
{
  bool isOK = true;
  double a;
  isOK = (load >> a);
  if (isOK) hyp._length = a;
  else load.clear(ios::badbit | load.rdstate());
  return load;
}

