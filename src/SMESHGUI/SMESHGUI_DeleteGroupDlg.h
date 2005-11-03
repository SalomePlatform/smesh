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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_DeleteGroupDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_DELETEGROUPDLG_H
#define SMESHGUI_DELETEGROUPDLG_H

#include <qdialog.h>
#include <qlist.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

class QCloseEvent;
class QFrame;
class QPushButton;
class QListBox;

class SMESHGUI;
class SVTK_ViewWindow;
class LightApp_SelectionMgr;

/*!
 *  Class       : SMESHGUI_DeleteGroupDlg
 *  Description : Delete groups and their contents
 */

class SMESHGUI_DeleteGroupDlg : public QDialog
{
  Q_OBJECT

public:
                          SMESHGUI_DeleteGroupDlg( SMESHGUI* );
  virtual                 ~SMESHGUI_DeleteGroupDlg();

  void                    Init ();

private:

  void                    closeEvent (QCloseEvent*);
  void                    enterEvent (QEvent*);

private slots:

  void                    onOk();
  bool                    onApply();
  void                    onClose();

  void                    onDeactivate();

  void                    onSelectionDone();

private:

  QFrame*                 createButtonFrame (QWidget*);
  QFrame*                 createMainFrame   (QWidget*);
  bool                    isValid();

private:

  QPushButton*            myOkBtn;
  QPushButton*            myApplyBtn;
  QPushButton*            myCloseBtn;
  QListBox*               myListBox;

  SMESHGUI*               mySMESHGUI;
  LightApp_SelectionMgr*  mySelectionMgr;

  QValueList<SMESH::SMESH_GroupBase_var> myListGrp;
  bool                                   myBlockSelection;
};

#endif
