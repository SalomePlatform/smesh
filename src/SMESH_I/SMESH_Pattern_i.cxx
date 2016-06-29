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
// File      : SMESH_Pattern_i.cxx
// Created   : Fri Aug 20 16:15:49 2004
// Author    : Edward AGAPOV (eap)
//
#include "SMESH_Pattern_i.hxx"

#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_PythonDump.hxx"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

#include <sstream>
#include <set>

using namespace std;

using SMESH::TPythonDump;
using SMESH::TVar;

//=======================================================================
//function : dumpErrorCode
//purpose  : 
//=======================================================================

static void addErrorCode(const char* thePyCommand)
{
  TPythonDump() << "if (isDone != 1):";
  TPythonDump() << "\tprint '" << thePyCommand << " :', pattern.GetErrorCode()";
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetPattern
 *
 *  Create pattern mapper
 */
//=============================================================================

SMESH::SMESH_Pattern_ptr SMESH_Gen_i::GetPattern()
{
  // Update Python script
  TPythonDump() << "pattern = " << this << ".GetPattern()";

  SMESH_Pattern_i* i = new SMESH_Pattern_i( this );
  SMESH::SMESH_Pattern_var anObj = i->_this();
  return anObj._retn();
}

//=======================================================================
//function : SMESH_Pattern_i
//purpose  : 
//=======================================================================

SMESH_Pattern_i::SMESH_Pattern_i( SMESH_Gen_i* theGen_i ):
  myGen( theGen_i )
{
}

//=======================================================================
//function : getMesh
//purpose  :
//=======================================================================

::SMESH_Mesh* SMESH_Pattern_i::getMesh( SMESH::SMESH_Mesh_ptr & theMesh )
{
  SMESH_Mesh_i* anImplPtr =
    dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( theMesh ).in() );
  if ( anImplPtr )
  {
    anImplPtr->Load();
    return & anImplPtr->GetImpl();
  }

  return 0;
}

//=======================================================================
//function : LoadFromFile
//purpose  :
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFromFile(const char* theFileContents)
{
  // remove some gabage from the end
  TCollection_AsciiString patternDescription = (char*) theFileContents;
  int pos = patternDescription.Length();
  while (! isdigit( patternDescription.Value( pos )))
    pos--;
  if ( pos != patternDescription.Length() ) {
    patternDescription.Trunc( pos );
  }

  // Update Python script
  TPythonDump() << "isDone = pattern.LoadFromFile("
                << TPythonDump::LongStringStart("Pattern")
                << patternDescription
                << TPythonDump::LongStringEnd()
                << ")";
  addErrorCode( "LoadFromFile" );

  return myPattern.Load( theFileContents );
}

//=======================================================================
//function : LoadFromFace
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFromFace(SMESH::SMESH_Mesh_ptr theMesh,
                                             GEOM::GEOM_Object_ptr theFace,
                                             CORBA::Boolean        theProject)
{
  if ( theMesh->_is_nil() || theFace->_is_nil() )
    return false;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  TopoDS_Shape aFace = myGen->GeomObjectToShape( theFace );
  if ( aFace.IsNull() || aFace.ShapeType() != TopAbs_FACE )
    return false;

  // Update Python script
  TPythonDump() << "isDone = pattern.LoadFromFace( " << theMesh << ".GetMesh(), "
                << theFace << ", " << theProject << " )";
  addErrorCode( "LoadFromFace" );

  return myPattern.Load( aMesh, TopoDS::Face( aFace ), theProject );
}

//=======================================================================
//function : LoadFrom3DBlock
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFrom3DBlock(SMESH::SMESH_Mesh_ptr theMesh,
                                                GEOM::GEOM_Object_ptr theBlock)
{
  if ( theMesh->_is_nil() || theBlock->_is_nil() )
    return false;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  TopoDS_Shape aShape = myGen->GeomObjectToShape( theBlock );
  if ( aShape.IsNull())
    return false;

  TopExp_Explorer exp ( aShape, TopAbs_SHELL );
  if ( !exp.More() )
    return false;

  // Update Python script
  TPythonDump() << "isDone = pattern.LoadFrom3DBlock( " << theMesh << ".GetMesh(), " << theBlock << " )";
  addErrorCode( "LoadFrom3DBlock" );

  return myPattern.Load( aMesh, TopoDS::Shell( exp.Current() ));
}

//=======================================================================
//function : ApplyToFace
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::ApplyToFace(GEOM::GEOM_Object_ptr theFace,
                                                 GEOM::GEOM_Object_ptr theVertexOnKeyPoint1,
                                                 CORBA::Boolean        theReverse)
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  TopoDS_Shape F = myGen->GeomObjectToShape( theFace );
  TopoDS_Shape V = myGen->GeomObjectToShape( theVertexOnKeyPoint1 );

  if (!F.IsNull() && F.ShapeType() == TopAbs_FACE &&
      !V.IsNull() && V.ShapeType() == TopAbs_VERTEX
      &&
      myPattern.Apply( TopoDS::Face( F ), TopoDS::Vertex( V ), theReverse ) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }
  // Update Python script
  TPythonDump() << "pattern.ApplyToFace( " << theFace << ", "
                << theVertexOnKeyPoint1 << ", " << theReverse << " )";

  return points._retn();
}

//=======================================================================
//function : ApplyTo3DBlock
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::ApplyTo3DBlock(GEOM::GEOM_Object_ptr theBlock,
                                                    GEOM::GEOM_Object_ptr theVertex000,
                                                    GEOM::GEOM_Object_ptr theVertex001)
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  TopExp_Explorer exp( myGen->GeomObjectToShape( theBlock ), TopAbs_SHELL );
  TopoDS_Shape V000 = myGen->GeomObjectToShape( theVertex000 );
  TopoDS_Shape V001 = myGen->GeomObjectToShape( theVertex001 );

  if (exp.More() &&
      !V000.IsNull() && V000.ShapeType() == TopAbs_VERTEX &&
      !V001.IsNull() && V001.ShapeType() == TopAbs_VERTEX 
      &&
      myPattern.Apply(TopoDS::Shell( exp.Current() ),
                      TopoDS::Vertex( V000 ),
                      TopoDS::Vertex( V001 )) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  // Update Python script
  TPythonDump() << "pattern.ApplyTo3DBlock( " << theBlock << ", "
                << theVertex000 << ", " << theVertex001 << " )";

  return points._retn();
}

//=======================================================================
//function : ApplyToMeshFaces
//purpose  : 
//=======================================================================

SMESH::point_array*
  SMESH_Pattern_i::ApplyToMeshFaces(SMESH::SMESH_Mesh_ptr    theMesh,
                                    const SMESH::long_array& theFacesIDs,
                                    CORBA::Long              theNodeIndexOnKeyPoint1,
                                    CORBA::Boolean           theReverse)
{
  SMESH::point_array_var points = new SMESH::point_array;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return points._retn();

  list<const gp_XYZ *> xyzList;
  set<const SMDS_MeshFace*> fset;
  for ( CORBA::ULong i = 0; i < theFacesIDs.length(); i++)
  {
    CORBA::Long index = theFacesIDs[i];
    const SMDS_MeshElement * elem = aMesh->GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face )
      fset.insert( static_cast<const SMDS_MeshFace *>( elem ));
  }
  bool ok = false;
  try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
    OCC_CATCH_SIGNALS;
#endif
    ok = myPattern.Apply( aMesh, fset, theNodeIndexOnKeyPoint1, theReverse );
  }
  catch (Standard_Failure& exc) {
    MESSAGE("OCCT Exception in SMESH_Pattern: " << exc.GetMessageString());
  }
  catch ( std::exception& exc ) {
    MESSAGE("STD Exception in SMESH_Pattern: << exc.what()");
  }
  catch ( ... ) {
    MESSAGE("Unknown Exception in SMESH_Pattern");
  }

  if ( ok && myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  // Update Python script
  TPythonDump() << "pattern.ApplyToMeshFaces( " << theMesh << ".GetMesh(), "
                << theFacesIDs << ", "
                << TVar( theNodeIndexOnKeyPoint1 ) << ", " << theReverse << " )";

  return points._retn();
}

//=======================================================================
//function : ApplyToHexahedrons
//purpose  : 
//=======================================================================

SMESH::point_array*
  SMESH_Pattern_i::ApplyToHexahedrons(SMESH::SMESH_Mesh_ptr    theMesh,
                                      const SMESH::long_array& theVolumesIDs,
                                      CORBA::Long              theNode000Index,
                                      CORBA::Long              theNode001Index)
{
  SMESH::point_array_var points = new SMESH::point_array;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return points._retn();

  list<const gp_XYZ *> xyzList;
  set<const SMDS_MeshVolume*> vset;
  for ( CORBA::ULong i = 0; i < theVolumesIDs.length(); i++)
  {
    CORBA::Long index = theVolumesIDs[i];
    const SMDS_MeshElement * elem = aMesh->GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Volume && elem->NbNodes() == 8 )
      vset.insert( static_cast<const SMDS_MeshVolume *>( elem ));
  }
  if (myPattern.Apply( vset, theNode000Index, theNode001Index ) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  // Update Python script
  TPythonDump() << "pattern.ApplyToHexahedrons( " << theMesh << ".GetMesh(), "
                << theVolumesIDs << ", "
                << TVar(theNode000Index) << ", " << TVar(theNode001Index) << " )";

  return points._retn();
}

//=======================================================================
//function : MakeMesh
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::MakeMesh (SMESH::SMESH_Mesh_ptr theMesh,
                                          const CORBA::Boolean  CreatePolygons,
                                          const CORBA::Boolean  CreatePolyedrs)
{
  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  // Update Python script
  TPythonDump() << "isDone = pattern.MakeMesh( " << theMesh << ".GetMesh(), "
                << CreatePolygons << ", " << CreatePolyedrs << " )";
  addErrorCode( "MakeMesh" );

  int nb = aMesh->NbNodes() + aMesh->NbEdges() + aMesh->NbFaces() + aMesh->NbVolumes();

  bool res = myPattern.MakeMesh( aMesh, CreatePolygons, CreatePolyedrs );

  if ( nb > 0 && nb != aMesh->NbNodes() + aMesh->NbEdges() + aMesh->NbFaces() + aMesh->NbVolumes())
    {
      aMesh->SetIsModified(true);
      aMesh->GetMeshDS()->Modified();
    }
  return res;
}

//=======================================================================
//function : GetString
//purpose  : 
//=======================================================================

char* SMESH_Pattern_i::GetString()
{
  ostringstream os;
  myPattern.Save( os );
  
  return CORBA::string_dup( os.str().c_str() );
}

//=======================================================================
//function : Is2D
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::Is2D()
{
  return myPattern.Is2D();
}

//=======================================================================
//function : GetPoints
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::GetPoints()
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  if (myPattern.GetPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : GetKeyPoints
//purpose  : 
//=======================================================================

SMESH::long_array* SMESH_Pattern_i::GetKeyPoints()
{
  SMESH::long_array_var ids = new SMESH::long_array;
  if ( myPattern.IsLoaded() ) {
    const list< int > & idList = myPattern.GetKeyPointIDs();
    ids->length( idList.size() );
    list< int >::const_iterator iIt = idList.begin();
    for ( int i = 0; iIt != idList.end(); iIt++, i++ )
      ids[ i ] = *iIt;
  }
  return ids._retn();
}

//=======================================================================
//function : GetElementPoints
//purpose  : 
//=======================================================================

SMESH::array_of_long_array* SMESH_Pattern_i::GetElementPoints(CORBA::Boolean applied)
{
  SMESH::array_of_long_array_var arrayOfArray = new SMESH::array_of_long_array;

  const list< list< int > >& listOfIdList = myPattern.GetElementPointIDs(applied);
  arrayOfArray->length( listOfIdList.size() );
  list< list< int > >::const_iterator llIt = listOfIdList.begin();
  for ( int i = 0 ; llIt != listOfIdList.end(); llIt++, i++ )
  {
    const list< int > & idList = (*llIt);
    SMESH::long_array& ids = arrayOfArray[ i ];
    ids.length( idList.size() );
    list< int >::const_iterator iIt = idList.begin();
    for ( int j = 0; iIt != idList.end(); iIt++, j++ )
      ids[ j ] = *iIt;
  }
  return arrayOfArray._retn();
}

//=======================================================================
//function : GetErrorCode
//purpose  : 
//=======================================================================

#define RETCASE(enm) case ::SMESH_Pattern::enm: return SMESH::SMESH_Pattern::enm;

SMESH::SMESH_Pattern::ErrorCode SMESH_Pattern_i::GetErrorCode()
{
  switch ( myPattern.GetErrorCode() ) {
    RETCASE( ERR_OK );
    RETCASE( ERR_READ_NB_POINTS );
    RETCASE( ERR_READ_POINT_COORDS );
    RETCASE( ERR_READ_TOO_FEW_POINTS );
    RETCASE( ERR_READ_3D_COORD );
    RETCASE( ERR_READ_NO_KEYPOINT );
    RETCASE( ERR_READ_BAD_INDEX );
    RETCASE( ERR_READ_ELEM_POINTS );
    RETCASE( ERR_READ_NO_ELEMS );
    RETCASE( ERR_READ_BAD_KEY_POINT );
    RETCASE( ERR_SAVE_NOT_LOADED );
    RETCASE( ERR_LOAD_EMPTY_SUBMESH );
    RETCASE( ERR_LOADF_NARROW_FACE );
    RETCASE( ERR_LOADF_CLOSED_FACE );
    RETCASE( ERR_LOADF_CANT_PROJECT );
    RETCASE( ERR_LOADV_BAD_SHAPE );
    RETCASE( ERR_LOADV_COMPUTE_PARAMS );
    RETCASE( ERR_APPL_NOT_LOADED );
    RETCASE( ERR_APPL_BAD_DIMENTION );
    RETCASE( ERR_APPL_BAD_NB_VERTICES );
    RETCASE( ERR_APPLF_BAD_TOPOLOGY );
    RETCASE( ERR_APPLF_BAD_VERTEX );
    RETCASE( ERR_APPLF_INTERNAL_EEROR );
    RETCASE( ERR_APPLV_BAD_SHAPE );
    RETCASE( ERR_MAKEM_NOT_COMPUTED );
  default:;
  };
  return SMESH::SMESH_Pattern::ERR_OK;
}

