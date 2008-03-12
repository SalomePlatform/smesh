// SMESH SMESHGUI : GUI for SMESH component
//
// Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License. 
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
// File   : SMESHGUI_GroupOpDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//

#ifndef SMESHGUI_GROUPOPDLG_H
#define SMESHGUI_GROUPOPDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

class QPushButton;
class QLineEdit;
class SMESHGUI;
class LightApp_SelectionMgr;
class SVTK_Selector;

/*
  Class       : SMESHGUI_GroupOpDlg
  Description : Perform boolean operations on groups
*/

class SMESHGUI_EXPORT SMESHGUI_GroupOpDlg : public QDialog
{ 
  Q_OBJECT
    
public:
  enum { UNION, INTERSECT, CUT };
    
public:
  SMESHGUI_GroupOpDlg( SMESHGUI*, const int );
  virtual ~SMESHGUI_GroupOpDlg();

  void                      Init();
  
private:
  void                      closeEvent( QCloseEvent* );
  void                      enterEvent( QEvent* );            
  void                      keyPressEvent( QKeyEvent* );
  
private slots:
  void                      onOk();
  bool                      onApply();
  void                      onClose();
  void                      onHelp();

  void                      onDeactivate();
  void                      onSelectionDone();
  void                      onFocusChanged();

private:
  QWidget*                  createButtonFrame( QWidget* );
  QWidget*                  createMainFrame  ( QWidget* );
  bool                      isValid();
  void                      reset();
  
private:
  QPushButton*              myOkBtn;
  QPushButton*              myApplyBtn;
  QPushButton*              myCloseBtn;
  QPushButton*              myHelpBtn;
  
  QLineEdit*                myNameEdit;
  QLineEdit*                myEdit1;
  QLineEdit*                myEdit2;
  QPushButton*              myBtn1;
  QPushButton*              myBtn2;
  
  SMESHGUI*                 mySMESHGUI;
  LightApp_SelectionMgr*    mySelectionMgr;
  int                       myMode;
  SVTK_Selector*            mySelector;
  
  QLineEdit*                myFocusWg;
  
  SMESH::SMESH_GroupBase_var    myGroup1;
  SMESH::SMESH_GroupBase_var    myGroup2;
  
  QString                   myHelpFileName;
};

#endif // SMESHGUI_GROUPOPDLG_H
