//  SALOME SALOMEGUI : implementation of desktop and GUI kernel
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//              
//  File   : SMESH_NumberFilter.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESH_NumberFilter.hxx"
#include "GEOMBase.h"
#include "TopTools_MapOfShape.hxx"
#include "TopExp_Explorer.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "QAD_Application.h"
#include "QAD_Desktop.h"


/*
  Class       : SMESH_NumberFilter
  Description : Filter for geom objects.
                Filter geom objects by number of subshapes of the given type
*/

IMPLEMENT_STANDARD_HANDLE( SMESH_NumberFilter, SALOME_TypeFilter )
IMPLEMENT_STANDARD_RTTIEXT( SMESH_NumberFilter, SALOME_TypeFilter )

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Constructor
//=======================================================================
SMESH_NumberFilter::SMESH_NumberFilter( const char*            theKind,
                                        const TopAbs_ShapeEnum theSubShapeType,
                                        const int              theNumber,
                                        const TopAbs_ShapeEnum theShapeType,
                                        GEOM::GEOM_Object_ptr  theMainObj,
                                        const bool             theIsClosedOnly )
: SALOME_TypeFilter( (char*)theKind )
{
  mySubShapeType = theSubShapeType;
  myNumber = theNumber;
  myIsClosedOnly = theIsClosedOnly;
  myShapeTypes.Add( theShapeType );
  myMainObj = GEOM::GEOM_Object::_duplicate( theMainObj );
}

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Constructor
//=======================================================================
SMESH_NumberFilter::SMESH_NumberFilter( const char*                 theKind,
                                        const TopAbs_ShapeEnum      theSubShapeType,
                                        const int                   theNumber,
                                        const TColStd_MapOfInteger& theShapeTypes,
                                        GEOM::GEOM_Object_ptr       theMainObj,
                                        const bool                  theIsClosedOnly )
: SALOME_TypeFilter( (char*)theKind )
{
  mySubShapeType = theSubShapeType;
  myNumber = theNumber;
  myIsClosedOnly = theIsClosedOnly;
  myShapeTypes = theShapeTypes;
  myMainObj = GEOM::GEOM_Object::_duplicate( theMainObj );
}

SMESH_NumberFilter::~SMESH_NumberFilter()
{
}

//=======================================================================
// name    : SMESH_NumberFilter::SMESH_NumberFilter
// Purpose : Verify validity of entry object
//=======================================================================
Standard_Boolean SMESH_NumberFilter::IsOk( const Handle(SALOME_InteractiveObject)& theObj ) const
{
  if ( theObj.IsNull() || !SALOME_TypeFilter::IsOk( theObj ) )
    return false;

  // Get geom object from IO
  GEOM::GEOM_Object_var aGeomObj = getGeom( theObj );
  if ( aGeomObj->_is_nil() )
    return false;

  // Get shape from geom object and verify its parameters
  TopoDS_Shape aShape;
  if ( !GEOMBase::GetShape( aGeomObj, aShape ) ||
        aShape.IsNull() ||
        !myShapeTypes.Contains( aShape.ShapeType() ) ||
        myIsClosedOnly && aShape.ShapeType() == TopAbs_SHELL && !aShape.Closed() )
    return false;
                        
  // Verify whether shape of entry object is sub-shape of myMainObj
  if ( !myMainObj->_is_nil() )
  {
    TopoDS_Shape aMainShape;
    if ( !GEOMBase::GetShape( myMainObj, aMainShape ) )
      return false;

    bool isFound = false;
    TopAbs_ShapeEnum aShapeType = aShape.ShapeType();
    TopExp_Explorer anExp( aMainShape, aShapeType );
    for ( ; anExp.More(); anExp.Next() )
      if ( anExp.Current() == aShape )
      {
        isFound = true;
        break;
      }
    if ( !isFound )
      return false;
  }
  // Verify number of sub-shapes
  if ( mySubShapeType == TopAbs_SHAPE );
    return true;

  int nbShapes = 0;
  TopExp_Explorer anExp2( aShape, mySubShapeType );
  TopTools_MapOfShape aMap;
  for ( ; anExp2.More(); anExp2.Next() )
    aMap.Add( anExp2.Current() );

  return myNumber == aMap.Extent();
}

//=======================================================================
// name    : SMESH_NumberFilter::getGeom
// Purpose : Retrieve geom object from SALOME_InteractiveObject
//=======================================================================
GEOM::GEOM_Object_ptr SMESH_NumberFilter::getGeom(
  const Handle(SALOME_InteractiveObject)& theObj ) const
{
  if ( theObj->isComponentType( "GEOM" ) )
  {
    Standard_Boolean aRes = Standard_False;
    GEOM::GEOM_Object_var aGeomObj = GEOMBase::ConvertIOinGEOMObject( theObj, aRes );
    return aRes ? aGeomObj._retn() : GEOM::GEOM_Object::_nil();
  }
  else
  {
    // Get geom object corresponding to the mesh
    SALOMEDS::Study_var aStudy = QAD_Application::getDesktop()->getActiveStudy()->getStudyDocument();
    SALOMEDS::SObject_var aSO = aStudy->FindObjectID( theObj->getEntry() );
    if ( aSO->_is_nil() )
      return GEOM::GEOM_Object::_nil();

    SALOMEDS::ChildIterator_var anIter = aStudy->NewChildIterator( aSO );
    for( ; anIter->More(); anIter->Next() )
    {
      SALOMEDS::SObject_var aSO = anIter->Value();
      SALOMEDS::SObject_var aRefSO;

      GEOM::GEOM_Object_var aMeshShape = GEOM::GEOM_Object::_narrow(
        aSO->ReferencedObject( aRefSO )? aRefSO->GetObject() : aSO->GetObject() );

      if ( !aMeshShape->_is_nil() )
        return aMeshShape._retn();
    }

    return GEOM::GEOM_Object::_nil();
  }
}


void SMESH_NumberFilter::SetSubShapeType( const TopAbs_ShapeEnum theSubShapeType )
{
  mySubShapeType = theSubShapeType;
}

void SMESH_NumberFilter::SetNumber( const int theNumber )
{
  myNumber = theNumber;
}

void SMESH_NumberFilter::SetClosedOnly( const bool theIsClosedOnly )
{
  myIsClosedOnly = theIsClosedOnly;
}

void SMESH_NumberFilter::SetShapeType( const TopAbs_ShapeEnum theShapeType )
{
  myShapeTypes.Add( theShapeType );
}

void SMESH_NumberFilter::SetMainShape( GEOM::GEOM_Object_ptr theMainObj )
{
  myMainObj = GEOM::GEOM_Object::_duplicate( theMainObj );
}





















