// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_LayerDistribution2D_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#include "StdMeshers_LayerDistribution2D_i.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution2D_i::StdMeshers_LayerDistribution2D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_LayerDistribution2D_i::StdMeshers_LayerDistribution2D_i
                                          (PortableServer::POA_ptr thePOA,
                                           int                     theStudyId,
                                           ::SMESH_Gen*            theGenImpl )
:SMESH_Hypothesis_i( thePOA ),
 StdMeshers_LayerDistribution_i(thePOA,theStudyId,theGenImpl)
{
  MESSAGE( "StdMeshers_LayerDistribution2D_i::StdMeshers_LayerDistribution2D_i" );
  myBaseImpl = new ::StdMeshers_LayerDistribution2D(theGenImpl->GetANewId(),
                                                    theStudyId,
                                                    theGenImpl);
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution2D_i::~StdMeshers_LayerDistribution2D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_LayerDistribution2D_i::~StdMeshers_LayerDistribution2D_i()
{
  MESSAGE("StdMeshers_LayerDistribution2D_i::~StdMeshers_LayerDistribution2D_i");
}

//=============================================================================
/*!
 *  StdMeshers_LayerDistribution2D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_LayerDistribution2D* StdMeshers_LayerDistribution2D_i::GetImpl()
{
  return ( ::StdMeshers_LayerDistribution2D* )myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  
CORBA::Boolean StdMeshers_LayerDistribution2D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_2D;
}


