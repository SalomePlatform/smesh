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
//  File   : SMESH_LocalLength.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_LocalLength.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LocalLength::SMESH_LocalLength(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_Hypothesis(hypId, studyId, gen)
{
	_length = 1.;
	_name = "LocalLength";
//   SCRUTE(_name);
//   SCRUTE(&_name);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_LocalLength::~SMESH_LocalLength()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_LocalLength::SetLength(double length) throw(SALOME_Exception)
{
	double oldLength = _length;
	if (length <= 0)
		throw SALOME_Exception(LOCALIZED("length must be positive"));
	_length = length;
	if (oldLength != _length)
		NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double SMESH_LocalLength::GetLength() const
{
	return _length;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_LocalLength::SaveTo(ostream & save)
{
	return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_LocalLength::LoadFrom(istream & load)
{
	return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, SMESH_LocalLength & hyp)
{
	save << hyp._length;
	return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, SMESH_LocalLength & hyp)
{
	bool isOK = true;
	double a;
	isOK = (load >> a);
	if (isOK)
		hyp._length = a;
	else
		load.clear(ios::badbit | load.rdstate());
	return load;
}
