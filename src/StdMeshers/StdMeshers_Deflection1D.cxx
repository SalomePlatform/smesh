//  SMESH StdMeshers_Deflection1D : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_Deflection1D.cxx
//  Module : SMESH
//  $Header$

using namespace std;

#include "StdMeshers_Deflection1D.hxx"
#include "utilities.h"


//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Deflection1D::StdMeshers_Deflection1D(int         hypId,
                                                 int         studyId,
                                                 SMESH_Gen * gen)
     :SMESH_Hypothesis(hypId, studyId, gen)
{
  _value = 1.;
  _name = "Deflection1D";
  _param_algo_dim = 1; // is used by SMESH_Regular_1D
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Deflection1D::~StdMeshers_Deflection1D()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Deflection1D::SetDeflection(double value)
     throw(SALOME_Exception)
{
  if (_value != value) {
    if (value <= 0.)
      throw SALOME_Exception(LOCALIZED("Value must be positive"));

    NotifySubMeshesHypothesisModification();

    _value = value;
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double StdMeshers_Deflection1D::GetDeflection() const
{
  return _value;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_Deflection1D::SaveTo(ostream & save)
{
  save << _value;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_Deflection1D::LoadFrom(istream & load)
{
  bool isOK = (load >> _value);
  if (!isOK)
    load.clear(ios::badbit | load.rdstate());
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_Deflection1D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_Deflection1D & hyp)
{
  return hyp.LoadFrom( load );
}
