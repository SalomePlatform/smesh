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
//  File   : SMESH_HypothesisCreator.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

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
