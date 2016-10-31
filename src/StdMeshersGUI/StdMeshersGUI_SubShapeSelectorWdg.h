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

// File   : StdMeshersGUI_SubShapeSelectorWdg.h
// Author : Open CASCADE S.A.S. (dmv)
//
#ifndef STDMESHERSGUI_SUBSHAPESELECTORWDG_H
#define STDMESHERSGUI_SUBSHAPESELECTORWDG_H

// SMESH includes
#include "SMESHGUI.h"
#include "SMESH_StdMeshersGUI.hxx"
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QWidget>
#include <QStringList>
#include <TopoDS_Shape.hxx>

#include <string>

class SMESHGUI;
class LightApp_SelectionMgr;
class SVTK_Selector;
class QPushButton;
class QLabel;
class QListWidget;
class SMESH_PreviewActorsCollection;
class vtkRenderer;
class SUIT_SelectionFilter;

class STDMESHERSGUI_EXPORT StdMeshersGUI_SubShapeSelectorWdg : public QWidget
{
  Q_OBJECT

public:
  StdMeshersGUI_SubShapeSelectorWdg( QWidget*         parent = 0,
                                     TopAbs_ShapeEnum subShType = TopAbs_EDGE,
                                     const bool       toShowList = true);
  ~StdMeshersGUI_SubShapeSelectorWdg();

  SMESH::long_array_var          GetListOfIDs();
  bool                           SetListOfIDs( SMESH::long_array_var );
  const QList<int>&              GetSelectedIDs() const { return mySelectedIDs; }

  void                           SetGeomShapeEntry( const QString& theEntry,
                                                    const QString& theMainShapeEntry);
  const char*                    GetMainShapeEntry();

  TopoDS_Shape                   GetGeomShape() { return myGeomShape; }
  TopoDS_Shape                   GetMainShape() { return myMainShape; }

  static GEOM::GEOM_Object_var   GetGeomObjectByEntry( const QString& );
  static TopoDS_Shape            GetTopoDSByEntry( const QString& );

  QString                        GetValue() const { return myParamValue; }

  void                           ShowPreview( bool );

  int                            GetListSize() { return myListOfIDs.size(); }

  void                           SetMaxSize(int aMaxSize) { myMaxSize = aMaxSize; }

  vtkRenderer*                   GetRenderer() { return myRenderer; }
  SMESH_PreviewActorsCollection* GetActorCollection() { return myPreviewActor; }
  void                           ClearSelected();
  void                           ActivateSelection( bool );

signals:
  void                           selectionChanged(); // in the list
  void                           shapeSelected();    // globally

private:
  void                           updateState();
  void                           setFilter();
  void                           updateButtons();

private slots:
  void                           onAdd(); 
  void                           onRemove(); 
  void                           onPrevious(); 
  void                           onNext(); 
  void                           selectionIntoArgument();
  void                           onListSelectionChanged();

private:
  void                           init();

private:
  SMESHGUI*                      mySMESHGUI;
  LightApp_SelectionMgr*         mySelectionMgr;          /* User shape selection */
  SVTK_Selector*                 mySelector;
  SMESH::SMESH_Mesh_var          myMesh;
  TopoDS_Shape                   myGeomShape; // shape whose sub-shapes are selected
  TopoDS_Shape                   myMainShape; // main shape of the mesh
  std::string                    myEntry;
  std::string                    myMainEntry;
  vtkRenderer*                   myRenderer;
  
  QListWidget*                   myListWidget;
  QPushButton*                   myAddButton;
  QPushButton*                   myRemoveButton;
  QLabel*                        myInfoLabel;
  QPushButton*                   myPrevButton;
  QPushButton*                   myNextButton;
  QList<int>                     mySelectedIDs;
  QList<int>                     myListOfIDs;
  
  QString                        myParamValue;
  bool                           myIsShown;

  // for manage possible size of myListOfIDs
  int                            myMaxSize;
  // for manage type of selected subshape
  TopAbs_ShapeEnum               mySubShType;
  
  SMESH_PreviewActorsCollection* myPreviewActor;
  QList<SUIT_SelectionFilter*>   myGeomFilters;
  SUIT_SelectionFilter*          myFilter;
};

#endif // STDMESHERSGUI_SUBSHAPESELECTORWDG_H
