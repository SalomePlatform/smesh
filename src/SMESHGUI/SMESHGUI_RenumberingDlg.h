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
// File   : SMESHGUI_RenumberingDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_RENUMBERINGDLG_H
#define SMESHGUI_RENUMBERINGDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;

//=================================================================================
// class    : SMESHGUI_RenumberingDlg
// purpose  : If the unit == 0 nodes will be renumbered, if the unit == 1 the elements will.
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_RenumberingDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_RenumberingDlg( SMESHGUI*, const int = 0 );
  ~SMESHGUI_RenumberingDlg();

private:
  void                          Init();
  void                          enterEvent( QEvent* );         /* mouse enter the QWidget */
  void                          keyPressEvent( QKeyEvent* );
  
  SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */
  int                           myConstructorId;         /* Current constructor id = radio button id */
  QLineEdit*                    myEditCurrentArgument;   /* Current  LineEdit */
  
  int myUnit;    
  SMESH::SMESH_Mesh_var         myMesh;
  //Handle(SMESH_TypeFilter)      myMeshFilter;
  SUIT_SelectionFilter*         myMeshFilter;
  
  QGroupBox*                    GroupConstructors;
  QRadioButton*                 Constructor1;
  QGroupBox*                    GroupButtons;
  QPushButton*                  buttonOk;
  QPushButton*                  buttonCancel;
  QPushButton*                  buttonApply;
  QPushButton*                  buttonHelp;
  QGroupBox*                    GroupMesh;
  QLabel*                       TextLabelMesh;
  QPushButton*                  SelectButton;
  QLineEdit*                    LineEditMesh;
  
  QString                       myHelpFileName;

protected slots:
  virtual void                  reject();

private slots:
  void                          ClickOnOk();
  void                          ClickOnApply();
  void                          ClickOnHelp();
  void                          SetEditCurrentArgument();
  void                          SelectionIntoArgument();
  void                          DeactivateActiveDialog();
  void                          ActivateThisDialog();
};

#endif // SMESHGUI_RENUMBERINGDLG_H
