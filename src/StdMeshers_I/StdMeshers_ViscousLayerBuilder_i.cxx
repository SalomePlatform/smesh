// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_ViscousLayers_i.cxx
//  Module : SMESH
//
#include "StdMeshers_ViscousLayerBuilder_i.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_PythonDump.hxx"

#include "BRepTools.hxx"
#include "Utils_CorbaException.hxx"
#include "utilities.h"

#include <TCollection_AsciiString.hxx>

#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=============================================================================
/*!
 *  StdMeshers_ViscousLayerBuilder_i::StdMeshers_ViscousLayerBuilder_i
 *
 *  Constructor
 */
//=============================================================================

StdMeshers_ViscousLayerBuilder_i::StdMeshers_ViscousLayerBuilder_i( PortableServer::POA_ptr thePOA,
                                                                      ::SMESH_Gen*            theGenImpl )
  : SALOME::GenericObj_i( thePOA ), 
      SMESH_Hypothesis_i( thePOA ),
      SMESH_Algo_i( thePOA ),
      SMESH_2D_Algo_i( thePOA )
{
  myBaseImpl = new ::StdMeshers_ViscousLayerBuilder( theGenImpl->GetANewId(),
                                                      theGenImpl );
}

::StdMeshers_ViscousLayerBuilder* StdMeshers_ViscousLayerBuilder_i::GetImpl()
{
  return ( ::StdMeshers_ViscousLayerBuilder* )myBaseImpl;
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
CORBA::Boolean StdMeshers_ViscousLayerBuilder_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D || type == SMESH::DIM_2D;
}

void StdMeshers_ViscousLayerBuilder_i::SetFaces(const ::SMESH::long_array& faceIDs,
                                                  CORBA::Boolean             toIgnore)
{
  vector<int> ids( faceIDs.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
    if (( ids[i] = faceIDs[i] ) < 1 )
      THROW_SALOME_CORBA_EXCEPTION( "Invalid face id", SALOME::BAD_PARAM );

  GetImpl()->SetBndShapes( ids, toIgnore );

  SMESH::TPythonDump() << _this() << ".SetFaces( " << faceIDs << ", " << toIgnore << " )";
}


void StdMeshers_ViscousLayerBuilder_i::SetIgnoreFaces(const ::SMESH::long_array& faceIDs)
{
  vector<int> ids( faceIDs.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
    if (( ids[i] = faceIDs[i] ) < 1 )
      THROW_SALOME_CORBA_EXCEPTION( "Invalid face id", SALOME::BAD_PARAM );
  GetImpl()->SetBndShapes( ids, /*toIgnore=*/true );
  SMESH::TPythonDump() << _this() << ".SetIgnoreFaces( " << faceIDs << " )";
}

void StdMeshers_ViscousLayerBuilder_i::SetTotalThickness(::CORBA::Double thickness)
{
  if ( thickness < 1e-100 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid thickness", SALOME::BAD_PARAM );
  GetImpl()->SetTotalThickness(thickness);
  SMESH::TPythonDump() << _this() << ".SetTotalThickness( " << SMESH::TVar(thickness) << " )";
}

void StdMeshers_ViscousLayerBuilder_i::SetNumberLayers(::CORBA::Short nb)
{
  if ( nb < 1 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid number of layers", SALOME::BAD_PARAM );
  GetImpl()->SetNumberLayers( nb );
  SMESH::TPythonDump() << _this() << ".SetNumberLayers( " << SMESH::TVar(nb) << " )";
}

void StdMeshers_ViscousLayerBuilder_i::SetStretchFactor(::CORBA::Double factor)
{
  if ( factor < 1 )
    THROW_SALOME_CORBA_EXCEPTION( "Invalid stretch factor, it must be >= 1.0", SALOME::BAD_PARAM );
  GetImpl()->SetStretchFactor(factor);
  SMESH::TPythonDump() << _this() << ".SetStretchFactor( " << SMESH::TVar(factor) << " )";
}

void StdMeshers_ViscousLayerBuilder_i::SetMethod( ::StdMeshers::VLExtrusionMethod how )
{
  GetImpl()->SetMethod( ::StdMeshers_ViscousLayers::ExtrusionMethod( how ));
  const char* methNames[3] = { "SURF_OFFSET_SMOOTH",
                               "FACE_OFFSET",
                               "NODE_OFFSET" };
  if ( how >= 0 && how < 3 )
    SMESH::TPythonDump() << _this() << ".SetMethod( StdMeshers." << methNames[ how ]<< " )";
}

void StdMeshers_ViscousLayerBuilder_i::SetGroupName(const char* name)
{
  GetImpl()->SetGroupName( name );
  SMESH::TPythonDump() << _this() << ".SetGroupName( '" << name << "' )";
}

GEOM::GEOM_Object_ptr StdMeshers_ViscousLayerBuilder_i::GetShrinkGeometry( SMESH::SMESH_Mesh_ptr finalMesh, GEOM::GEOM_Object_ptr theShapeObject )
{
  
  GEOM::GEOM_Object_var aShapeObj;
  TopoDS_Shape theShape        = StdMeshers_ObjRefUlils::GeomObjectToShape( theShapeObject );
  SMESH_Mesh_i* theFinalMesh_i = SMESH::DownCast< SMESH_Mesh_i* >( finalMesh );
  TopoDS_Shape shrinkGeometry;
  
  try 
  {
    shrinkGeometry = GetImpl()->GetShrinkGeometry( theFinalMesh_i->GetImpl(), theShape );
  }
  catch ( std::exception& exc )
  {
    std::cout << exc.what() << "\n";
    THROW_SALOME_CORBA_EXCEPTION( exc.what(), SALOME::INTERNAL_ERROR  );
    return aShapeObj; // Maybe better to return a init and empty object(?)
  }

  if ( !shrinkGeometry.IsNull() )
  {     
    std::ostringstream streamShape;
    //Write TopoDS_Shape in ASCII format to the stream
    BRepTools::Write(shrinkGeometry, streamShape);
    //Returns the number of bytes that have been stored in the stream's buffer.
    int size = streamShape.str().size();
    //Allocate octect buffer of required size
    CORBA::Octet* OctetBuf = SALOMEDS::TMPFile::allocbuf(size);
    //Copy ostrstream content to the octect buffer
    memcpy(OctetBuf, streamShape.str().c_str(), size);
    //Create and return TMPFile
    SALOMEDS::TMPFile_var SeqFile = new SALOMEDS::TMPFile(size,size,OctetBuf,1);
    // Get the geom engine
    GEOM::GEOM_Gen_var geomEngine = theShapeObject->GetGen();
    auto iOp                      = geomEngine->GetIInsertOperations();
    aShapeObj                     = iOp->RestoreShape( SeqFile );
    geomEngine->AddInStudy( aShapeObj, "Shrink", GEOM::GEOM_Object::_nil());        
  }

  return aShapeObj;
}

CORBA::Boolean StdMeshers_ViscousLayerBuilder_i::AddLayers( SMESH::SMESH_Mesh_ptr shrinkMesh, SMESH::SMESH_Mesh_ptr finalMesh, GEOM::GEOM_Object_ptr theShapeObject )
{
  TopoDS_Shape theShape         = StdMeshers_ObjRefUlils::GeomObjectToShape( theShapeObject );
  SMESH_Mesh_i* shrinkMesh_i    = SMESH::DownCast< SMESH_Mesh_i* >( shrinkMesh );
  SMESH_Mesh_i* theFinalMesh_i  = SMESH::DownCast< SMESH_Mesh_i* >( finalMesh );
  
  bool success = GetImpl()->AddLayers( shrinkMesh_i->GetImpl(), theFinalMesh_i->GetImpl(), theShape );  
  
  return success;
}
//=============================================================================
/*!
 *  StdMeshers_ViscousLayerBuilder_i::~StdMeshers_ViscousLayerBuilder_i
 *
 *  Destructor
 */
//=============================================================================
StdMeshers_ViscousLayerBuilder_i::~StdMeshers_ViscousLayerBuilder_i()
{
}