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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_GroupDlg.h
// Author : Natalia KOPNOVA, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_GROUPDLG_H
#define SMESHGUI_GROUPDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>
#include <QList>
#include <QMap>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)

class QGroupBox;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QListWidget;
class QMenu;
class QPushButton;
class QToolButton;
class QCheckBox;
class QStackedWidget;
class QAction;
class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_FilterDlg;
class SMESHGUI_ShapeByMeshOp;
class QtxColorButton;
class SUIT_Operation;
class SVTK_Selector;
class SUIT_SelectionFilter;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_GroupDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_GroupDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_GroupDlg( SMESHGUI*,
                     SMESH::SMESH_Mesh_ptr = SMESH::SMESH_Mesh::_nil() );
  SMESHGUI_GroupDlg( SMESHGUI*,
                     SMESH::SMESH_GroupBase_ptr,
                     const bool theIsConvert = false );
  ~SMESHGUI_GroupDlg();
  
  static QString                GetDefaultName( const QString& );
  
public slots:
  void onAdd();
  void onRemove();

protected slots:
  virtual void                  reject();

private slots:
  void                          onTypeChanged( int );
  void                          onGrpTypeChanged( int );
  void                          onColorChanged( QColor );
  
  void                          onOK();
  bool                          onApply();
  void                          onHelp();
  void                          onDeactivate();
  void                          onVisibilityChanged();
  
  void                          onListSelectionChanged();
  void                          onObjectSelectionChanged();
  
  void                          onSelectAll();
  void                          onSelectSubMesh( bool );
  void                          onSelectGroup( bool );
  void                          onSelectGeomGroup( bool );
  void                          setCurrentSelection();
  
  void                          setFilters();
  void                          onSort();
  
  void                          onNameChanged( const QString& );
  void                          onFilterAccepted();
  
  void                          onGeomPopup( QAction* );
  void                          onGeomSelectionButton( bool );
  
  void                          onPublishShapeByMeshDlg( SUIT_Operation* );
  void                          onCloseShapeByMeshDlg( SUIT_Operation* );

  void                          onOpenView();
  void                          onCloseView();

private:
  void                          initDialog( bool );
  void                          init( SMESH::SMESH_Mesh_ptr );
  void                          init( SMESH::SMESH_GroupBase_ptr,
                                      const bool theIsConvert = false );
  void                          enterEvent( QEvent* );
  void                          keyPressEvent( QKeyEvent* );
  void                          setSelectionMode( int );
  void                          updateButtons();
  void                          updateGeomPopup();
  bool                          SetAppropriateActor();
  void                          setShowEntityMode();
  void                          restoreShowEntityMode();

  bool                          IsActorVisible( SMESH_Actor* );
  
  void                          setGroupColor( const SALOMEDS::Color& );
  SALOMEDS::Color               getGroupColor() const;
  
  void                          setGroupQColor( const QColor& );
  QColor                        getGroupQColor() const;
  void                          setDefaultName() const;
  void                          setDefaultGroupColor();

  void                          setIsApplyAndClose( const bool theFlag );
  bool                          isApplyAndClose() const;

 private:

  SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */
  int                           myGrpTypeId;             /* Current group type id : standalone or group on geometry */
  int                           myTypeId;                /* Current type id = radio button id */
  int                           myStoredShownEntity;     /* Store ShowEntity mode of myMesh */
  QLineEdit*                    myCurrentLineEdit;       /* Current  LineEdit */
  SVTK_Selector*                mySelector;
  
  QPushButton*                  myMeshGroupBtn;
  QLineEdit*                    myMeshGroupLine;
  
  QButtonGroup*                 myTypeGroup;
  QLineEdit*                    myName;
  QString                       myOldName;
  
  QButtonGroup*                 myGrpTypeGroup;
  
  QStackedWidget*               myWGStack;
  QCheckBox*                    mySelectAll;
  QCheckBox*                    myAllowElemsModif;
  QLabel*                       myElementsLab;
  QListWidget*                  myElements;
  QPushButton*                  myFilterBtn;
  QPushButton*                  myAddBtn;
  QPushButton*                  myRemoveBtn;
  QPushButton*                  mySortBtn;
  
  QGroupBox*                    mySelectBox;
  QCheckBox*                    mySelectSubMesh;
  QPushButton*                  mySubMeshBtn;
  QLineEdit*                    mySubMeshLine;
  QCheckBox*                    mySelectGroup;
  QPushButton*                  myGroupBtn;
  QLineEdit*                    myGroupLine;
  
  QtxColorButton*               myColorBtn;
  
  QCheckBox*                    mySelectGeomGroup;
  QToolButton*                  myGeomGroupBtn;
  QLineEdit*                    myGeomGroupLine;
  QMenu*                        myGeomPopup;

  QPushButton*                  myOKBtn;
  QPushButton*                  myApplyBtn;
  QPushButton*                  myCloseBtn;
  QPushButton*                  myHelpBtn;
  
  SMESHGUI_ShapeByMeshOp*       myShapeByMeshOp;
  
  SMESH::SMESH_Mesh_var         myMesh;
  QList<SMESH_Actor*>           myActorsList;
  SMESH::SMESH_Group_var        myGroup;
  SMESH::SMESH_GroupOnGeom_var  myGroupOnGeom;
  SMESH::SMESH_GroupOnFilter_var myGroupOnFilter;
  SMESH::Filter_var             myFilter;
  QList<int>                    myIdList;
  GEOM::ListOfGO_var            myGeomObjects;
  
  int                           mySelectionMode;
  //Handle(SMESH_TypeFilter)      myMeshFilter;
  //Handle(SMESH_TypeFilter)      mySubMeshFilter;
  //Handle(SMESH_TypeFilter)      myGroupFilter;
  SUIT_SelectionFilter*         myMeshFilter;
  SMESH_LogicalFilter*          mySubMeshFilter;
  SMESH_LogicalFilter*          myGroupFilter;
  SUIT_SelectionFilter*         myGeomFilter;
  
  SMESHGUI_FilterDlg*           myFilterDlg;
  
  bool                          myCreate, myIsBusy;
  
  QString                       myHelpFileName;
  
  QMap<QAction*, int>           myActions;

  bool                          myNameChanged; //added by skl for IPAL19574
  int                           myNbChangesOfContents; // nb add's and remove's

  QString                       myObjectToSelect;
  bool                          myIsApplyAndClose;
};

#endif // SMESHGUI_GROUPDLG_H
