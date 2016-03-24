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
//  File   : StdMeshers_Deflection1D_i.cxx
//           Moved here from SMESH_LocalLength_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "StdMeshers_Deflection1D_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i( PortableServer::POA_ptr thePOA,
                                          int                     theStudyId,
                                          ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_Deflection1D_i::StdMeshers_Deflection1D_i" );
  myBaseImpl = new ::StdMeshers_Deflection1D( theGenImpl->GetANewId(),
                                        theStudyId,
                                        theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i()
{
  MESSAGE( "StdMeshers_Deflection1D_i::~StdMeshers_Deflection1D_i" );
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::SetDeflection
 *
 *  Set deflection
 */
//=============================================================================

void StdMeshers_Deflection1D_i::SetDeflection( CORBA::Double theValue )
     throw ( SALOME::SALOME_Exception )
{
  MESSAGE( "StdMeshers_Deflection1D_i::SetDeflection" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetDeflection( theValue );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetDeflection( " << SMESH::TVar(theValue) << " )";
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::GetDeflection
 *
 *  Get deflection
 */
//=============================================================================

CORBA::Double StdMeshers_Deflection1D_i::GetDeflection()
{
  MESSAGE( "StdMeshers_Deflection1D_i::GetDeflection" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetDeflection();
}

//=============================================================================
/*!
 *  StdMeshers_Deflection1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_Deflection1D* StdMeshers_Deflection1D_i::GetImpl()
{
  MESSAGE( "StdMeshers_Deflection1D_i::GetImpl" );
  return ( ::StdMeshers_Deflection1D* )myBaseImpl;
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
CORBA::Boolean StdMeshers_Deflection1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

//================================================================================
/*!
 * \brief Return method name corresponding to index of variable parameter
 */
//================================================================================

std::string StdMeshers_Deflection1D_i::getMethodOfParameter(const int paramIndex,
                                                            int       /*nbVars*/) const
{
  return "SetDeflection";
}
