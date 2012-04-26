// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_DuplicateNodesDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_DUPLICATENODESDLG_H
#define SMESHGUI_DUPLICATENODESDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;

class LightApp_SelectionMgr;

class SMESHGUI;

/*!
  \class SMESHGUI_DuplicateNodesDlg
  \brief Dialog for duplication of nodes.
*/
class SMESHGUI_EXPORT SMESHGUI_DuplicateNodesDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_DuplicateNodesDlg( SMESHGUI* );
  ~SMESHGUI_DuplicateNodesDlg();

private:
  void                    Init();

  bool                    isValid();
  
  void                    closeEvent( QCloseEvent* );
  void                    enterEvent( QEvent* );
  void                    keyPressEvent( QKeyEvent* );
  
private slots:
  void                    onConstructorsClicked( int );
 
  void                    onOk();
  void                    onClose();
  bool                    onApply();
  void                    onHelp();

  void                    onEditCurrentArgument();
  void                    onSelectionChanged();

  void                    onDeactivate();

private:
  QLineEdit*              myCurrentLineEdit;
  
  QButtonGroup*           myGroupConstructors;
  
  QGroupBox*              myGroupArguments;
  QLabel*                 myTextLabel1;
  QLabel*                 myTextLabel2;
  QLabel*                 myTextLabel3;
  QPushButton*            mySelectButton1;
  QPushButton*            mySelectButton2;
  QPushButton*            mySelectButton3;
  QLineEdit*              myLineEdit1;
  QLineEdit*              myLineEdit2;
  QLineEdit*              myLineEdit3;
  QCheckBox*              myCheckBoxNewElemGroup;
  QCheckBox*              myCheckBoxNewNodeGroup;

  QPushButton*            myButtonOk;
  QPushButton*            myButtonApply;
  QPushButton*            myButtonClose;
  QPushButton*            myButtonHelp;

  SMESHGUI*               mySMESHGUI;
  LightApp_SelectionMgr*  mySelectionMgr;

  QList<SMESH::SMESH_GroupBase_var> myGroups1;
  QList<SMESH::SMESH_GroupBase_var> myGroups2;
  QList<SMESH::SMESH_GroupBase_var> myGroups3;

  bool                    myBusy;
  
  QString                 myHelpFileName;
};

#endif // SMESHGUI_DUPLICATENODESDLG_H
