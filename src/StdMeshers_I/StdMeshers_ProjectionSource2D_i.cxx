//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : StdMeshers_ProjectionSource2D_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$

#include "StdMeshers_ProjectionSource2D_i.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "StdMeshers_ObjRefUlils.hxx"

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource2D_i::StdMeshers_ProjectionSource2D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_ProjectionSource2D_i::StdMeshers_ProjectionSource2D_i
( PortableServer::POA_ptr thePOA,
  int                     theStudyId,
  ::SMESH_Gen*            theGenImpl ) : SALOME::GenericObj_i( thePOA ), 
                                         SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_ProjectionSource2D_i::StdMeshers_ProjectionSource2D_i" );
  myBaseImpl = new ::StdMeshers_ProjectionSource2D( theGenImpl->GetANewId(),
                                                    theStudyId,
                                                    theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource2D_i::~StdMeshers_ProjectionSource2D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_ProjectionSource2D_i::~StdMeshers_ProjectionSource2D_i()
{
  MESSAGE( "StdMeshers_ProjectionSource2D_i::~StdMeshers_ProjectionSource2D_i" );
}

//=============================================================================
  /*!
   * Sets a source <face> to take a mesh pattern from
   */
//=============================================================================

void StdMeshers_ProjectionSource2D_i::SetSourceFace(GEOM::GEOM_Object_ptr face)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetSourceFace( StdMeshers_ObjRefUlils::GeomObjectToShape( face ));
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetSourceFace( " << face << " )";
}

//=============================================================================
/*!
 * Sets source <mesh> to take a mesh pattern from
 */
//=============================================================================

void StdMeshers_ProjectionSource2D_i::SetSourceMesh(SMESH::SMESH_Mesh_ptr theMesh)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );

  ::SMESH_Mesh* mesh = 0;

  if ( !CORBA::is_nil( theMesh ))
  {
    SMESH_Mesh_i* mesh_i = SMESH::DownCast< SMESH_Mesh_i* >( theMesh );
    if ( !mesh_i )
      THROW_SALOME_CORBA_EXCEPTION( "bad mesh", SALOME::BAD_PARAM );
    mesh = &mesh_i->GetImpl();
  }

  try {
    this->GetImpl()->SetSourceMesh ( mesh );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }

  myCorbaMesh = SMESH::SMESH_Mesh::_duplicate( theMesh );

  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetSourceMesh( " << theMesh << " )";
}

//=============================================================================
/*!
 * Return source mesh
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr StdMeshers_ProjectionSource2D_i::GetSourceMesh()
{
  SMESH::SMESH_Mesh_var mesh = myCorbaMesh;
  return mesh._retn();
}

//=============================================================================
/*!
 * Sets vertex association between the source face and the target one.
 * This parameter is optional.
 * Two vertices must belong to one edge of a face
 */
//=============================================================================

void StdMeshers_ProjectionSource2D_i::SetVertexAssociation(GEOM::GEOM_Object_ptr sourceVertex1,
                                                           GEOM::GEOM_Object_ptr sourceVertex2,
                                                           GEOM::GEOM_Object_ptr targetVertex1,
                                                           GEOM::GEOM_Object_ptr targetVertex2)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    TopoDS_Shape v1 = StdMeshers_ObjRefUlils::GeomObjectToShape( sourceVertex1 );
    TopoDS_Shape v2 = StdMeshers_ObjRefUlils::GeomObjectToShape( sourceVertex2 );
    TopoDS_Shape v3 = StdMeshers_ObjRefUlils::GeomObjectToShape( targetVertex1 );
    TopoDS_Shape v4 = StdMeshers_ObjRefUlils::GeomObjectToShape( targetVertex2 );
    this->GetImpl()->SetVertexAssociation( v1, v2, v3, v4 );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetVertexAssociation( "
                       << sourceVertex1 << ", "
                       << sourceVertex2 << ", "
                       << targetVertex1 << ", "
                       << targetVertex2 << " )";
}

//=============================================================================
/*!
 * Returns the source face
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource2D_i::GetSourceFace()
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetSourceFace() );
}

//=============================================================================
/*!
 * Returns the vertex associated with the target vertex.
 * Result may be nil if association not set
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource2D_i::GetSourceVertex(CORBA::Long i)
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetSourceVertex((int) i ));
}

//=============================================================================
/*!
 * Returns the <i>-th target vertex associated with the <i>-th source vertex.
 * Result may be nil if association not set.
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource2D_i::GetTargetVertex(CORBA::Long i)
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetTargetVertex( (int)i ));
}

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource2D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_ProjectionSource2D* StdMeshers_ProjectionSource2D_i::GetImpl()
{
  return ( ::StdMeshers_ProjectionSource2D* )myBaseImpl;
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
CORBA::Boolean StdMeshers_ProjectionSource2D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_2D;
}

//================================================================================
/*!
 * \brief Write parameters in a string
  * \retval char* - resulting string
 */
//================================================================================

char* StdMeshers_ProjectionSource2D_i::SaveTo()
{
  ASSERT( myBaseImpl );
  std::ostringstream os;

  TopoDS_Shape s1, s2, s3, s4, s5;
  GetImpl()->GetStoreParams( s1, s2, s3, s4, s5 );

  StdMeshers_ObjRefUlils::SaveToStream( s1, os );
  StdMeshers_ObjRefUlils::SaveToStream( s2, os );
  StdMeshers_ObjRefUlils::SaveToStream( s3, os );
  StdMeshers_ObjRefUlils::SaveToStream( s4, os );
  StdMeshers_ObjRefUlils::SaveToStream( s5, os );
  StdMeshers_ObjRefUlils::SaveToStream( GetSourceMesh(), os );

  myBaseImpl->SaveTo( os );

  return CORBA::string_dup( os.str().c_str() );
}

//================================================================================
/*!
 * \brief Retrieve parameters from the string
  * \param theStream - the input string
 */
//================================================================================

void StdMeshers_ProjectionSource2D_i::LoadFrom( const char* theStream )
{
  ASSERT( myBaseImpl );
  std::istringstream is( theStream );

  TopoDS_Shape s1 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s2 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s3 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s4 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s5 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  SMESH::SMESH_Mesh_var mesh = 
    StdMeshers_ObjRefUlils::LoadObjectFromStream< SMESH::SMESH_Mesh >( is );

  ::SMESH_Mesh* meshImpl = 0;

  if ( !CORBA::is_nil( mesh ))
  {
    SMESH_Mesh_i* mesh_i = SMESH::DownCast< SMESH_Mesh_i* >( mesh );
    if ( mesh_i )
      meshImpl = &mesh_i->GetImpl();
  }

  myCorbaMesh = SMESH::SMESH_Mesh::_duplicate( mesh );
  GetImpl()->RestoreParams( s1, s2, s3, s4, s5, meshImpl );

  myBaseImpl->LoadFrom( is );
}

