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
//  File   : StdMeshers_Arithmetic1D_i.hxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//
#ifndef _SMESH_ARITHMETIC1D_I_HXX_
#define _SMESH_ARITHMETIC1D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_Arithmetic1D.hxx"

// ======================================================
// Arithmetic 1D hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_Arithmetic1D_i:
  public virtual POA_StdMeshers::StdMeshers_Arithmetic1D,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_Arithmetic1D_i( PortableServer::POA_ptr thePOA,
                            int                     theStudyId,
                             ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_Arithmetic1D_i();

  // Set length
  // * OBSOLETE *. Avoid such a way of interface design
  void SetLength( CORBA::Double theLength, CORBA::Boolean theIsStart )
    throw ( SALOME::SALOME_Exception );

  // Sets <start segment length> parameter value
  void SetStartLength( CORBA::Double length) throw (SALOME::SALOME_Exception);

  // Sets <end segment length> parameter value
  void SetEndLength( CORBA::Double length) throw (SALOME::SALOME_Exception);

  // Get length
  CORBA::Double GetLength(CORBA::Boolean theIsStart);

  //Set Reversed Edges
  void SetReversedEdges( const SMESH::long_array& theIDs);

  //Get Reversed Edges
  SMESH::long_array*  GetReversedEdges();
  
  //Set the Entry of the Object
  void SetObjectEntry( const char* theEntry);

  //Get Object Entry
  char* GetObjectEntry();

  // Get implementation
  ::StdMeshers_Arithmetic1D* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 protected:
  virtual std::string getMethodOfParameter(const int paramIndex, int nbVars) const;
};

#endif
