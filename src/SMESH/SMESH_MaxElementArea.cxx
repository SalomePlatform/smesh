//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_MaxElementArea.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
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

