//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_Hypothesis_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Gen.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis_i::SMESH_Hypothesis_i()
{
  MESSAGE("SMESH_Hypothesis_i");
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis_i::~SMESH_Hypothesis_i()
{
  MESSAGE("~SMESH_Hypothesis_i");
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

char* SMESH_Hypothesis_i::GetName()
{
  MESSAGE("GetName");
  return CORBA::string_dup(_baseImpl->GetName());
//   const char* name = _baseImpl->GetName();
//   SCRUTE(name);
//   return CORBA::string_dup(name);
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Hypothesis_i::GetId()
{
  MESSAGE("GetId");
  return _baseImpl->GetID();
}

::SMESH_Hypothesis* SMESH_Hypothesis_i::getImpl()
{
	return _baseImpl;
}