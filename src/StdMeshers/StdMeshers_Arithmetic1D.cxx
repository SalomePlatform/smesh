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
//  File   : StdMeshers_Arithmetic1D.cxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_Arithmetic1D.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Arithmetic1D::StdMeshers_Arithmetic1D(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_Hypothesis(hypId, studyId, gen)
{
  _begLength = 1.;
  _endLength = 1.;
  _name = "Arithmetic1D";
  _param_algo_dim = 1; 
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Arithmetic1D::~StdMeshers_Arithmetic1D()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Arithmetic1D::SetLength(double length, bool isStartLength)
     throw(SALOME_Exception)
{
  if ( (isStartLength ? _begLength : _endLength) != length ) {
    if (length <= 0)
      throw SALOME_Exception(LOCALIZED("length must be positive"));
    if ( isStartLength )
      _begLength = length;
    else
      _endLength = length;

    NotifySubMeshesHypothesisModification();
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double StdMeshers_Arithmetic1D::GetLength(bool isStartLength) const
{
  return isStartLength ? _begLength : _endLength;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_Arithmetic1D::SaveTo(ostream & save)
{
  save << _begLength << _endLength;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_Arithmetic1D::LoadFrom(istream & load)
{
  bool isOK = true;
  isOK = (load >> _begLength);
  if (!isOK)
    load.clear(ios::badbit | load.rdstate());
  isOK = (load >> _endLength);
  if (!isOK)
    load.clear(ios::badbit | load.rdstate());
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_Arithmetic1D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_Arithmetic1D & hyp)
{
  return hyp.LoadFrom( load );
}
