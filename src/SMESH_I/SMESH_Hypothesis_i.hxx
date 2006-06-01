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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_Hypothesis_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HYPOTHESIS_I_HXX_
#define _SMESH_HYPOTHESIS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Hypothesis.hxx"
#include "SALOME_GenericObj_i.hh"

#include "SMESH_Gen.hxx"

// ======================================================
// Generic hypothesis
// ======================================================
class SMESH_Hypothesis_i:
  public virtual POA_SMESH::SMESH_Hypothesis,
  public virtual SALOME::GenericObj_i
{
public:
  // Constructor : placed in protected section to prohibit creation of generic class instance
  SMESH_Hypothesis_i( PortableServer::POA_ptr thePOA );

public:
  // Destructor
  virtual ~SMESH_Hypothesis_i();

  // Get type name of hypothesis
  char* GetName();

  // Get plugin library name of hypothesis
  char* GetLibName();

  // Set plugin library name of hypothesis
  void SetLibName( const char* theLibName );

  // Get unique id of hypothesis
  CORBA::Long GetId();

  // Get implementation
  ::SMESH_Hypothesis* GetImpl();
  
  // _CS_gbo_ Activate the object using the POA
  void Activate();

  // Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );
  
protected:
  ::SMESH_Hypothesis* myBaseImpl;    // base hypothesis implementation

  // _CS_gbo_070505 To keep the reference and delayed the activation
  // in the methode Activate().
  PortableServer::POA_ptr myPOA;
};

// ======================================================
// Generic hypothesis creator
// ======================================================
class GenericHypothesisCreator_i
{
public:
  // Create a hypothesis
  virtual SMESH_Hypothesis_i* Create(PortableServer::POA_ptr thePOA,
                                     int                     theStudyId,
                                     ::SMESH_Gen*            theGenImpl) = 0;
  // return the name of IDL module
  virtual std::string GetModuleName() = 0;
};

//=============================================================================
//
// Specific Hypothesis Creators are generated with a template which inherits a
// generic hypothesis creator. Each creator returns an hypothesis of the type
// given in the template. 
//
//=============================================================================
template <class T> class HypothesisCreator_i: public GenericHypothesisCreator_i
{
public:
  virtual SMESH_Hypothesis_i* Create (PortableServer::POA_ptr thePOA,
                                      int                     theStudyId,
                                      ::SMESH_Gen*            theGenImpl)
  {
    return new T (thePOA, theStudyId, theGenImpl);
  };
};

#endif
