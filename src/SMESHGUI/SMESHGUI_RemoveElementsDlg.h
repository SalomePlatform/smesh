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
// File   : SMESHGUI_RemoveElementsDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_REMOVEELEMENTSDLG_H
#define SMESHGUI_REMOVEELEMENTSDLG_H

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
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;

//=================================================================================
// class    : SMESHGUI_RemoveElementsDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_RemoveElementsDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_RemoveElementsDlg( SMESHGUI* );
  ~SMESHGUI_RemoveElementsDlg();

private:
  void                   Init();
  void                   enterEvent( QEvent* );        /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  
  LightApp_SelectionMgr* mySelectionMgr;
  SVTK_Selector*         mySelector;
  SMESHGUI*              mySMESHGUI;
  
  int                    myNbOkElements;           /* to check when arguments is defined */
  int                    myConstructorId;          /* Current constructor id = radio button id */
  QLineEdit*             myEditCurrentArgument;    /* Current  LineEdit */
  
  bool                   myBusy;
  SMESH::SMESH_Mesh_var  myMesh;
  SMESH_Actor*           myActor;
  
  QGroupBox*             GroupConstructors;
  QRadioButton*          Constructor1;
  QGroupBox*             GroupButtons;
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;
  QGroupBox*             GroupC1;
  QLabel*                TextLabelC1A1;
  QPushButton*           SelectButtonC1A1;
  QLineEdit*             LineEditC1A1;
  
  QString                myHelpFileName;

  SMESHGUI_FilterDlg*    myFilterDlg;

protected slots:
  virtual void           reject();

private slots:
  void                   ClickOnOk();
  void                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SetEditCurrentArgument();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onOpenView();
  void                   onCloseView();
  void                   setFilters();
  void                   updateButtons();
};

#endif // SMESHGUI_REMOVEELEMENTSDLG_H
