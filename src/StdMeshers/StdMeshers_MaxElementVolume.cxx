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
//  File   : StdMeshers_MaxElementVolume.cxx
//           Moved here from SMESH_MaxElementVolume.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;

#include "StdMeshers_MaxElementVolume.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_MaxElementVolume::StdMeshers_MaxElementVolume(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _maxVolume =1.;
  _name = "MaxElementVolume";
//   SCRUTE(_name);
  SCRUTE(&_name);
  _param_algo_dim = 3;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_MaxElementVolume::~StdMeshers_MaxElementVolume()
{
  MESSAGE("StdMeshers_MaxElementVolume::~StdMeshers_MaxElementVolume");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_MaxElementVolume::SetMaxVolume(double maxVolume)
  throw (SALOME_Exception)
{
  double oldVolume = _maxVolume;
  if (maxVolume <= 0) 
    throw SALOME_Exception(LOCALIZED("maxVolume must be positive"));
  _maxVolume = maxVolume;
  if (_maxVolume != oldVolume)
    NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double StdMeshers_MaxElementVolume::GetMaxVolume() const
{
  return _maxVolume;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_MaxElementVolume::SaveTo(ostream & save)
{
  save << this->_maxVolume;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_MaxElementVolume::LoadFrom(istream & load)
{
  bool isOK = true;
  double a;
  isOK = (load >> a);
  if (isOK)
    this->_maxVolume = a;
  else 
    load.clear(ios::badbit | load.rdstate());
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, StdMeshers_MaxElementVolume & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, StdMeshers_MaxElementVolume & hyp)
{
  return hyp.LoadFrom( load );
}

