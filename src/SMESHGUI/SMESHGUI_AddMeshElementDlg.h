//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : SMESHGUI_AddMeshElementDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_ADDMESHELEMENTDLG_H
#define SMESHGUI_ADDMESHELEMENTDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include <SMDSAbs_ElementType.hxx>

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
class QCheckBox;
class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;

namespace SMESH
{
  struct TElementSimulation;
}

//=================================================================================
// class    : SMESHGUI_AddMeshElementDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_AddMeshElementDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_AddMeshElementDlg( SMESHGUI*, SMDSAbs_ElementType = SMDSAbs_Edge, int = 2 );
  ~SMESHGUI_AddMeshElementDlg();
  
private:
  void                        Init();
  void                        closeEvent( QCloseEvent* );
  void                        hideEvent( QHideEvent* );     /* ESC key */
  void                        enterEvent( QEvent* );        /* mouse enter the QWidget */
  void                        keyPressEvent( QKeyEvent* );
  void                        displaySimulation();
  
  SMESHGUI*                   mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*      mySelectionMgr;          /* User shape selection */
  int                         myNbOkNodes;             /* to check when arguments is defined */
  bool                        myBusy;
  SVTK_Selector*              mySelector;
  
  QLineEdit*                  myEditCurrentArgument;   /* Current  LineEdit */
  
  int                         myElementType;
  int                         myNbNodes;
  bool                        myIsPoly;
  
  SMESH::SMESH_Mesh_var       myMesh;
  SMESH_Actor*                myActor;
  SMESH::TElementSimulation*  mySimulation;
  
  QGroupBox*                  GroupConstructors;
  QRadioButton*               Constructor1;
  QGroupBox*                  GroupButtons;
  QPushButton*                buttonOk;
  QPushButton*                buttonCancel;
  QPushButton*                buttonApply;
  QPushButton*                buttonHelp;
  QGroupBox*                  GroupC1;
  QLabel*                     TextLabelC1A1;
  QPushButton*                SelectButtonC1A1;
  QLineEdit*                  LineEditC1A1;
  QCheckBox*                  Reverse;
  
  QString                     myHelpFileName;
  
private slots:
  void                        ClickOnOk();
  void                        ClickOnCancel();
  void                        ClickOnApply();
  void                        ClickOnHelp();
  void                        SetEditCurrentArgument();
  void                        SelectionIntoArgument();
  void                        DeactivateActiveDialog();
  void                        ActivateThisDialog();
  void                        CheckBox( int );
  void                        onTextChange( const QString& );
};

#endif // SMESHGUI_ADDMESHELEMENTDLG_H
