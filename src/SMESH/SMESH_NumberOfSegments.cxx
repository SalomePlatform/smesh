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
//  File   : SMESH_NumberOfSegments.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_NumberOfSegments.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NumberOfSegments::SMESH_NumberOfSegments(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_Hypothesis(hypId, studyId, gen)
{
	_numberOfSegments = 1;
	_scaleFactor = 1.0;
	_name = "NumberOfSegments";
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_NumberOfSegments::~SMESH_NumberOfSegments()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_NumberOfSegments::SetNumberOfSegments(int segmentsNumber)
throw(SALOME_Exception)
{
	int oldNumberOfSegments = _numberOfSegments;
	if (segmentsNumber <= 0)
		throw
			SALOME_Exception(LOCALIZED("number of segments must be positive"));
	_numberOfSegments = segmentsNumber;

	if (oldNumberOfSegments != _numberOfSegments)
		NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_NumberOfSegments::GetNumberOfSegments() const
{
	return _numberOfSegments;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_NumberOfSegments::SetScaleFactor(double scaleFactor)
throw(SALOME_Exception)
{
	if (scaleFactor < 0)
		throw SALOME_Exception(LOCALIZED("scale factor must be positive"));
	_scaleFactor = scaleFactor;

	NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double SMESH_NumberOfSegments::GetScaleFactor() const
{
	return _scaleFactor;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_NumberOfSegments::SaveTo(ostream & save)
{
	return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_NumberOfSegments::LoadFrom(istream & load)
{
	return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, SMESH_NumberOfSegments & hyp)
{
	save << hyp._numberOfSegments;
	return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, SMESH_NumberOfSegments & hyp)
{
	bool isOK = true;
	int a;
	isOK = (load >> a);
	if (isOK)
		hyp._numberOfSegments = a;
	else
		load.clear(ios::badbit | load.rdstate());
	return load;
}
