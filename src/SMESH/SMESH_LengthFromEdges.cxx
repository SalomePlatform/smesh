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
//  File   : SMESH_LengthFromEdges.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
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

