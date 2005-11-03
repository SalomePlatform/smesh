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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_MoveNodesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_MOVE_NODES_H
#define DIALOGBOX_MOVE_NODES_H

#include "LightApp_SelectionMgr.h"

#include <qdialog.h>

class QLineEdit;
class QPushButton;
class SMESHGUI_SpinBox;
class SALOME_Actor;
class QFrame;

class SMESHGUI;
class SMESH_Actor;
class SUIT_Desktop;
class SVTK_Selector;
class SVTK_ViewWindow;

//=================================================================================
// class    : SMESHGUI_MoveNodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_MoveNodesDlg : public QDialog
{ 
  Q_OBJECT

public:

                                SMESHGUI_MoveNodesDlg(SMESHGUI* theModule,
						      const char* name = 0);
 virtual                        ~SMESHGUI_MoveNodesDlg();
    
 void                           Init();

private slots:

  void                          onOk();
  bool                          onApply();
  void                          onClose();

  void                          onDeactivate();

  void                          onSelectionDone();
  void                          redisplayPreview();
  void                          onTextChange(const QString&);

private:

  void                          closeEvent( QCloseEvent* e ) ;
  void                          enterEvent ( QEvent * ) ;
  void                          hideEvent ( QHideEvent * );
  void                          erasePreview();
  QFrame*                       createButtonFrame( QWidget* );
  QFrame*                       createMainFrame  ( QWidget* );
  bool                          isValid( const bool ) const;
  void                          reset();
  void                          updateButtons();

private:

  QPushButton*                  myOkBtn;
  QPushButton*                  myApplyBtn;
  QPushButton*                  myCloseBtn;
  
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
};

#endif // DIALOGBOX_MOVE_NODES_H
