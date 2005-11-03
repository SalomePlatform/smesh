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
//  File   : SMESHGUI_MeshPatternDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_MeshPatternDlg_H
#define SMESHGUI_MeshPatternDlg_H

#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Pattern)

class QCloseEvent;
class QFrame;
class QLineEdit;
class SMESHGUI_SpinBox;
class QPushButton;
class LightApp_SelectionMgr;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QLabel;
class QSpinBox;
class QGroupBox;
class SMESHGUI_CreatePatternDlg;
class SMESHGUI_PatternWidget;
class vtkUnstructuredGrid;
class SALOME_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;
class SMESHGUI;

/*
  Class       : SMESHGUI_MeshPatternDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_MeshPatternDlg : public QDialog
{
  Q_OBJECT

  // Pattern type
  enum { Type_2d, Type_3d };

  // selection input
  enum { Mesh, Object, Vertex1, Vertex2, Ids };

public:
                                      SMESHGUI_MeshPatternDlg( SMESHGUI*,
                                                               const char* = 0 );
  virtual                             ~SMESHGUI_MeshPatternDlg();

  void                                Init();
  
private slots:

  void                                onOk();
  bool                                onApply();
  void                                onClose();

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
  void                                onNodeChanged( int value );

private:

  QFrame*                             createButtonFrame( QWidget* );
  QFrame*                             createMainFrame  ( QWidget* );
  void                                displayPreview();
  vtkUnstructuredGrid*                getGrid();
  void                                erasePreview();
  void                                updateWgState();
  bool                                loadFromFile( const QString& );
  void                                activateSelection();
  QStringList                         prepareFilters() const;
  QString                             autoExtension( const QString& theFileName ) const;
  void                                closeEvent( QCloseEvent* e ) ;
  void                                enterEvent ( QEvent * ) ;
  bool                                isValid( const bool theMess = true );
  void                                resetSelInput();
  bool                                isRefine() const;

  bool                                getIds( QValueList<int>& ) const;
  int                                 getNode( bool = false ) const;

private:

  QPushButton*                        myOkBtn;
  QPushButton*                        myApplyBtn;
  QPushButton*                        myCloseBtn;

  QButtonGroup*                       myTypeGrp;
  QRadioButton*                       mySwitch2d;
  QRadioButton*                       mySwitch3d;

  QCheckBox*                          myRefine;

  QFrame*                             myRefineGrp;
  QSpinBox*                           myNode1;
  QSpinBox*                           myNode2;
  QLabel*                             myNode2Lbl;

  QGroupBox*                          myGeomGrp;
  QMap< int, QPushButton* >           mySelBtn;
  QMap< int, QLineEdit* >             mySelEdit;
  QMap< int, QLabel* >                mySelLbl;

  QLineEdit*                          myName;
  QPushButton*                        myOpenBtn;
  QPushButton*                        myNewBtn;

  QCheckBox*                          myReverseChk;
  QCheckBox*                          myCreatePolygonsChk;
  QCheckBox*                          myCreatePolyedrsChk;
  SMESHGUI_PatternWidget*             myPicture2d;
  QFrame*                             myPicture3d;
  QLabel*                             myPreview3d;

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
  QMap< int, GEOM::GEOM_Object_var >  myGeomObj;
  
  SMESHGUI_CreatePatternDlg*          myCreationDlg;
  SMESH::SMESH_Pattern_var            myPattern;
  SALOME_Actor*                       myPreviewActor;
};

#endif
