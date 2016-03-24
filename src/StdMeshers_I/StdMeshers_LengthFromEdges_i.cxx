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
//  File   : StdMeshers_LengthFromEdges_i.cxx
//           Moved here from SMESH_LengthFromEdges_i.cxx
//  Author : Nadir BOUHAMOU CEA/DEN, Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#include "StdMeshers_LengthFromEdges_i.hxx"
#include "SMESH_Gen.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_LengthFromEdges_i::StdMeshers_LengthFromEdges_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_LengthFromEdges_i::StdMeshers_LengthFromEdges_i( PortableServer::POA_ptr thePOA,
                                                  int                     theStudyId,
                                                  ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_LengthFromEdges_i::StdMeshers_LengthFromEdges_i" );
  myBaseImpl = new ::StdMeshers_LengthFromEdges( theGenImpl->GetANewId(),
                                            theStudyId,
                                            theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_LengthFromEdges_i::~StdMeshers_LengthFromEdges_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_LengthFromEdges_i::~StdMeshers_LengthFromEdges_i()
{
  MESSAGE( "StdMeshers_LengthFromEdges_i::~StdMeshers_LengthFromEdges_i" );
}

//=============================================================================
/*!
 *  StdMeshers_LengthFromEdges_i::SetMode
 *
 *  Set mode
 */
//=============================================================================

void StdMeshers_LengthFromEdges_i::SetMode( CORBA::Long theMode )
  throw (SALOME::SALOME_Exception)
{
  MESSAGE( "StdMeshers_LengthFromEdges_i::SetMode" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetMode( theMode );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }
}

//=============================================================================
/*!
 *  StdMeshers_LengthFromEdges_i::GetMode
 *
 *  Get mode
 */
//=============================================================================

CORBA::Long StdMeshers_LengthFromEdges_i::GetMode()
{
  MESSAGE( "StdMeshers_LengthFromEdges_i::GetMode" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetMode();
}

//================================================================================
/*!
 * \brief Return false as in SALOME the mode is not used
 */
//================================================================================

CORBA::Boolean StdMeshers_LengthFromEdges_i::HasParameters()
{
  return false;
}

//=============================================================================
/*!
 *  StdMeshers_LengthFromEdges_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_LengthFromEdges* StdMeshers_LengthFromEdges_i::GetImpl()
{
  MESSAGE( "StdMeshers_LengthFromEdges_i::GetImpl" );
  return ( ::StdMeshers_LengthFromEdges* )myBaseImpl;
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
CORBA::Boolean StdMeshers_LengthFromEdges_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_2D;
}

