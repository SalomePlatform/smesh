//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MoveNodesDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MOVENODESDLG_H
#define SMESHGUI_MOVENODESDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

class QLineEdit;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;
class SALOME_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;

//=================================================================================
// class    : SMESHGUI_MoveNodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_MoveNodesDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_MoveNodesDlg( SMESHGUI* );
  virtual ~SMESHGUI_MoveNodesDlg();
    
  void                          Init();

private slots:
  void                          onOk();
  bool                          onApply();
  void                          onClose();
  void                          onHelp();

  void                          onDeactivate();

  void                          onSelectionDone();
  void                          redisplayPreview();
  void                          onTextChange( const QString& );

private:
  void                          closeEvent( QCloseEvent* );
  void                          enterEvent( QEvent* );
  void                          hideEvent( QHideEvent* );
  void                          keyPressEvent( QKeyEvent* );
  void                          erasePreview();
  QWidget*                      createButtonFrame( QWidget* );
  QWidget*                      createMainFrame  ( QWidget* );
  bool                          isValid( const bool );
  void                          reset();
  void                          updateButtons();

private:
  QPushButton*                  myOkBtn;
  QPushButton*                  myApplyBtn;
  QPushButton*                  myCloseBtn;
  QPushButton*                  myHelpBtn;
  
  QLineEdit*                    myId;
  SMESHGUI_SpinBox*             myX;
  SMESHGUI_SpinBox*             myY;
  SMESHGUI_SpinBox*             myZ;

  LightApp_SelectionMgr*        mySelectionMgr;
  SVTK_Selector*                mySelector;
  SMESHGUI*                     mySMESHGUI;
  
  SALOME_Actor*                 myPreviewActor;
  SMESH_Actor*                  myMeshActor;
  bool                          myBusy;

  QString                       myHelpFileName;
};

#endif // SMESHGUI_MOVENODESDLG_H
