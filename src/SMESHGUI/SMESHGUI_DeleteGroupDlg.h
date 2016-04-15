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

// File   : SMESHGUI_DeleteGroupDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_DELETEGROUPDLG_H
#define SMESHGUI_DELETEGROUPDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include <SALOME_ListIO.hxx>

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

class QPushButton;
class QListWidget;
class SMESHGUI;
class LightApp_SelectionMgr;

/*!
 *  Class       : SMESHGUI_DeleteGroupDlg
 *  Description : Delete groups and their contents
 */

class SMESHGUI_EXPORT SMESHGUI_DeleteGroupDlg : public QDialog
{
  Q_OBJECT

public:
  SMESHGUI_DeleteGroupDlg( SMESHGUI* );
  virtual ~SMESHGUI_DeleteGroupDlg();

  void                    Init ();

private:
  void                    enterEvent( QEvent* );
  void                    keyPressEvent( QKeyEvent* );

protected slots:
  virtual void            reject();

private slots:
  void                    onOk();
  bool                    onApply();
  void                    onHelp();

  void                    onDeactivate();

  void                    onSelectionDone();

private:
  QWidget*                createButtonFrame( QWidget* );
  QWidget*                createMainFrame( QWidget* );
  bool                    isValid();

private:
  QPushButton*            myOkBtn;
  QPushButton*            myApplyBtn;
  QPushButton*            myCloseBtn;
  QPushButton*            myHelpBtn;
  QListWidget*            myListBox;

  SMESHGUI*               mySMESHGUI;
  LightApp_SelectionMgr*  mySelectionMgr;

  SALOME_ListIO           myListGrpIO;
  QList<SMESH::SMESH_GroupBase_var> myListGrp;
  bool                    myBlockSelection;

  QString                 myHelpFileName;
};

#endif // SMESHGUI_DELETEGROUPDLG_H
