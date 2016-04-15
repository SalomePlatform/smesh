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
// File   : SMESHGUI_SymmetryDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SYMMETRYDLG_H
#define SMESHGUI_SYMMETRYDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_SymmetryDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SymmetryDlg : public SMESHGUI_MultiPreviewDlg
{ 
  Q_OBJECT

public:
  SMESHGUI_SymmetryDlg( SMESHGUI* );
  ~SMESHGUI_SymmetryDlg();

private:
  void                   Init( bool = true );
  void                   enterEvent( QEvent* );         /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  int                    GetConstructorId();
  bool                   IsMirrorOk();
  void                   setNewMeshName();

  void                   getMirror(SMESH::AxisStruct& theMirror, SMESH::SMESH_MeshEditor::MirrorType& aMirrorType);

  bool                   isValid();

  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  int                    myNbOkElements;          /* to check when elements are defined */
  QString                myElementsId;
  SVTK_Selector*         mySelector;

  QWidget*               myEditCurrentArgument;   /* Current  LineEdit */

  QList<SMESH::SMESH_IDSource_var> myObjects;
  QList<QString>                   myObjectsNames;
  QList<SMESH::SMESH_Mesh_var>     myMeshes;

  bool                   myBusy;
  SMESH_Actor*           myActor;
  SMESH_LogicalFilter*   myMeshOrSubMeshOrGroupFilter;
    
  QGroupBox*             ConstructorsBox;
  QButtonGroup*          GroupConstructors;
  QRadioButton*          RadioButton1;
  QRadioButton*          RadioButton2;
  QRadioButton*          RadioButton3;
  QGroupBox*             GroupButtons;
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;
  QGroupBox*             GroupArguments;
  QGroupBox*             GroupMirror;
  QLabel*                TextLabelElements;
  QPushButton*           SelectElementsButton;
  QLineEdit*             LineEditElements;
  QCheckBox*             CheckBoxMesh;

  QLabel*                TextLabelPoint;
  QPushButton*           SelectPointButton;
  QLabel*                TextLabelX;
  SMESHGUI_SpinBox*      SpinBox_X;
  QLabel*                TextLabelY;
  SMESHGUI_SpinBox*      SpinBox_Y;
  QLabel*                TextLabelZ;
  SMESHGUI_SpinBox*      SpinBox_Z;
  QLabel*                TextLabelVector;
  QPushButton*           SelectVectorButton;
  QLabel*                TextLabelDX;
  SMESHGUI_SpinBox*      SpinBox_DX;
  QLabel*                TextLabelDY;
  SMESHGUI_SpinBox*      SpinBox_DY;
  QLabel*                TextLabelDZ;
  SMESHGUI_SpinBox*      SpinBox_DZ;
    
  QGroupBox*             ActionBox;
  QButtonGroup*          ActionGroup;
  QCheckBox*             MakeGroupsCheck;
  QLineEdit*             LineEditNewMesh;

  QString                myHelpFileName;
   
  QPushButton*           myFilterBtn;
  SMESHGUI_FilterDlg*    myFilterDlg;

protected slots:
  virtual void           onDisplaySimulation( bool );
  virtual void           reject();
  void                   onFilterAccepted();

private slots:
  void                   ConstructorsClicked( int );
  void                   ClickOnOk();
  bool                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SetEditCurrentArgument();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onSelectMesh( bool );
  void                   onVectorChanged();
  void                   onActionClicked( int );
  void                   onOpenView();
  void                   onCloseView();
  void                   setFilters();
};

#endif // SMESHGUI_SYMMETRYDLG_H
