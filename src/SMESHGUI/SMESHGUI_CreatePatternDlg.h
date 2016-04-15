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

// File   : SMESHGUI_CreatePatternDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_CREATEPATTERNDLG_H
#define SMESHGUI_CREATEPATTERNDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Pattern)

class SMESHGUI_PatternWidget;
class SALOMEDSClient_SObject;

class QLineEdit;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESHGUI;

/*!
 *  Class       : SMESHGUI_CreatePatternDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

class SMESHGUI_EXPORT SMESHGUI_CreatePatternDlg : public QDialog
{
  Q_OBJECT

public:
  enum { Type_2d, Type_3d };

public:
  SMESHGUI_CreatePatternDlg( SMESHGUI*, const int );
  virtual ~SMESHGUI_CreatePatternDlg();

  void                     Init( const int );
  QString                  GetPatternName() const;
  SMESH::SMESH_Pattern_ptr GetPattern();
  void                     SetMesh( SMESH::SMESH_Mesh_ptr );

signals:
  void                     NewPattern();
  void                     Close();

private:
  void                     enterEvent( QEvent* );
  void                     keyPressEvent( QKeyEvent* );
 
private slots:
  void                     onOk();
  void                     onSave();
  void                     reject();
  void                     onHelp();

  void                     onDeactivate();

  void                     onSelectionDone();
  void                     onTypeChanged( int );
  void                     onProject( bool );
  void                     onSelBtnClicked();

private:
  QWidget*                 createButtonFrame( QWidget* );
  QWidget*                 createMainFrame( QWidget* );
  void                     displayPreview();
  void                     erasePreview();
  void                     activateSelection();
  QString                  autoExtension( const QString& ) const;
  bool                     isValid();
  bool                     loadFromObject( const bool = true );
  QString                  getDefaultName() const;
  GEOM::GEOM_Object_ptr    getGeom( SALOMEDSClient_SObject* ) const;

private:
  QButtonGroup*            myTypeGrp;
  QRadioButton*            mySwitch2d;
  QRadioButton*            mySwitch3d;

  QLineEdit*               myMeshEdit;
  QLineEdit*               myName;

  SMESHGUI_PatternWidget*  myPicture2d;

  QCheckBox*               myProjectChk;

  SMESHGUI*                mySMESHGUI;
  LightApp_SelectionMgr*   mySelectionMgr;
  int                      myType;

  SMESH::SMESH_Mesh_var    myMesh;
  SMESH::SMESH_subMesh_var mySubMesh;
  GEOM::GEOM_Object_var    myGeomObj;

  SMESH::SMESH_Pattern_var myPattern;
  bool                     myIsLoaded;

  QString                  myHelpFileName;
};

#endif // SMESHGUI_CREATEPATTERNDLG_H
