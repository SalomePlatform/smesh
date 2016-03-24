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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_NumberOfLayers_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#include "StdMeshers_NumberOfLayers_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_NumberOfLayers_i::StdMeshers_NumberOfLayers_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_NumberOfLayers_i::StdMeshers_NumberOfLayers_i( PortableServer::POA_ptr thePOA,
                                                          int                     theStudyId,
                                                          ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_NumberOfLayers_i::StdMeshers_NumberOfLayers_i" );
  myBaseImpl = new ::StdMeshers_NumberOfLayers( theGenImpl->GetANewId(),
                                                 theStudyId,
                                                 theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfLayers_i::~StdMeshers_NumberOfLayers_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_NumberOfLayers_i::~StdMeshers_NumberOfLayers_i()
{
  MESSAGE( "StdMeshers_NumberOfLayers_i::~StdMeshers_NumberOfLayers_i" );
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfLayers_i::SetNumberOfLayers
 *
 *  Sets <number of segments> parameter value
 */
//=============================================================================

void StdMeshers_NumberOfLayers_i::SetNumberOfLayers(CORBA::Long numberOfLayers)
     throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetNumberOfLayers( numberOfLayers );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetNumberOfLayers( " << SMESH::TVar(numberOfLayers) << " )";
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfLayers_i::GetNumberOfLayers
 *
 *  Returns <number of layers> parameter value
 */
//=============================================================================

CORBA::Long StdMeshers_NumberOfLayers_i::GetNumberOfLayers()
{
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetNumberOfLayers();
}

//=============================================================================
/*!
 *  StdMeshers_NumberOfLayers_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_NumberOfLayers* StdMeshers_NumberOfLayers_i::GetImpl()
{
  return ( ::StdMeshers_NumberOfLayers* )myBaseImpl;
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
CORBA::Boolean StdMeshers_NumberOfLayers_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_NumberOfLayers_i::getMethodOfParameter(const int, int) const
{
  return "SetNumberOfLayers";
}
