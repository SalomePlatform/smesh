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
//  File   : SMESH_1D_Algo_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "SMESH_1D_Algo_i.hxx"

//=============================================================================
/*!
 *  SMESH_1D_Algo_i::SMESH_1D_Algo_i
 * 
 *  Constructor
 */
//=============================================================================

SMESH_1D_Algo_i::SMESH_1D_Algo_i( PortableServer::POA_ptr thePOA )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA )
{
}

//=============================================================================
/*!
 *  SMESH_1D_Algo_i::~SMESH_1D_Algo_i
 * 
 *  Destructor
 */
//=============================================================================

SMESH_1D_Algo_i::~SMESH_1D_Algo_i()
{
}

//================================================================================
/*!
 * \brief Verify whether algorithm supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether algorithm supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================
CORBA::Boolean SMESH_1D_Algo_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}














