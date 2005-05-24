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
//  File   : SMESHGUI_CreatePatternDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_CreatePatternDlg_H
#define SMESHGUI_CreatePatternDlg_H

#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Pattern)

class SMESHGUI_PatternWidget;
class QCloseEvent;
class QFrame;
class QLineEdit;
class QPushButton;
class SALOME_Selection;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QLabel;

/*
  Class       : SMESHGUI_CreatePatternDlg
  Description : Dialog to specify filters for VTK viewer
*/

class SMESHGUI_CreatePatternDlg : public QDialog
{
  Q_OBJECT

public:
  
  enum { Type_2d, Type_3d };

public:
                              SMESHGUI_CreatePatternDlg( QWidget*,
                                                         SALOME_Selection*,
                                                         const int,
                                                         const char* = 0 );
  virtual                     ~SMESHGUI_CreatePatternDlg();

  void                        Init( SALOME_Selection*, const int );
  QString                     GetPatternName() const;
  SMESH::SMESH_Pattern_ptr    GetPattern();
  void                        SetMesh( SMESH::SMESH_Mesh_ptr );
  
signals:

  void                        NewPattern();
  void                        Close();
    
private:

  void                        closeEvent( QCloseEvent* e ) ;
  void                        enterEvent ( QEvent * ) ;            
  
private slots:

  void                        onOk();
  void                        onSave();
  void                        onClose();

  void                        onDeactivate();

  void                        onSelectionDone();
  void                        onTypeChanged( int );
  void                        onProject( bool );
  void                        onSelBtnClicked();

private:

  QFrame*                     createButtonFrame( QWidget* );
  QFrame*                     createMainFrame  ( QWidget* );
  void                        displayPreview();
  void                        erasePreview();
  void                        activateSelection();
  QString                     autoExtension( const QString& theFileName ) const;
  bool                        isValid();
  bool                        loadFromObject( const bool = true );
  QString                     getDefaultName() const;
  GEOM::GEOM_Object_ptr       getGeom( SALOMEDS::SObject_ptr );

private:

  QPushButton*                myOkBtn;
  QPushButton*                mySaveBtn;
  QPushButton*                myCloseBtn;

  QButtonGroup*               myTypeGrp;
  QRadioButton*               mySwitch2d;
  QRadioButton*               mySwitch3d;
  QRadioButton*               mySwitchSMESH_Pattern3d;

  QLineEdit*                  myMeshEdit;
  QLineEdit*                  myName;

  SMESHGUI_PatternWidget*     myPicture2d;
  
  QCheckBox*                  myProjectChk;
    
  SALOME_Selection*           mySelection;
  int                         myNbPoints;
  int                         myType;

  SMESH::SMESH_Mesh_var       myMesh;
  SMESH::SMESH_subMesh_var    mySubMesh;
  GEOM::GEOM_Object_var       myGeomObj;

  SMESH::SMESH_Pattern_var    myPattern;
  bool                        myIsLoaded;
};

#endif

















