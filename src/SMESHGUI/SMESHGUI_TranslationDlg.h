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
// File   : SMESHGUI_TranslationDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_TRANSLATIONDLG_H
#define SMESHGUI_TRANSLATIONDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

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
// class    : SMESHGUI_TranslationDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_TranslationDlg : public SMESHGUI_MultiPreviewDlg
{ 
  Q_OBJECT

public:
  SMESHGUI_TranslationDlg( SMESHGUI* );
  ~SMESHGUI_TranslationDlg();

private:
  void                   Init( bool = true );
  void                   enterEvent( QEvent* );           /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  int                    GetConstructorId();
  void                   setNewMeshName();

  bool                   isValid();

  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  QString                myElementsId;
  int                    myNbOkElements;          /* to check when elements are defined */

  SVTK_Selector*         mySelector;

  QWidget*               myEditCurrentArgument;

  bool                   myBusy;
  SMESH_Actor*           myActor;
  SMESH_LogicalFilter*   myMeshOrSubMeshOrGroupFilter;

  QList<SMESH::SMESH_Mesh_var> myMeshes;
  QList<SMESH::SMESH_IDSource_var> myObjects;
  QList<QString>            myObjectsNames;

  QGroupBox*             ConstructorsBox;
  QButtonGroup*          GroupConstructors;
  QRadioButton*          RadioButton1;
  QRadioButton*          RadioButton2;
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
  QLabel*                TextLabel1;
  QPushButton*           SelectButton1;
  QLabel*                TextLabel1_1;
  SMESHGUI_SpinBox*      SpinBox1_1;
  QLabel*                TextLabel1_2;
  SMESHGUI_SpinBox*      SpinBox1_2;
  QLabel*                TextLabel1_3;
  SMESHGUI_SpinBox*      SpinBox1_3;
  QLabel*                TextLabel2;
  QPushButton*           SelectButton2;
  QLabel*                TextLabel2_1;
  SMESHGUI_SpinBox*      SpinBox2_1;
  QLabel*                TextLabel2_2;
  SMESHGUI_SpinBox*      SpinBox2_2;
  QLabel*                TextLabel2_3;
  SMESHGUI_SpinBox*      SpinBox2_3;
  //QCheckBox* CheckBoxCopy;
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
  void                   onActionClicked( int );
  void                   onOpenView();
  void                   onCloseView();
  void                   setFilters();

};

#endif // SMESHGUI_TRANSLATIONDLG_H
