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
//  File   : StdMeshers_NumberOfSegments.cxx
//           Moved here from SMESH_NumberOfSegments.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_NumberOfSegments.hxx"

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NumberOfSegments::StdMeshers_NumberOfSegments(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_Hypothesis(hypId, studyId, gen)
{
	_numberOfSegments = 1;
	_scaleFactor = 1.0;
	_name = "NumberOfSegments";
        _param_algo_dim = 1; 
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NumberOfSegments::~StdMeshers_NumberOfSegments()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_NumberOfSegments::SetNumberOfSegments(int segmentsNumber)
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

int StdMeshers_NumberOfSegments::GetNumberOfSegments() const
{
	return _numberOfSegments;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_NumberOfSegments::SetScaleFactor(double scaleFactor)
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

double StdMeshers_NumberOfSegments::GetScaleFactor() const
{
	return _scaleFactor;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_NumberOfSegments::SaveTo(ostream & save)
{
  save << this->_numberOfSegments << " " << this->_scaleFactor;
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_NumberOfSegments::LoadFrom(istream & load)
{
  bool isOK = true;
  int a;
  isOK = (load >> a);
  if (isOK)
    this->_numberOfSegments = a;
  else
    load.clear(ios::badbit | load.rdstate());
  double b;
  isOK = (load >> b);
  if (isOK)
    this->_scaleFactor = b;
  else
    load.clear(ios::badbit | load.rdstate());
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_NumberOfSegments & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_NumberOfSegments & hyp)
{
  return hyp.LoadFrom( load );
}
