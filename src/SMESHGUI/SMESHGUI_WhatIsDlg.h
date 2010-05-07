//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : SMESHGUI_WhatIsDlg.h
// Author : Vladimir TURIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_WHATISDLG_H
#define SMESHGUI_WHATISDLG_H

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
class QTextBrowser;
class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;

//=================================================================================
// class    : SMESHGUI_WhatIsDlg
// purpose  :
//=================================================================================
class SMESHGUI_WhatIsDlg : public QDialog
{ 
  Q_OBJECT
   
public:
  SMESHGUI_WhatIsDlg( SMESHGUI* );
  ~SMESHGUI_WhatIsDlg();
  
private:
  void                          Init( bool = true );
  void                          closeEvent( QCloseEvent* );
  void                          enterEvent( QEvent* );         /* mouse enter the QWidget */
  void                          hideEvent( QHideEvent* );      /* ESC key */
  void                          keyPressEvent( QKeyEvent* );

  SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */

  SVTK_Selector*                mySelector;

  bool                          myBusy;
  SMESH::SMESH_Mesh_var         myMesh;
  SMESH_Actor*                  myActor;
  SMESH_LogicalFilter*          myMeshOrSubMeshOrGroupFilter;

  QGroupBox*                    GroupSelections;
  QRadioButton*                 RadioButtonNodes;
  QRadioButton*                 RadioButtonElements;
  QGroupBox*                    GroupButtons;
  QPushButton*                  buttonOk;
  QPushButton*                  buttonHelp;
  QGroupBox*                    GroupArguments;
  QGroupBox*                    GroupMesh;
  QLabel*                       TextLabelElements;
  QLineEdit*                    LineEditElements;
  QLabel*                       MeshLabel;
  QLineEdit*                    MeshName;

  QTextBrowser*                 Info;
    
  QString                       myHelpFileName;

private slots:
  void                          SelectionsClicked( int );
  void                          ClickOnOk();
  void                          ClickOnCancel();
  void                          ClickOnHelp();
  void                          SelectionIntoArgument();
  void                          DeactivateActiveDialog();
  void                          ActivateThisDialog();
  void                          onTextChange( const QString& );
};

#endif // SMESHGUI_WHATISDLG_H
