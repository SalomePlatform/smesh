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
//  File   : StdMeshers_QuadraticMesh_i.cxx
//           Moved here from SMESH_LocalLength_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#include "StdMeshers_QuadraticMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_QuadraticMesh_i::StdMeshers_QuadraticMesh_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_QuadraticMesh_i::StdMeshers_QuadraticMesh_i
( PortableServer::POA_ptr thePOA,
  int                     theStudyId,
  ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ),SMESH_Hypothesis_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_QuadraticMesh( theGenImpl->GetANewId(),
                                               theStudyId,
                                               theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_QuadraticMesh_i::~StdMeshers_QuadraticMesh_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_QuadraticMesh_i::~StdMeshers_QuadraticMesh_i()
{
}

//=============================================================================
/*!
 *  StdMeshers_QuadraticMesh_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_QuadraticMesh* StdMeshers_QuadraticMesh_i::GetImpl()
{
  return ( ::StdMeshers_QuadraticMesh* )myBaseImpl;
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

CORBA::Boolean StdMeshers_QuadraticMesh_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

