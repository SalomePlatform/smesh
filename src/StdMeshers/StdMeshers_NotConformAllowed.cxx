//  SMESH StdMeshers : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_NotConformAllowed.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_NotConformAllowed.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NotConformAllowed::StdMeshers_NotConformAllowed(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
  _name = "NotConformAllowed";
  _param_algo_dim = -1; 
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NotConformAllowed::~StdMeshers_NotConformAllowed()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_NotConformAllowed::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_NotConformAllowed::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, StdMeshers_NotConformAllowed & hyp)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, StdMeshers_NotConformAllowed & hyp)
{
  return load;
}
