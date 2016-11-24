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
//  File   : StdMeshers_QuadrangleParams_i.cxx
//  Author : Sergey KUUL, OCC
//  Module : SMESH

#include "StdMeshers_QuadrangleParams_i.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "StdMeshers_ObjRefUlils.hxx"

#include <Utils_CorbaException.hxx>
#include <utilities.h>

#include <Standard_ErrorHandler.hxx>
#include "SMESH_TryCatch.hxx"

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
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetTriaVertex( " << vertID << " )";
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
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
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

//================================================================================
/*!
 * \brief Set positions of enforced nodes
 */
//================================================================================

void StdMeshers_QuadrangleParams_i::SetEnforcedNodes(const GEOM::ListOfGO&     theVertices,
                                                     const SMESH::nodes_array& thePoints)
  throw ( SALOME::SALOME_Exception )
{
  try {
    std::vector< TopoDS_Shape > shapes;
    std::vector< gp_Pnt       > points;
    shapes.reserve( theVertices.length() );
    points.reserve( thePoints.length() );

    myShapeEntries.clear();

    for ( size_t i = 0; i < theVertices.length(); ++i )
    {
      if ( CORBA::is_nil( theVertices[i] ))
        continue;
      CORBA::String_var entry = theVertices[i]->GetStudyEntry();
      if ( !entry.in() || !entry.in()[0] )
        THROW_SALOME_CORBA_EXCEPTION( "Not published enforced vertex shape", SALOME::BAD_PARAM );

      shapes.push_back( StdMeshers_ObjRefUlils::GeomObjectToShape( theVertices[i].in() ));
      myShapeEntries.push_back( entry.in() );
    }
    for ( size_t i = 0; i < thePoints.length(); ++i )
    {
      points.push_back( gp_Pnt( thePoints[i].x, thePoints[i].y, thePoints[i].z ));
    }
    this->GetImpl()->SetEnforcedNodes( shapes, points );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetEnforcedNodes( "
                       << theVertices << ", " << thePoints << " )";
}
  
//================================================================================
/*!
 * \brief Returns positions of enforced nodes
 */
//================================================================================

void StdMeshers_QuadrangleParams_i::GetEnforcedNodes(GEOM::ListOfGO_out     theVertices,
                                                     SMESH::nodes_array_out thePoints)
{
  SMESH_TRY;

  std::vector< TopoDS_Shape > shapes;
  std::vector< gp_Pnt       > points;
  this->GetImpl()->GetEnforcedNodes( shapes, points );

  theVertices = new GEOM::ListOfGO;
  thePoints   = new SMESH::nodes_array;

  size_t i = 0;
  theVertices->length( myShapeEntries.size() );
  for ( i = 0; i < myShapeEntries.size(); ++i )
    theVertices[i] =
      StdMeshers_ObjRefUlils::EntryOrShapeToGeomObject( myShapeEntries[i], shapes[i] );

  thePoints->length( points.size() );
  for ( i = 0; i < points.size(); ++i )
  {
    thePoints[i].x = points[i].X();
    thePoints[i].y = points[i].Y();
    thePoints[i].z = points[i].Z();
  }
  SMESH_CATCH( SMESH::doNothing );
}

//================================================================================
/*!
 * \brief Returns study entries of shapes defining enforced nodes
 */
//================================================================================

SMESH::string_array* StdMeshers_QuadrangleParams_i::GetEnfVertices()
{
  SMESH::string_array_var arr = new SMESH::string_array;
  arr->length( myShapeEntries.size() );

  for ( size_t i = 0; i < myShapeEntries.size(); ++i )
    arr[ i ] = myShapeEntries[ i ].c_str();

  return arr._retn();
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

//================================================================================
/*!
 * \brief Write parameters in a string
  * \retval char* - resulting string
 */
//================================================================================

char* StdMeshers_QuadrangleParams_i::SaveTo()
{
  ASSERT( myBaseImpl );
  std::ostringstream os;

  os << "ENTRIES: " << myShapeEntries.size();
  for ( size_t i = 0; i < myShapeEntries.size(); ++i )
    StdMeshers_ObjRefUlils::SaveToStream( myShapeEntries[ i ], os );
  os << " ";

  myBaseImpl->SaveTo( os );

  return CORBA::string_dup( os.str().c_str() );
}

//================================================================================
/*!
 * \brief Retrieve parameters from the string
  * \param theStream - the input string
 */
//================================================================================

void StdMeshers_QuadrangleParams_i::LoadFrom( const char* theStream )
{
  ASSERT( myBaseImpl );

  bool hasEntries = ( strncmp( "ENTRIES: ", theStream, 9 ) == 0 );
  std::istringstream is( theStream + ( hasEntries ? 9 : 0 ));

  if ( hasEntries )
  {
    int nb = 0;
    if ( is >> nb && nb > 0 )
    {
      std::vector< TopoDS_Shape > shapes;
      std::vector< gp_Pnt >       points;
      myShapeEntries.resize( nb );

      for ( int i = 0; i < nb; ++i )
        shapes.push_back( StdMeshers_ObjRefUlils::LoadFromStream( is, & myShapeEntries[i] ));

      try {
        GetImpl()->SetEnforcedNodes( shapes, points );
      }
      catch (...) {
      }
    }
  }

  myBaseImpl->LoadFrom( is );
}
