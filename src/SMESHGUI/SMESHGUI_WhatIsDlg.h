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
//  File   : SMESHGUI_WhatIsDlg.h
//  Author : Vladimir TURIN
//  Module : SMESH
//  $Header: 

#ifndef SMESHGUI_WHATISDLG_H
#define SMESHGUI_WHATISDLG_H

#include "LightApp_SelectionMgr.h"

#include "SMESH_LogicalFilter.hxx"

// QT Includes
#include <qdialog.h>

class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QTextBrowser;
class SMESHGUI;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)


//=================================================================================
// class    : SMESHGUI_WhatIsDlg
// purpose  :
//=================================================================================
class SMESHGUI_WhatIsDlg : public QDialog
{ 
  Q_OBJECT
    
public:

  SMESHGUI_WhatIsDlg( SMESHGUI*,
		      const char* name = 0,
		      bool modal = FALSE,
		      WFlags fl = 0);
  ~SMESHGUI_WhatIsDlg();
  
private:

  void Init (bool ResetControls = true);
  void closeEvent (QCloseEvent*);
  void enterEvent (QEvent*);                             /* mouse enter the QWidget */
  void hideEvent (QHideEvent*);                          /* ESC key */
  void keyPressEvent(QKeyEvent*);

  SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */

  SVTK_Selector*                mySelector;

  bool                          myBusy;
  SMESH::SMESH_Mesh_var         myMesh;
  SMESH_Actor*                  myActor;
  SMESH_LogicalFilter*          myMeshOrSubMeshOrGroupFilter;

  QButtonGroup* GroupSelections;
  QRadioButton* RadioButtonNodes;
  QRadioButton* RadioButtonElements;
  QGroupBox* GroupButtons;
  QPushButton* buttonOk;
  QPushButton* buttonHelp;
  QGroupBox* GroupArguments;
  QGroupBox* GroupMesh;
  QLabel* TextLabelElements;
  QLineEdit* LineEditElements;
  QLabel* MeshLabel;
  QLabel* MeshName;

  QTextBrowser* Info;
    
  QString myHelpFileName;

private slots:

  void SelectionsClicked(int selectionId);
  void ClickOnOk();
  void ClickOnCancel();
  void ClickOnHelp();
  void SelectionIntoArgument() ;
  void DeactivateActiveDialog() ;
  void ActivateThisDialog() ;
  void onTextChange(const QString&);
    
protected:

  QGridLayout* SMESHGUI_WhatIsDlgLayout;
  QGridLayout* GroupSelectionsLayout;
  QGridLayout* GroupMeshLayout;
  QGridLayout* GroupButtonsLayout;
  QGridLayout* GroupArgumentsLayout;
};

#endif // SMESHGUI_WHATISDLG_H
