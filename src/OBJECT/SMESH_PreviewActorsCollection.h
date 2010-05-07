//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH OBJECT : interactive object for SMESH visualization
//  File   : SMESH_PreviewActorsCollection.h
//  Author : OCN
//  Module : SMESH
//  $Header: /home/server/cvs/SMESH/SMESH_SRC/src/OBJECT/SMESH_PreviewActorsCollection.h,v 1
//
#ifndef SMESH_PREVIEW_ACTOR_COLLECTION_H
#define SMESH_PREVIEW_ACTOR_COLLECTION_H

#include "SMESH_Actor.h"
#include <GEOM_Actor.h>
#include "SMESH_Object.h"
#include <TopoDS_Shape.hxx>
#include <SMDS_SetIterator.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <QString>
#include <QMap>

class vtkRenderer;
class GEOM_Actor;

class SMESHOBJECT_EXPORT SMESH_PreviewActorsCollection
{
 public:
  SMESH_PreviewActorsCollection();
  ~SMESH_PreviewActorsCollection();

  virtual void    AddToRender(vtkRenderer* theRenderer);
  virtual void    RemoveFromRender(vtkRenderer* theRenderer);

  bool            Init( const TopoDS_Shape& theShape, TopAbs_ShapeEnum subShapeType = TopAbs_EDGE, const QString& = QString("") );

  void            SetSelector( SVTK_Selector* );

  void            HighlightAll( bool );
  void            HighlightID( int );

  GEOM_Actor*     GetActorByIndex( int );

  int             GetIndexByShape( const TopoDS_Shape& );

  void            SetShown( bool );

 protected:
   GEOM_Actor*    createActor( const TopoDS_Shape& );

 protected:
  TopoDS_Shape                 myMainShape;
  SVTK_Selector*               mySelector;
  vtkRenderer*                 myRenderer;
  TopTools_IndexedMapOfShape   myMapOfShapes;
  QMap<int, GEOM_Actor*>       myMapOfActors;
};


#endif //SMESH_DEVICE_ACTOR_COLLECTION_H
