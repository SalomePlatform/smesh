using namespace std;
//=============================================================================
// File      : SMESH_HypothesisFactory_i.cxx
// Created   : dim mai 19 22:02:42 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_HypothesisFactory_i.hxx"
#include "SMESH_Hypothesis_i.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

// Add new hypothesis here (include file)
//---------------------------------------
#include "SMESH_LocalLength_i.hxx"
#include "SMESH_NumberOfSegments_i.hxx"
#include "SMESH_MaxElementArea_i.hxx"
#include "SMESH_Regular_1D_i.hxx"
#include "SMESH_MEFISTO_2D_i.hxx"
#include "SMESH_Quadrangle_2D_i.hxx"
#include "SMESH_Hexa_3D_i.hxx"

//---------------------------------------

//=============================================================================
/*!
 * Specific Hypothesis Creators are generated with a template which inherits a
 * generic hypothesis creator. Each creator returns an hypothesis of the type
 * given in the template. 
 */
//=============================================================================

template <class T> class HypothesisCreator_i: public GenericHypothesisCreator_i
{
public:
  virtual SMESH_Hypothesis_i* Create (const char* anHyp,
				      int studyId,
				      ::SMESH_Gen* genImpl) 
  {
    return new T(anHyp, studyId, genImpl);
  };
};

//=============================================================================
/*!
 * Constructor: instanciate specific hypothesis creators, fill a private map
 * indexed by hypothesis names. THIS METHOD MUST BE COMPLETED WHEN A NEW
 * HYPOTHESIS IS ADDED. 
 * Specific hypothesis creator are defined with the above template.
 * Hypothesis names are related to the corresponding class names:
 * prefix = SMESH_ ; suffix = _i .
 */
//=============================================================================

SMESH_HypothesisFactory_i::SMESH_HypothesisFactory_i()
{
// Add new hypothesis here (creators)
//---------------------------------------
_creatorMap["LocalLength"] = new HypothesisCreator_i<SMESH_LocalLength_i>;
_creatorMap["NumberOfSegments"] = new HypothesisCreator_i<SMESH_NumberOfSegments_i>;
_creatorMap["MaxElementArea"] = new HypothesisCreator_i<SMESH_MaxElementArea_i>;
_creatorMap["Regular_1D"] = new HypothesisCreator_i<SMESH_Regular_1D_i>;
_creatorMap["MEFISTO_2D"] = new HypothesisCreator_i<SMESH_MEFISTO_2D_i>;
_creatorMap["Quadrangle_2D"] = new HypothesisCreator_i<SMESH_Quadrangle_2D_i>;
_creatorMap["Hexa_3D"] = new HypothesisCreator_i<SMESH_Hexa_3D_i>;

//---------------------------------------
}

//=============================================================================
/*!
 * Destructor: deletes specific hypothesis creators instanciated in the
 * constructor.
 */
//=============================================================================

SMESH_HypothesisFactory_i::~SMESH_HypothesisFactory_i()
{
  map<string, GenericHypothesisCreator_i*>::iterator it;
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

SMESH_Hypothesis_i* SMESH_HypothesisFactory_i::Create(const char* anHyp,
						      CORBA::Long studyId,
						      ::SMESH_Gen* genImpl)
  throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_HypothesisFactory::Create " << anHyp);
  if (_creatorMap.find(anHyp) == _creatorMap.end())
    {
      MESSAGE("levee exception CORBA");
      THROW_SALOME_CORBA_EXCEPTION("bad hypothesis type name", \
				   SALOME::BAD_PARAM);
    }
  SMESH_Hypothesis_i* myHyp = _creatorMap[anHyp]->Create(anHyp,
							 studyId,
							 genImpl);
  return myHyp;
}


