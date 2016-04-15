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
//  File   : StdMeshers_MaxElementArea_i.cxx
//           Moved here from SMESH_MaxElementArea_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "StdMeshers_MaxElementArea_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_MaxElementArea_i::StdMeshers_MaxElementArea_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_MaxElementArea_i::StdMeshers_MaxElementArea_i( PortableServer::POA_ptr thePOA,
                                                int                     theStudyId,
                                                ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ) 
{
  MESSAGE( "StdMeshers_MaxElementArea_i::StdMeshers_MaxElementArea_i" );
  myBaseImpl = new ::StdMeshers_MaxElementArea( theGenImpl->GetANewId(),
                                           theStudyId,
                                           theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_MaxElementArea_i::~StdMeshers_MaxElementArea_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_MaxElementArea_i::~StdMeshers_MaxElementArea_i()
{
  MESSAGE( "StdMeshers_MaxElementArea_i::~StdMeshers_MaxElementArea_i" );
}

//=============================================================================
/*!
 *  StdMeshers_MaxElementArea_i::SetMaxElementArea
 *
 *  Set maximum element area 
 */
//=============================================================================

void StdMeshers_MaxElementArea_i::SetMaxElementArea( CORBA::Double theArea )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_MaxElementArea_i::SetMaxElementArea" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetMaxArea( theArea );
  }
  catch (SALOME_Exception& S_ex) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetMaxElementArea( " << SMESH::TVar(theArea) << " )";
}

//=============================================================================
/*!
 *  StdMeshers_MaxElementArea_i::GetMaxElementArea
 *
 *  Get maximum element area 
 */
//=============================================================================

CORBA::Double StdMeshers_MaxElementArea_i::GetMaxElementArea()
{
  MESSAGE( "StdMeshers_MaxElementArea_i::GetMaxElementArea" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetMaxArea();
}

//=============================================================================
/*!
 *  StdMeshers_MaxElementArea_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_MaxElementArea* StdMeshers_MaxElementArea_i::GetImpl()
{
  MESSAGE( "StdMeshers_MaxElementArea_i::GetImpl" );
  return ( ::StdMeshers_MaxElementArea* )myBaseImpl;
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
CORBA::Boolean StdMeshers_MaxElementArea_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_2D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_MaxElementArea_i::getMethodOfParameter(const int paramIndex,
                                                              int       /*nbVars*/) const
{
  return "SetMaxElementArea";
}
