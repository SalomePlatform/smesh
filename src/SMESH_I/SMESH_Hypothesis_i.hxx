// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_Hypothesis_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#ifndef _SMESH_HYPOTHESIS_I_HXX_
#define _SMESH_HYPOTHESIS_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include "SMESH_Hypothesis.hxx"
#include "SALOME_GenericObj_i.hh"

#include "SMESH_Gen.hxx"

#include <map>
#include <string>

class TCollection_AsciiString;

// ======================================================
// Generic hypothesis
// ======================================================
class SMESH_I_EXPORT SMESH_Hypothesis_i:
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
  virtual char* GetName();

  // Get plugin library name of hypothesis
  virtual char* GetLibName();

  // Set plugin library name of hypothesis
  void SetLibName( const char* theLibName );

  // Get unique id of hypothesis
  virtual CORBA::Long GetId();

  // Return true if a hypothesis has parameters
  virtual CORBA::Boolean HasParameters();

  // Set the variable parameter (a variable name or a parameter value); \a method is a name
  // of method setting this parameter.
  // This method must be called by the hypothesis creator just before calling hyp->method()
  virtual void SetVarParameter (const char* parameter, const char* method);

  // Return the variable parameter used at Hypothesis Creation by the name of method
  // setting this parameter. The returned variable name is used at Hypothesis Edition.
  virtual char* GetVarParameter (const char* methodName);

  // Store a hypothesis wrapping this not published one. This hyp, which has
  // no own parameters but is published, is used to store variables defining parameters
  // of this hypothesis. This method is to be called before setting parameters
  // of this hypothesis.
  virtual void SetHolderHypothesis(const SMESH::SMESH_Hypothesis_ptr hyp);

  //Return true if hypothesis was published in study
  bool IsPublished();

  // Get implementation
  ::SMESH_Hypothesis* GetImpl();
  
  // Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );
  virtual void  UpdateAsMeshesRestored(); // for hyps needing full data restored

 protected:

  // base hypothesis implementation
  ::SMESH_Hypothesis*         myBaseImpl;

  // a published hypothesis wrapping this not published one
  SMESH::SMESH_Hypothesis_var myHolder;

  // variable parameters
  std::map< std::string, std::string > myMethod2VarParams;

 public:
  // Methods for backward compatibility of notebook variables
  
  // restore myMethod2VarParams by parameters stored in an old study
  virtual void setOldParameters (const char* theParameters);

  // method used to convert variable parameters stored in an old study
  // into myMethod2VarParams. It should return a method name for an index of
  // variable parameters. Index is countered from zero
  virtual std::string getMethodOfParameter(const int paramIndex, int nbVars) const { return ""; }

  // method intended to remove explicit treatment of Netgen hypotheses from SMESH_NoteBook
  virtual int getParamIndex(const TCollection_AsciiString& method, int nbVars) const { return -1; }
};

// ======================================================
// Generic hypothesis creator
// ======================================================
class SMESH_I_EXPORT GenericHypothesisCreator_i
{
public:
  // Create a hypothesis
  virtual SMESH_Hypothesis_i* Create(PortableServer::POA_ptr thePOA,
                                     int                     theStudyId,
                                     ::SMESH_Gen*            theGenImpl) = 0;
  virtual ~GenericHypothesisCreator_i() {}

  // return the name of IDL module
  virtual std::string GetModuleName() = 0;
  virtual bool IsApplicable( const TopoDS_Shape &S, bool toCheckAll ) {return true;}
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
