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

// File   : SMESHGUI_MeshPatternDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHPATTERNDLG_H
#define SMESHGUI_MESHPATTERNDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>
#include <QMap>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Pattern)

class QFrame;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QLabel;
class SalomeApp_IntSpinBox;
class vtkUnstructuredGrid;
class SALOME_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESHGUI;
class SMESHGUI_CreatePatternDlg;
class SMESHGUI_PatternWidget;

/*
  Class       : SMESHGUI_MeshPatternDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_EXPORT SMESHGUI_MeshPatternDlg : public QDialog
{
  Q_OBJECT

  // Pattern type
  enum { Type_2d, Type_3d };

  // selection input
  enum { Mesh, Object, Vertex1, Vertex2, Ids };

public:
  SMESHGUI_MeshPatternDlg( SMESHGUI* );
  virtual ~SMESHGUI_MeshPatternDlg();

  void                                Init();
  
protected slots:
  virtual void                        reject();

private slots:
  void                                onOk();
  bool                                onApply();
  void                                onHelp();

  void                                onDeactivate();

  void                                onSelectionDone();
  void                                onSelInputChanged();

  void                                onTypeChanged( int );
  void                                onModeToggled( bool );
  void                                onOpen();
  void                                onNew();
  void                                onReverse( bool );
  void                                onPreview( bool );
  void                                onOkCreationDlg();
  void                                onCloseCreationDlg();
  void                                onTextChanged( const QString& );
  void                                onNodeChanged( int );
  void                                onOpenView();
  void                                onCloseView();

private:
  QWidget*                            createButtonFrame( QWidget* );
  QWidget*                            createMainFrame( QWidget* );
  void                                displayPreview();
  vtkUnstructuredGrid*                getGrid();
  void                                erasePreview();
  void                                updateWgState();
  bool                                loadFromFile( const QString& );
  void                                activateSelection();
  QStringList                         prepareFilters() const;
  QString                             autoExtension( const QString& ) const;
  void                                enterEvent( QEvent* );
  void                                keyPressEvent( QKeyEvent* );
  bool                                isValid( const bool = true );
  void                                resetSelInput();
  bool                                isRefine() const;

  bool                                getIds( QList<int>& ) const;
  int                                 getNode( bool = false ) const;

private:
  QPushButton*                        myOkBtn;
  QPushButton*                        myApplyBtn;
  QPushButton*                        myCloseBtn;
  QPushButton*                        myHelpBtn;

  QButtonGroup*                       myTypeGrp;
  QRadioButton*                       mySwitch2d;
  QRadioButton*                       mySwitch3d;

  QCheckBox*                          myRefine;

  QFrame*                             myRefineGrp;
  SalomeApp_IntSpinBox*               myNode1;
  SalomeApp_IntSpinBox*               myNode2;
  QLabel*                             myNode2Lbl;

  QFrame*                             myGeomGrp;
  QMap<int, QPushButton*>             mySelBtn;
  QMap<int, QLineEdit*>               mySelEdit;
  QMap<int, QLabel*>                  mySelLbl;

  QWidget*                            myMainFrame;

  QLineEdit*                          myName;
  QPushButton*                        myOpenBtn;
  QPushButton*                        myNewBtn;

  QCheckBox*                          myReverseChk;
  QCheckBox*                          myCreatePolygonsChk;
  QCheckBox*                          myCreatePolyedrsChk;
  SMESHGUI_PatternWidget*             myPicture2d;
  QLabel*                             myPicture3d;

  QCheckBox*                          myPreviewChk;

  SMESHGUI*                           mySMESHGUI;
  SVTK_Selector*                      mySelector;
  LightApp_SelectionMgr*              mySelectionMgr;
  int                                 mySelInput;
  int                                 myNbPoints;
  int                                 myType;
  bool                                myIsCreateDlgOpen;
  bool                                myBusy;

  SMESH::SMESH_Mesh_var               myMesh;
  GEOM::GEOM_Object_var               myMeshShape;
  QMap<int, GEOM::GEOM_Object_var>    myGeomObj;
  
  SMESHGUI_CreatePatternDlg*          myCreationDlg;
  SMESH::SMESH_Pattern_var            myPattern;
  SALOME_Actor*                       myPreviewActor;

  QString                             myHelpFileName;
};

#endif // SMESHGUI_MESHPATTERNDLG_H
