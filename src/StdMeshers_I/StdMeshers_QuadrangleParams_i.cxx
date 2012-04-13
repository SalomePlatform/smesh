// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//  File   : StdMeshers_QuadrangleParams_i.cxx
//  Author : Sergey KUUL, OCC
//  Module : SMESH

#include "StdMeshers_QuadrangleParams_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::StdMeshers_QuadrangleParams_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_QuadrangleParams_i::StdMeshers_QuadrangleParams_i
                                          (PortableServer::POA_ptr thePOA,
                                           int                     theStudyId,
                                           ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::StdMeshers_QuadrangleParams_i" );
  myBaseImpl = new ::StdMeshers_QuadrangleParams(theGenImpl->GetANewId(),
                                                 theStudyId,
                                                 theGenImpl);
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::~StdMeshers_QuadrangleParams_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_QuadrangleParams_i::~StdMeshers_QuadrangleParams_i()
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::~StdMeshers_QuadrangleParams_i" );
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::SetTriaVertex
 *
 *  Set base vertex for triangles
 */
//=============================================================================

void StdMeshers_QuadrangleParams_i::SetTriaVertex(CORBA::Long vertID)
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::SetTriaVertex" );
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetTriaVertex( vertID );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetTriaVertex( "
      << vertID << " )";
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::GetTriaVertex
 *
 *  Get base vertex for triangles
 */
//=============================================================================

CORBA::Long StdMeshers_QuadrangleParams_i::GetTriaVertex()
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::GetTriaVertex" );
  ASSERT( myBaseImpl );
  return this->GetImpl()->GetTriaVertex();
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::SetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

void StdMeshers_QuadrangleParams_i::SetObjectEntry( const char* entry )
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::SetObjectEntry" );
  ASSERT( myBaseImpl );

  try {
    this->GetImpl()->SetObjectEntry( entry );
    // Update Python script
    //    SMESH::TPythonDump() << _this() << ".SetObjectEntry( '" << entry << "' )";
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::GetObjectEntry
 *
 *  Set the Entry for the Main Object
 */
//=============================================================================

char* StdMeshers_QuadrangleParams_i::GetObjectEntry()
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::SetObjectEntry" );
  ASSERT( myBaseImpl );
  const char* entry;
  try {
    entry = this->GetImpl()->GetObjectEntry();
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(),
                                  SALOME::BAD_PARAM );
  }
  return CORBA::string_dup( entry );
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::SetQuadType
 *
 *  Set the type of quadrangulation
 */
//=============================================================================
void StdMeshers_QuadrangleParams_i::SetQuadType(StdMeshers::QuadType type)
{
  //static char* quadTypes[5] = {"StdMeshers.QUAD_STANDARD",
  //                             "StdMeshers.QUAD_TRIANGLE_PREF",
  //                             "StdMeshers.QUAD_QUADRANGLE_PREF",
  //                             "StdMeshers.QUAD_QUADRANGLE_PREF_REVERSED",
  //                             "StdMeshers.QUAD_REDUCED"};

  MESSAGE("StdMeshers_QuadrangleParams_i::SetQuadType");
  ASSERT(myBaseImpl);

  if (int(type) >= int(StdMeshers::QUAD_NB_TYPES)) {
    THROW_SALOME_CORBA_EXCEPTION("Bad type of quadrangulation", SALOME::BAD_PARAM);
  }

  try {
    this->GetImpl()->SetQuadType(StdMeshers_QuadType(int(type)));
  }
  catch (SALOME_Exception& S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }

  // Update Python script
  const char* quadType;
  switch (type) {
  case StdMeshers::QUAD_STANDARD:
    quadType = "StdMeshers.QUAD_STANDARD"; break;
  case StdMeshers::QUAD_TRIANGLE_PREF:
    quadType = "StdMeshers.QUAD_TRIANGLE_PREF"; break;
  case StdMeshers::QUAD_QUADRANGLE_PREF:
    quadType = "StdMeshers.QUAD_QUADRANGLE_PREF"; break;
  case StdMeshers::QUAD_QUADRANGLE_PREF_REVERSED:
    quadType = "StdMeshers.QUAD_QUADRANGLE_PREF_REVERSED"; break;
  case StdMeshers::QUAD_REDUCED:
    quadType = "StdMeshers.QUAD_REDUCED"; break;
  default:
    quadType = "UNKNOWN";
  }
  SMESH::TPythonDump() << _this() << ".SetQuadType( " << quadType << " )";
  //SMESH::TPythonDump() << _this() << ".SetQuadType( " << quadTypes[int(type)] << " )";
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::GetQuadType
 *
 *  Get the type of quadrangulation
 */
//=============================================================================
StdMeshers::QuadType StdMeshers_QuadrangleParams_i::GetQuadType()
{
  MESSAGE("StdMeshers_QuadrangleParams_i::GetQuadType");
  ASSERT(myBaseImpl);
  return StdMeshers::QuadType(int(this->GetImpl()->GetQuadType()));
}

//=============================================================================
/*!
 *  StdMeshers_QuadrangleParams_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_QuadrangleParams* StdMeshers_QuadrangleParams_i::GetImpl()
{
  MESSAGE( "StdMeshers_QuadrangleParams_i::GetImpl" );
  return ( ::StdMeshers_QuadrangleParams* )myBaseImpl;
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
CORBA::Boolean StdMeshers_QuadrangleParams_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_2D;
}
