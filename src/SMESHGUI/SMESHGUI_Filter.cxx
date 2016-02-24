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

// SMESHGUI_Filter : Filters for VTK viewer
// File   : SMESHGUI_Filter.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Filter.h"

#include "SMESHGUI_Utils.h"

#include <SMESH_Actor.h>
#include <SMDS_Mesh.hxx>
#include <SMDSAbs_ElementType.hxx>

OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_Filter, VTKViewer_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_PredicateFilter, SMESHGUI_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_QuadrangleFilter, SMESHGUI_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_TriangleFilter, SMESHGUI_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_FacesFilter, SMESHGUI_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_VolumesFilter, SMESHGUI_Filter)
OCCT_IMPLEMENT_STANDARD_RTTIEXT(SMESHGUI_VolumeShapeFilter, SMESHGUI_Filter)

/*
  Class       : SMESHGUI_PredicateFilter
  Description : Selection filter for VTK viewer. This class aggregate object
                of SMESH_Predicate class and uses it for verification of criterion
*/

//=======================================================================
// name    : SMESHGUI_PredicateFilter::SMESHGUI_PredicateFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_PredicateFilter::SMESHGUI_PredicateFilter()
{
}

SMESHGUI_PredicateFilter::~SMESHGUI_PredicateFilter()
{
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::IsValid
// Purpose : Verify whether entry id satisfies to criterion of the filter
//=======================================================================
bool SMESHGUI_PredicateFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 || myPred->_is_nil() )
    return false;

  SMESH_Actor* anActor = dynamic_cast<SMESH_Actor*>( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  SMDSAbs_ElementType anElemType = (SMDSAbs_ElementType)myPred->GetElementType();
  int aMeshId = anElemType == SMDSAbs_Node ? anActor->GetNodeObjId( theCellId )
                                           : anActor->GetElemObjId( theCellId );

  // if type of element != type of predicate return true because
  // this predicate is not intended for filtering sush elements
  const SMDS_MeshElement* anElem = anElemType == SMDSAbs_Node ? aMesh->FindNode( aMeshId )
                                                              : aMesh->FindElement( aMeshId );
  // here we guess that predicate element type can not be All in case of node selection
  if ( !anElem || (anElemType != SMDSAbs_All && anElem->GetType() != anElemType) )
    return false;
  
  return myPred->IsSatisfy( aMeshId );
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::IsValid
// Purpose : Verify whether entry id satisfies to criterion of the filter
//=======================================================================
bool SMESHGUI_PredicateFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 || myPred->_is_nil() )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  SMDSAbs_ElementType anElemType = (SMDSAbs_ElementType)myPred->GetElementType();

  // if type of element != type of predicate return true because
  // this predicate is not intended for filtering sush elements
  const SMDS_MeshElement* anElem = anElemType == SMDSAbs_Node ? aMesh->FindNode( theObjId )
                                                              : aMesh->FindElement( theObjId );
  // here we guess that predicate element type can not be All in case of node selection
  if ( !anElem || (anElemType != SMDSAbs_All && anElem->GetType() != anElemType) )
    return false;

  return myPred->IsSatisfy( theObjId );
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_PredicateFilter::IsNodeFilter() const
{
  return GetId() == SMESH::NodeFilter;
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::SetPredicate
// Purpose : Set new pridicate to the filter
//=======================================================================
void SMESHGUI_PredicateFilter::SetPredicate( SMESH::Predicate_ptr thePred )
{
  myPred = SMESH::Predicate::_duplicate( thePred );
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::SetActor
// Purpose : Set new actor
//=======================================================================
void SMESHGUI_PredicateFilter::SetActor( SALOME_Actor* theActor )
{
  if ( myActor == theActor )
    return;
  SMESHGUI_Filter::SetActor( theActor );

  if ( myActor != 0 && !myPred->_is_nil() )
  {
    SALOME_Actor* sActor = dynamic_cast<SALOME_Actor*>( myActor );
    Handle(SALOME_InteractiveObject) anIO;
    if( sActor )
      anIO = sActor->getIO();
    if ( !anIO.IsNull() )
    {
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIO);
      if(!aMesh->_is_nil())
        myPred->SetMesh(aMesh);
    }
  }
}

//=======================================================================
// name    : SMESHGUI_PredicateFilter::SetActor
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_PredicateFilter::GetId() const
{
  if      ( myPred->GetElementType() == SMESH::NODE   ) return SMESH::NodeFilter;
  else if ( myPred->GetElementType() == SMESH::EDGE   ) return SMESH::EdgeFilter;
  else if ( myPred->GetElementType() == SMESH::FACE   ) return SMESH::FaceFilter;
  else if ( myPred->GetElementType() == SMESH::VOLUME ) return SMESH::VolumeFilter;
  else if ( myPred->GetElementType() == SMESH::ALL    ) return SMESH::AllElementsFilter;
  else                                                  return SMESH::UnknownFilter;
}


/*
  Class       : SMESHGUI_QuadrangleFilter
  Description : Verify whether selected cell is quadranle
*/

//=======================================================================
// name    : SMESHGUI_QuadrangleFilter::SMESHGUI_QuadrangleFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_QuadrangleFilter::SMESHGUI_QuadrangleFilter()
: SMESHGUI_Filter()
{
}

SMESHGUI_QuadrangleFilter::~SMESHGUI_QuadrangleFilter()
{
}

//=======================================================================
// name    : SMESHGUI_QuadrangleFilter::IsValid
// Purpose : Verify whether selected cell is quadranle
//=======================================================================
bool SMESHGUI_QuadrangleFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( anActor->GetElemObjId( theCellId ) );

  return anElem && anElem->GetType() == SMDSAbs_Face &&
    ( anElem->NbNodes() == ( anElem->IsQuadratic() ? 8 : 4 ));
}

//=======================================================================
// name    : SMESHGUI_QuadrangleFilter::IsValid
// Purpose : Verify whether selected cell is quadranle
//=======================================================================
bool SMESHGUI_QuadrangleFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( theObjId );

  return anElem && anElem->GetType() == SMDSAbs_Face &&
    ( anElem->NbNodes() == ( anElem->IsQuadratic() ? 8  : 4 ));
}

//=======================================================================
// name    : SMESHGUI_QuadrangleFilter::SetActor
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_QuadrangleFilter::GetId() const
{
  return SMESH::QuadFilter;
}

//=======================================================================
// name    : SMESHGUI_QuadrangleFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_QuadrangleFilter::IsNodeFilter() const
{
  return false;
}


/*
  Class       : SMESHGUI_TriangleFilter
  Description : Verify whether selected cell is triangle
*/


//=======================================================================
// name    : SMESHGUI_TriangleFilter::SMESHGUI_TriangleFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_TriangleFilter::SMESHGUI_TriangleFilter()
: SMESHGUI_Filter()
{
}

SMESHGUI_TriangleFilter::~SMESHGUI_TriangleFilter()
{
}

//=======================================================================
// name    : SMESHGUI_TriangleFilter::IsValid
// Purpose : Verify whether selected cell is triangle
//=======================================================================
bool SMESHGUI_TriangleFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( anActor->GetElemObjId( theCellId ) );

  return anElem && anElem->GetType() == SMDSAbs_Face &&
    ( anElem->NbNodes() == ( anElem->IsQuadratic() ? 6  : 3 ));
}

//=======================================================================
// name    : SMESHGUI_TriangleFilter::IsValid
// Purpose : Verify whether selected cell is triangle
//=======================================================================
bool SMESHGUI_TriangleFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( theObjId );

  return anElem && anElem->GetType() == SMDSAbs_Face &&
    ( anElem->NbNodes() == ( anElem->IsQuadratic() ? 6  : 3 ));
}

//=======================================================================
// name    : SMESHGUI_TriangleFilter::SetActor
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_TriangleFilter::GetId() const
{
  return SMESH::TriaFilter;
}

//=======================================================================
// name    : SMESHGUI_TriangleFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_TriangleFilter::IsNodeFilter() const
{
  return false;
}

/*
  Class       : SMESHGUI_FacesFilter
  Description : Verify whether selected cell is any face
*/


//=======================================================================
// name    : SMESHGUI_FacesFilter::SMESHGUI_FacesFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_FacesFilter::SMESHGUI_FacesFilter()
: SMESHGUI_Filter()
{
}

SMESHGUI_FacesFilter::~SMESHGUI_FacesFilter()
{
}

//=======================================================================
// name    : SMESHGUI_FacesFilter::IsValid
// Purpose : Verify whether selected cell is face
//=======================================================================
bool SMESHGUI_FacesFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( anActor->GetElemObjId( theCellId ) );

  return anElem && anElem->GetType() == SMDSAbs_Face;
}

//=======================================================================
// name    : SMESHGUI_FacesFilter::IsValid
// Purpose : Verify whether selected cell is face
//=======================================================================
bool SMESHGUI_FacesFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( theObjId );

  return anElem && anElem->GetType() == SMDSAbs_Face;
}

//=======================================================================
// name    : SMESHGUI_FacesFilter::GetId
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_FacesFilter::GetId() const
{
  return SMESH::FaceFilter;
}

//=======================================================================
// name    : SMESHGUI_FacesFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_FacesFilter::IsNodeFilter() const
{
  return false;
}


/*
  Class       : SMESHGUI_VolumesFilter
  Description : Verify whether selected cell is any volume
*/


//=======================================================================
// name    : SMESHGUI_VolumesFilter::SMESHGUI_VolumesFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_VolumesFilter::SMESHGUI_VolumesFilter()
: SMESHGUI_Filter()
{
}

SMESHGUI_VolumesFilter::~SMESHGUI_VolumesFilter()
{
}

//=======================================================================
// name    : SMESHGUI_VolumesFilter::IsValid
// Purpose : Verify whether selected cell is volume
//=======================================================================
bool SMESHGUI_VolumesFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 || theCellId < 1 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( anActor->GetElemObjId( theCellId ) );

  return anElem && anElem->GetType() == SMDSAbs_Volume;
}

//=======================================================================
// name    : SMESHGUI_VolumesFilter::IsValid
// Purpose : Verify whether selected cell is volume
//=======================================================================
bool SMESHGUI_VolumesFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( theObjId );

  return anElem && anElem->GetType() == SMDSAbs_Volume;
}

//=======================================================================
// name    : SMESHGUI_VolumesFilter::GetId
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_VolumesFilter::GetId() const
{
  return SMESH::VolumeFilter;
}

//=======================================================================
// name    : SMESHGUI_VolumesFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_VolumesFilter::IsNodeFilter() const
{
  return false;
}


/*
  Class       : SMESHGUI_VolumeShapeFilter
  Description : Verify whether selected cell is a volume of a certain shape
*/


//=======================================================================
// name    : SMESHGUI_VolumeShapeFilter::SMESHGUI_VolumeShapeFilter
// Purpose : Constructor
//=======================================================================
SMESHGUI_VolumeShapeFilter::SMESHGUI_VolumeShapeFilter(const SMDSAbs_GeometryType shape)
  : SMESHGUI_Filter(), myGeometryType( shape )
{
}

//=======================================================================
// name    : SMESHGUI_VolumeShapeFilter::IsValid
// Purpose : Verify whether selected cell is a volume of a certain shape
//=======================================================================
bool SMESHGUI_VolumeShapeFilter::IsValid( const int theCellId ) const
{
  if ( myActor == 0 || theCellId < 1 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( anActor->GetElemObjId( theCellId ) );

  return anElem && anElem->GetGeomType() == myGeometryType;
}

//=======================================================================
// name    : SMESHGUI_VolumeShapeFilter::IsValid
// Purpose : Verify whether selected cell is volume
//=======================================================================
bool SMESHGUI_VolumeShapeFilter::IsObjValid( const int theObjId ) const
{
  if ( myActor == 0 )
    return false;

  SMESH_Actor* anActor = dynamic_cast< SMESH_Actor* >( myActor );
  if ( !anActor || anActor->GetObject() == 0 )
    return false;

  SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();
  const SMDS_MeshElement* anElem = aMesh->FindElement( theObjId );

  return anElem && anElem->GetGeomType() == myGeometryType;
}

//=======================================================================
// name    : SMESHGUI_VolumeShapeFilter::GetId
// Purpose : Get ID of the filter. Must return value from SMESHGUI_FilterType
//           enumeration. All filters must have different ids
//=======================================================================
int SMESHGUI_VolumeShapeFilter::GetId() const
{
  return GetId( myGeometryType );
}

//=======================================================================
//function : GetId
//purpose  : Compose filter ID basing on 
//=======================================================================

int SMESHGUI_VolumeShapeFilter::GetId( SMDSAbs_GeometryType shape )
{
  return SMESH::FirstGeometryTypeFilter + shape;
}

//=======================================================================
// name    : SMESHGUI_VolumeShapeFilter::IsNodeFilter
// Purpose : Returns true if filter is intended for nodes
//=======================================================================
bool SMESHGUI_VolumeShapeFilter::IsNodeFilter() const
{
  return false;
}
