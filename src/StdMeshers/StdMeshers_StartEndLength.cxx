//  SMESH StdMeshers_StartEndLength : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_StartEndLength.cxx
//  Module : SMESH
//  $Header$

using namespace std;

#include "StdMeshers_StartEndLength.hxx"
#include "utilities.h"


//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_StartEndLength::StdMeshers_StartEndLength(int         hypId,
                                                     int         studyId,
                                                     SMESH_Gen * gen)
     :SMESH_Hypothesis(hypId, studyId, gen)
{
  _begLength = 1.;
  _endLength = 1.;
  _name = "StartEndLength";
  _param_algo_dim = 1; // is used by SMESH_Regular_1D
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_StartEndLength::~StdMeshers_StartEndLength()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_StartEndLength::SetLength(double length, bool isStartLength)
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

double StdMeshers_StartEndLength::GetLength(bool isStartLength) const
{
  return isStartLength ? _begLength : _endLength;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_StartEndLength::SaveTo(ostream & save)
{
  save << _begLength << _endLength;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_StartEndLength::LoadFrom(istream & load)
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

ostream & operator <<(ostream & save, StdMeshers_StartEndLength & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_StartEndLength & hyp)
{
  return hyp.LoadFrom( load );
}
