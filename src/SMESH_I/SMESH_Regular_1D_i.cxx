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
//  File   : SMESH_Regular_1D_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_Regular_1D_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_HypothesisFactory.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Regular_1D_i::SMESH_Regular_1D_i(const char* anHyp,
				       int studyId,
				       ::SMESH_Gen* genImpl)
{
  MESSAGE("SMESH_Regular_1D_i::SMESH_Regular_1D_i");
  _genImpl = genImpl;
  ::SMESH_Regular_1D* impl 
      = new ::SMESH_Regular_1D(_genImpl->_hypothesisFactory.GetANewId(),
			       studyId,
			       genImpl);
  SetImpl(impl);
  _baseImpl = _impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Regular_1D_i::~SMESH_Regular_1D_i()
{
  MESSAGE("SMESH_Regular_1D_i::~SMESH_Regular_1D_i");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Regular_1D_i::SetImpl(::SMESH_Regular_1D* impl)
{
  MESSAGE("SMESH_Regular_1D_i::SetImpl");
  SMESH_1D_Algo_i::SetImpl(impl);
  _impl = impl;
}
