using namespace std;
//=============================================================================
// File      : SMESH_NumberOfSegments.cxx
// Created   : sam mai 18 08:11:15 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_NumberOfSegments.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NumberOfSegments::SMESH_NumberOfSegments(int hypId, int studyId, SMESH_Gen* gen)
 : SMESH_Hypothesis(hypId, studyId, gen)
{
  _numberOfSegments = 1;
  _name = "NumberOfSegments";
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NumberOfSegments::~SMESH_NumberOfSegments()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_NumberOfSegments::SetNumberOfSegments(int segmentsNumber)
  throw (SALOME_Exception)
{
  int oldNumberOfSegments = _numberOfSegments;
  if (segmentsNumber <= 0) 
    throw SALOME_Exception(LOCALIZED("number of segments must be positive"));
  _numberOfSegments = segmentsNumber;

  if (oldNumberOfSegments != _numberOfSegments)
    NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_NumberOfSegments::GetNumberOfSegments()
{
  return _numberOfSegments;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_NumberOfSegments::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_NumberOfSegments::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream& operator << (ostream & save, SMESH_NumberOfSegments & hyp)
{
  save << hyp._numberOfSegments;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream& operator >> (istream & load, SMESH_NumberOfSegments & hyp)
{
  bool isOK = true;
  int a;
  isOK = (load >> a);
  if (isOK) hyp._numberOfSegments = a;
  else load.clear(ios::badbit | load.rdstate());
  return load;
}

