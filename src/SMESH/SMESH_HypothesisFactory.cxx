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
//  File   : SMESH_HypothesisFactory.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_HypothesisFactory.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_HypothesisCreator.hxx"
#include "SMESH_Gen.hxx"

#include "utilities.h"

// Add new hypothesis here (include file)
//---------------------------------------
#include "SMESH_LocalLength.hxx"
#include "SMESH_LengthFromEdges.hxx"
#include "SMESH_NumberOfSegments.hxx"
#include "SMESH_MaxElementArea.hxx"
#include "SMESH_MaxElementVolume.hxx"
#include "SMESH_Regular_1D.hxx"
#include "SMESH_MEFISTO_2D.hxx"
#include "SMESH_Quadrangle_2D.hxx"
#include "SMESH_Hexa_3D.hxx"
#include "SMESH_NETGEN_3D.hxx"

//---------------------------------------

//=============================================================================
/*!
 * Specific Hypothesis Creators are generated with a template which inherits a
 * generic hypothesis creator. Each creator returns an hypothesis of the type
 * given in the template. 
 */
//=============================================================================

// template <class T> class HypothesisCreator: public GenericHypothesisCreator
// {
// public:
//   virtual T* Create (int hypId)
//   {
// //     return new T(hypId);
//   };

// };

//=============================================================================
/*!
 * Constructor: instanciate specific hypothesis creators, fill a private map
 * indexed by hypothesis names. THIS METHOD MUST BE COMPLETED WHEN A NEW
 * HYPOTHESIS IS ADDED. 
 * Specific hypothesis creator are defined with the above template.
 * Hypothesis names are related to the corresponding class names:
 * prefix = SMESH_ ; suffix = .
 */
//=============================================================================

SMESH_HypothesisFactory::SMESH_HypothesisFactory()
{
  _hypId = 0;

// Add new hypothesis here (creators)
//---------------------------------------
_creatorMap["LocalLength"] = new SMESH_HypothesisCreator<SMESH_LocalLength>;
_creatorMap["NumberOfSegments"] = new SMESH_HypothesisCreator<SMESH_NumberOfSegments>;
_creatorMap["LengthFromEdges"] = new SMESH_HypothesisCreator<SMESH_LengthFromEdges>;
_creatorMap["MaxElementArea"] = new SMESH_HypothesisCreator<SMESH_MaxElementArea>;
_creatorMap["MaxElementVolume"] = new SMESH_HypothesisCreator<SMESH_MaxElementVolume>;
_creatorMap["Regular_1D"] = new SMESH_HypothesisCreator<SMESH_Regular_1D>;
_creatorMap["MEFISTO_2D"] = new SMESH_HypothesisCreator<SMESH_MEFISTO_2D>;
_creatorMap["Quadrangle_2D"] = new SMESH_HypothesisCreator<SMESH_Quadrangle_2D>;
_creatorMap["Hexa_3D"] = new SMESH_HypothesisCreator<SMESH_Hexa_3D>;
_creatorMap["NETGEN_3D"] = new SMESH_HypothesisCreator<SMESH_NETGEN_3D>;
//---------------------------------------
}

//=============================================================================
/*!
 * Destructor: deletes specific hypothesis creators instanciated in the
 * constructor.
 */
//=============================================================================

SMESH_HypothesisFactory::~SMESH_HypothesisFactory()
{
  map<string, GenericHypothesisCreator*>::iterator it;
  for (it = _creatorMap.begin(); it !=  _creatorMap.end(); it++)
    {
      delete (*it).second;
    }
  _creatorMap.clear();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Hypothesis* SMESH_HypothesisFactory::Create(const char* anHypName,
						  int studyId)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_HypothesisFactory::Create " << anHypName);
  if (_creatorMap.find(anHypName) == _creatorMap.end())
    throw(SALOME_Exception(LOCALIZED("bad hypothesis type name")));
  SMESH_Hypothesis* myHyp = _creatorMap[anHypName]->Create(_hypId++,
							   studyId,
							   _gen);
  return myHyp;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

GenericHypothesisCreator*
SMESH_HypothesisFactory::GetCreator(const char* anHypName)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_HypothesisFactory::GetCreator " << anHypName);
  if (_creatorMap.find(anHypName) == _creatorMap.end())
    throw(SALOME_Exception(LOCALIZED("bad hypothesis type name")));
  return _creatorMap[anHypName];
}

//=============================================================================
/*!
 *
 */
//=============================================================================

int SMESH_HypothesisFactory::GetANewId()
{
  //MESSAGE("SMESH_HypothesisFactory::GetANewId");
  return _hypId++;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_HypothesisFactory::SetGen(SMESH_Gen* gen)
{
  //MESSAGE("SMESH_HypothesisFactory::SetGen");
  _gen = gen;
}

