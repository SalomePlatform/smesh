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

//  File   : StdMeshers_LayerDistribution_i.hxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#ifndef _SMESH_LayerDistribution_I_HXX_
#define _SMESH_LayerDistribution_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_LayerDistribution.hxx"

class SMESH_Gen;

// =========================================================
/*!
 * This hypothesis is used by "Radial prism" algorithm.
 * It specifies 1D hypothesis defining distribution of segments between the internal 
 * and the external surfaces.
 */
// =========================================================

class StdMeshers_LayerDistribution_i:
  public virtual POA_StdMeshers::StdMeshers_LayerDistribution,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_LayerDistribution_i( PortableServer::POA_ptr thePOA,
                                  int                     theStudyId,
                                  ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_LayerDistribution_i();

  /*!
   * \brief Sets  1D hypothesis specifying distribution of layers
    * \param hyp1D - 1D hypothesis
   */
  void SetLayerDistribution(SMESH::SMESH_Hypothesis_ptr hyp1D)
    throw ( SALOME::SALOME_Exception );

  /*!
   * \brief Returns 1D hypothesis specifying distribution of layers
    * \retval SMESH::SMESH_Hypothesis_ptr - 1D hypothesis
   */
  SMESH::SMESH_Hypothesis_ptr GetLayerDistribution();

  // Get implementation
  ::StdMeshers_LayerDistribution* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

  // Redefined Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );

protected:
  // restore myMethod2VarParams by parameters stored in an old study
  virtual void setOldParameters (const char* theParameters);
  
private:
  SMESH::SMESH_Hypothesis_var myHyp;
};

#endif
