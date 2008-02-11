//  SMESH SMESHGUI : GUI for SMESH component
//
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_ExtrusionDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_EXTRUSION_H
#define DIALOGBOX_EXTRUSION_H

#include "SMESH_SMESHGUI.hxx"

#include "LightApp_SelectionMgr.h"
#include "SUIT_SelectionFilter.h"
#include "Handle_SALOME_InteractiveObject.hxx"

// QT Includes
#include <qdialog.h>

class QGridLayout; 
class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;
class SVTK_ViewWindow;
class SVTK_Selector;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_ExtrusionDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ExtrusionDlg : public QDialog
{
  Q_OBJECT

public:
  SMESHGUI_ExtrusionDlg (SMESHGUI*,
			 bool modal = FALSE);
  ~SMESHGUI_ExtrusionDlg();

private:
  void Init (bool ResetControls = true);
  void enterEvent (QEvent*);                           /* mouse enter the QWidget */
  void keyPressEvent(QKeyEvent*);
  int  GetConstructorId();
  //void closeEvent (QCloseEvent*);
  //void hideEvent (QHideEvent*);                        /* ESC key */

  SMESHGUI*                     mySMESHGUI;            /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;        /* User shape selection */
  QLineEdit*                    myEditCurrentArgument; /* Current  LineEdit */
  //QString                       myElementsId;
  int                           myNbOkElements;        /* to check when elements are defined */
  SVTK_Selector*                mySelector;

  bool                          myBusy;
  SMESH::SMESH_Mesh_var         myMesh;
  SMESH::long_array_var         myElementsId;
  SMESH_Actor*                  myActor;
  Handle(SALOME_InteractiveObject) myIO;
  SUIT_SelectionFilter*         myMeshOrSubMeshOrGroupFilter;

  // widgets
  QButtonGroup*     GroupConstructors;
  QRadioButton*     RadioButton1;
  QRadioButton*     RadioButton2;

  QGroupBox*        GroupArguments;
  QLabel*           TextLabelElements;
  QPushButton*      SelectElementsButton;
  QLineEdit*        LineEditElements;
  QCheckBox*        CheckBoxMesh;
  QLabel*           TextLabelVector;
  QLabel*           TextLabelDx;
  SMESHGUI_SpinBox* SpinBox_Dx;
  QLabel*           TextLabelDy;
  SMESHGUI_SpinBox* SpinBox_Dy;
  QLabel*           TextLabelDz;
  SMESHGUI_SpinBox* SpinBox_Dz;
  QLabel*           TextLabelNbSteps;
  QSpinBox*         SpinBox_NbSteps;
  QCheckBox*        MakeGroupsCheck;

  QGroupBox*        GroupButtons;
  QPushButton*      buttonOk;
  QPushButton*      buttonCancel;
  QPushButton*      buttonApply;
  QPushButton*      buttonHelp;

  QString           myHelpFileName;

private slots:
  void ConstructorsClicked (int);
  void CheckIsEnable();
  void ClickOnOk();
  bool ClickOnApply();
  void ClickOnCancel();
  void ClickOnHelp();
  void SetEditCurrentArgument();
  void SelectionIntoArgument();
  void DeactivateActiveDialog();
  void ActivateThisDialog();
  void onTextChange (const QString&);
  void onSelectMesh (bool toSelectMesh);
};

#endif // DIALOGBOX_EXTRUSION_H
