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
// File   : SMESHGUI_SmoothingDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SMOOTHINGDLG_H
#define SMESHGUI_SMOOTHINGDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QComboBox;
class QCheckBox;
class SMESHGUI;
class SalomeApp_IntSpinBox;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_SmoothingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SmoothingDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_SmoothingDlg( SMESHGUI* );
  ~SMESHGUI_SmoothingDlg();

private:
  void                   Init();
  void                   enterEvent( QEvent* );        /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  void                   setFilters( const bool theIsElem );

  bool                   isValid();
  
  SMESHGUI*              mySMESHGUI;              /* Current SMESHGUI object */
  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  int                    myNbOkElements;          /* to check when elements are defined */
  int                    myNbOkNodes;             /* to check when fixed nodes are defined */
  int                    myConstructorId;         /* Current constructor id = radio button id */
  QLineEdit*             myEditCurrentArgument;   /* Current  LineEdit */
  SVTK_Selector*         mySelector;

  SMESH::SMESH_IDSource_var mySelectedObject;

  bool                   myBusy;
  SMESH::SMESH_Mesh_var  myMesh;
  SMESH_Actor*           myActor;
  Handle(SALOME_InteractiveObject) myIO;
  SMESH_LogicalFilter*   myMeshOrSubMeshOrGroupFilter;

  QGroupBox*             GroupConstructors;
  QRadioButton*          Constructor1;
  QGroupBox*             GroupButtons;
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;
  QGroupBox*             GroupArguments;
  QLabel*                TextLabelElements;
  QPushButton*           SelectElementsButton;
  QLineEdit*             LineEditElements;
  QCheckBox*             CheckBoxMesh;
  QLabel*                TextLabelNodes;
  QPushButton*           SelectNodesButton;
  QLineEdit*             LineEditNodes;
  QLabel*                TextLabelMethod;
  QComboBox*             ComboBoxMethod;
  QLabel*                TextLabelLimit;
  SalomeApp_IntSpinBox*  SpinBox_IterationLimit;
  QLabel*                TextLabelAspectRatio;
  SMESHGUI_SpinBox*      SpinBox_AspectRatio;
  QCheckBox*             CheckBoxParametric;

  QString                myHelpFileName;

  QPushButton*           myElemFilterBtn;
  SMESHGUI_FilterDlg*    myFilterDlg;
   
protected slots:
  virtual void           reject();


private slots:
  void                   ClickOnOk();
  bool                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SetEditCurrentArgument();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onSelectMesh( bool );
  void                   onOpenView();
  void                   onCloseView();
  void                   setElemFilters();
  void                   setNodeFilters();
};

#endif // SMESHGUI_SMOOTHINGDLG_H
