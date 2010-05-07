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
// File   : SMESHGUI_StandardMeshInfosDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_STANDARDMESHINFOSDLG_H
#define SMESHGUI_STANDARDMESHINFOSDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

class QGroupBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextBrowser;
class SMESHGUI;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;

class SMESHGUI_EXPORT SMESHGUI_StandardMeshInfosDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_StandardMeshInfosDlg( SMESHGUI* );
  ~SMESHGUI_StandardMeshInfosDlg();

protected:
  void                     closeEvent( QCloseEvent* );
  void                     keyPressEvent( QKeyEvent* );
  void                     windowActivationChange( bool );
  void                     DumpMeshInfos();

private slots:
  void                     onSelectionChanged();
  void                     DeactivateActiveDialog();
  void                     ActivateThisDialog();
  void                     onStartSelection();
  void                     onHelp();

private:
  SMESHGUI*                mySMESHGUI;
  LightApp_SelectionMgr*   mySelectionMgr; 
  bool                     myStartSelection;
  bool                     myIsActiveWindow;
    
  SUIT_SelectionFilter*    myMeshFilter;

  QLabel*                  myNameLab;
  QPushButton*             mySelectBtn;
  QLineEdit*               myMeshLine;
    
  QTextBrowser*            myInfo;
    
  QGroupBox*               myMeshGroup;
  QGroupBox*               myInfoGroup;
    
  QGroupBox*               myButtonsGroup;
  QPushButton*             myOkBtn;
  QPushButton*             myHelpBtn;
    
  QString                  myHelpFileName;
};

#endif // SMESHGUI_STANDARDMESHINFOSDLG_H
