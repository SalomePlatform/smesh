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
//  File   : StdMeshers_ProjectionSource1D_i.cxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$

#include "StdMeshers_ProjectionSource1D_i.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include "StdMeshers_ObjRefUlils.hxx"

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource1D_i::StdMeshers_ProjectionSource1D_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_ProjectionSource1D_i::StdMeshers_ProjectionSource1D_i
( PortableServer::POA_ptr thePOA,
  int                     theStudyId,
  ::SMESH_Gen*            theGenImpl ) : SALOME::GenericObj_i( thePOA ), 
                                         SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "StdMeshers_ProjectionSource1D_i::StdMeshers_ProjectionSource1D_i" );
  myBaseImpl = new ::StdMeshers_ProjectionSource1D( theGenImpl->GetANewId(),
                                                    theStudyId,
                                                    theGenImpl );
}

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource1D_i::~StdMeshers_ProjectionSource1D_i
 *
 *  Destructor
 */
//=============================================================================

StdMeshers_ProjectionSource1D_i::~StdMeshers_ProjectionSource1D_i()
{
  MESSAGE( "StdMeshers_ProjectionSource1D_i::~StdMeshers_ProjectionSource1D_i" );
}

//=============================================================================
  /*!
   * Sets source <edge> or a group containing edges to take a mesh pattern from
   */
//=============================================================================

void StdMeshers_ProjectionSource1D_i::SetSourceEdge(GEOM::GEOM_Object_ptr edge)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    this->GetImpl()->SetSourceEdge( StdMeshers_ObjRefUlils::GeomObjectToShape( edge ));
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetSourceEdge( " << edge << " )";
}

//=============================================================================
/*!
 * Sets vertex association between the source edge and the target one.
 * This parameter is optional
 */
//=============================================================================

void StdMeshers_ProjectionSource1D_i::SetVertexAssociation(GEOM::GEOM_Object_ptr sourceVertex,
                                                           GEOM::GEOM_Object_ptr targetVertex)
  throw ( SALOME::SALOME_Exception )
{
  ASSERT( myBaseImpl );
  try {
    TopoDS_Shape v1 = StdMeshers_ObjRefUlils::GeomObjectToShape( sourceVertex );
    TopoDS_Shape v2 = StdMeshers_ObjRefUlils::GeomObjectToShape( targetVertex );
    this->GetImpl()->SetVertexAssociation( v1, v2 );
  }
  catch ( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  // Update Python script
  SMESH::TPythonDump() << _this() << ".SetVertexAssociation( "
                       << sourceVertex << ", " << targetVertex << " )";
}

//=============================================================================
/*!
 * Sets source <mesh> to take a mesh pattern from
 */
//=============================================================================

void StdMeshers_ProjectionSource1D_i::SetSourceMesh(SMESH::SMESH_Mesh_ptr theMesh)
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

SMESH::SMESH_Mesh_ptr StdMeshers_ProjectionSource1D_i::GetSourceMesh()
{
  SMESH::SMESH_Mesh_var mesh = myCorbaMesh;
  return mesh._retn();
}

//=============================================================================
/*!
 * Returns the source edge or a group containing edge
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource1D_i::GetSourceEdge()
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetSourceEdge() );
}

//=============================================================================
/*!
 * Returns the vertex associated with the target vertex.
 * Result may be nil if association not set
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource1D_i::GetSourceVertex()
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetSourceVertex() );
}

//=============================================================================
/*!
 * Returns the vertex associated with the source vertex.
 * Result may be nil if association not set
 */
//=============================================================================

GEOM::GEOM_Object_ptr StdMeshers_ProjectionSource1D_i::GetTargetVertex()
{
  ASSERT( myBaseImpl );
  return StdMeshers_ObjRefUlils::ShapeToGeomObject( this->GetImpl()->GetTargetVertex() );
}

//=============================================================================
/*!
 *  StdMeshers_ProjectionSource1D_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::StdMeshers_ProjectionSource1D* StdMeshers_ProjectionSource1D_i::GetImpl()
{
  return ( ::StdMeshers_ProjectionSource1D* )myBaseImpl;
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
CORBA::Boolean StdMeshers_ProjectionSource1D_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_1D;
}

//================================================================================
/*!
 * \brief Write parameters in a string
  * \retval char* - resulting string
 */
//================================================================================

char* StdMeshers_ProjectionSource1D_i::SaveTo()
{
  ASSERT( myBaseImpl );
  std::ostringstream os;

  TopoDS_Shape s1, s2, s3;
  GetImpl()->GetStoreParams( s1, s2, s3 );

  StdMeshers_ObjRefUlils::SaveToStream( s1, os );
  StdMeshers_ObjRefUlils::SaveToStream( s2, os );
  StdMeshers_ObjRefUlils::SaveToStream( s3, os );
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

void StdMeshers_ProjectionSource1D_i::LoadFrom( const char* theStream )
{
  ASSERT( myBaseImpl );
  std::istringstream is( theStream );

  TopoDS_Shape s1 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s2 = StdMeshers_ObjRefUlils::LoadFromStream( is );
  TopoDS_Shape s3 = StdMeshers_ObjRefUlils::LoadFromStream( is );
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
  GetImpl()->RestoreParams( s1, s2, s3, meshImpl );

  myBaseImpl->LoadFrom( is );
}

