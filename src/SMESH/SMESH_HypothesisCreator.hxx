//=============================================================================
// File      : SMESH_HypothesisCreator.hxx
// Created   : lun mai 27 15:28:35 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_HYPOTHESISCREATOR_HXX_
#define _SMESH_HYPOTHESISCREATOR_HXX_

#include  "SMESH_HypothesisFactory.hxx"

class SMESH_gen;

//=============================================================================
/*!
 * Specific Hypothesis Creators are generated with a template which inherits a
 * generic hypothesis creator. Each creator returns an hypothesis of the type
 * given in the template. 
 */
//=============================================================================

template <class T> class SMESH_HypothesisCreator
  : public GenericHypothesisCreator
{
public:
//   map<int, T*> _instances;

//   virtual T* GetInstance(int hypId)
//   {
//     if (_instances.find(hypId) != _instances.end())
//       return _instances[hypId];
//     else
//       return NULL;
//   }

  virtual T* Create (int hypId, int studyId, SMESH_Gen* gen)
  {
    T* anInstance=  new T(hypId, studyId, gen);
//     _gen->StoreHypothesisInstance(anInstance);
//     _instances[hypId] = anInstance;
    return anInstance;
  };
};


#endif
