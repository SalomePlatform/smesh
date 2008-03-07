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
//  File   : SMESHGUI_BuildCompoundDlg.h
//  Author : Alexander KOVALEV
//  Module : SMESH

#ifndef SMESHGUI_BuildCompoundDlg_H
#define SMESHGUI_BuildCompoundDlg_H

#include "LightApp_SelectionMgr.h"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>

#include <vector>

// Open CASCADE Includes

class QGridLayout;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI;
class QCheckBox;
class SMESHGUI_SpinBox;
class QComboBox;
class QSizePolicy;
class QString;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Gen)

//=================================================================================
// class    : SMESHGUI_BuildCompoundDlg
// purpose  :
//=================================================================================
class SMESHGUI_BuildCompoundDlg : public QDialog
{
  Q_OBJECT;

 public:

  SMESHGUI_BuildCompoundDlg (SMESHGUI  * theModule);
  ~SMESHGUI_BuildCompoundDlg();

public :
  static QString GetDefaultName(const QString& theOperation);

 private:
  void Init();
  void closeEvent (QCloseEvent*);
  void enterEvent (QEvent*);              /* mouse enter the QWidget */
  void hideEvent  (QHideEvent*);          /* ESC key */
  void keyPressEvent(QKeyEvent*);

 private:
  SMESHGUI*               mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*  mySelectionMgr; /* User shape selection */

  SMESH::SMESH_Mesh_var   myMesh;
  SUIT_SelectionFilter*   myMeshFilter;
  SMESH::mesh_array_var   myMeshArray;

  // Widgets
  QButtonGroup* GroupConstructors;
  QRadioButton* Constructor1;

  QGroupBox* GroupButtons;
  QPushButton* buttonOk;
  QPushButton* buttonCancel;
  QPushButton* buttonApply;
  QPushButton* buttonHelp;

  QGroupBox* GroupName;
  QLabel* TextLabelName;
  QLineEdit* LineEditName;

  QGroupBox* GroupArgs;
  QLabel* TextLabelMeshes;
  QPushButton* SelectButton;
  QLineEdit* LineEditMeshes;
  QLabel* TextLabelUnion;
  QComboBox* ComboBoxUnion;
  QCheckBox* CheckBoxCommon;
  QCheckBox* CheckBoxMerge;
  QLabel* TextLabelTol;
  SMESHGUI_SpinBox* SpinBoxTol;

  //protected:
  QGridLayout* SMESHGUI_BuildCompoundDlgLayout;
  QGridLayout* GroupConstructorsLayout;
  QGridLayout* GroupButtonsLayout;
  QGridLayout* GroupNameLayout;
  QGridLayout* GroupArgsLayout;

  QString myHelpFileName;

 private slots:
  void ClickOnOk();
  void ClickOnCancel();
  bool ClickOnApply();
  void ClickOnHelp();
  void SelectionIntoArgument();
  void DeactivateActiveDialog();
  void ActivateThisDialog();
  void onSelectMerge(bool);
};

#endif // SMESHGUI_BuildCompoundDlg_H
