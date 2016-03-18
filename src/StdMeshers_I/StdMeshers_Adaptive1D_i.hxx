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
//  File   : StdMeshers_Adaptive1D_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_Adaptive1D_I_HXX_
#define _SMESH_Adaptive1D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

class StdMeshers_Adaptive1D;
class SMESH_Gen;

// ======================================================
// Adaptive1D hypothesis
// ======================================================

class STDMESHERS_I_EXPORT StdMeshers_Adaptive1D_i:
  public virtual POA_StdMeshers::StdMeshers_Adaptive1D,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  StdMeshers_Adaptive1D_i( PortableServer::POA_ptr thePOA,
                           int                     theStudyId,
                           ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_Adaptive1D_i();

  /*!
   * Sets minimal allowed segment length
   */
  void SetMinSize( CORBA::Double minSegLen ) throw (SALOME::SALOME_Exception);
  CORBA::Double GetMinSize();

  /*!
   * Sets maximal allowed segment length
   */
  void SetMaxSize( CORBA::Double maxSegLen ) throw (SALOME::SALOME_Exception);
  CORBA::Double GetMaxSize();

  /*!
   * Sets <deflection> parameter value, 
   * i.e. a maximal allowed distance between a segment and an edge.
   */
  void SetDeflection( CORBA::Double theLength ) throw (SALOME::SALOME_Exception);
  CORBA::Double GetDeflection();


  /*!
   * Returns implementation
   */
  ::StdMeshers_Adaptive1D* GetImpl();

  /*!
   * \brief Verify whether hypothesis supports given entity type 
   * \param type - dimension (see SMESH::Dimension enumeration)
   * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
   *
   * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
   */
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
