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
//  File   : SMESH_NumberFilter.hxx
//  Author : Sergey LITONIN
//  Module : SMESH

#ifndef _SMESH_NumberFilter_HeaderFile
#define _SMESH_NumberFilter_HeaderFile

#include <Standard_DefineHandle.hxx>
#include "SALOME_TypeFilter.hxx"
#include <TopAbs_ShapeEnum.hxx>
#include <TColStd_MapOfInteger.hxx>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(GEOM_Gen)

DEFINE_STANDARD_HANDLE( SMESH_NumberFilter, SALOME_TypeFilter )

/*
  Class       : SMESH_NumberFilter
  Description : Filter for geom or smesh objects.
                Filter geom objects by number of subshapes of the given type
                Parameters of constructor:
                * theSubShapeType - Type of subshape
                * theNumber       - Number of subshapes. Object is selected if it contains theNumber of
                                    theSubShapeType sub-shapes
                * theShapeType    - This map specifies types of object to be selected
                * theMainObject   - Sub-shapes of this object is selected only
                * theIsClosedOnly - Closed shapes is selected if this parameter is true
*/
class SMESH_NumberFilter : public SALOME_TypeFilter
{

public:

                        SMESH_NumberFilter( const char*            theKind,
                                            const TopAbs_ShapeEnum theSubShapeType,
                                            const int              theNumber,
                                            const TopAbs_ShapeEnum theShapeType = TopAbs_SHAPE,
                                            GEOM::GEOM_Object_ptr  theMainObj = GEOM::GEOM_Object::_nil(),
                                            const bool             theIsClosedOnly = false );

                        SMESH_NumberFilter( const char*                 theKind,
                                            const TopAbs_ShapeEnum      theSubShapeType,
                                            const int                   theNumber,
                                            const TColStd_MapOfInteger& theShapeTypes,
                                            GEOM::GEOM_Object_ptr       theMainObj = GEOM::GEOM_Object::_nil(),
                                            const bool                  theIsClosedOnly = false );
                                        
  virtual               ~SMESH_NumberFilter();
  virtual               Standard_Boolean IsOk( const Handle(SALOME_InteractiveObject)& ) const;
  
  void                  SetSubShapeType( const TopAbs_ShapeEnum );
  void                  SetNumber( const int );
  void                  SetClosedOnly( const bool );
  void                  SetShapeType( const TopAbs_ShapeEnum );
  void                  SetShapeTypes( const TColStd_MapOfInteger& );
  void                  SetMainShape( GEOM::GEOM_Object_ptr );

private:
  GEOM::GEOM_Object_ptr getGeom( const Handle(SALOME_InteractiveObject)& theObj ) const;

                                  
private:
  TopAbs_ShapeEnum      mySubShapeType;
  int                   myNumber;
  bool                  myIsClosedOnly;
  TColStd_MapOfInteger  myShapeTypes;
  GEOM::GEOM_Object_var myMainObj;
  
public:
  DEFINE_STANDARD_RTTI( SMESH_NumberFilter )
};

#endif


























