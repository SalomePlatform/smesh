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
// File   : SMESHGUI_EditMeshDlg.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_EDITMESHDLG_H
#define SMESHGUI_EDITMESHDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// OCCT includes
#include <gp_XYZ.hxx>

// STL includes
#include <list>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QListWidget;
class SMESHGUI;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;
class TColStd_MapOfInteger;

namespace SMESH
{
  struct TIdPreview;
}

//=================================================================================
// class    : SMESHGUI_EditMeshDlg
// purpose  : 
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_EditMeshDlg : public QDialog
{
  Q_OBJECT;

public:
  SMESHGUI_EditMeshDlg( SMESHGUI*, int );
  ~SMESHGUI_EditMeshDlg();

private:
  void                      Init();
  void                      closeEvent( QCloseEvent* );
  void                      enterEvent( QEvent* );              /* mouse enter the QWidget */
  void                      hideEvent( QHideEvent* );           /* ESC key */
  void                      keyPressEvent( QKeyEvent* );
  void                      onEditGroup();

  void                      FindGravityCenter( TColStd_MapOfInteger&, 
                                               std::list<gp_XYZ>& );
  // add the centers of gravity of ElemsIdMap elements to the GrCentersXYZ list

private:
  SMESHGUI*                 mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*    mySelectionMgr; /* User shape selection */
  SVTK_Selector*            mySelector;
  
  QWidget*                  myEditCurrentArgument;

  SMESH::SMESH_Mesh_var     myMesh;
  SMESH::SMESH_IDSource_var mySubMeshOrGroup;
  SMESH_Actor*              myActor;
  SUIT_SelectionFilter*     myMeshOrSubMeshOrGroupFilter;

  SMESH::TIdPreview*        myIdPreview;

  int                       myAction;
  bool                      myIsBusy;

  // Widgets
  QGroupBox*                GroupConstructors;
  QRadioButton*             RadioButton;

  QGroupBox*                GroupButtons;
  QPushButton*              buttonOk;
  QPushButton*              buttonCancel;
  QPushButton*              buttonApply;
  QPushButton*              buttonHelp;

  QGroupBox*                GroupMesh;
  QLabel*                   TextLabelName;
  QPushButton*              SelectMeshButton;
  QLineEdit*                LineEditMesh;

  QGroupBox*                GroupCoincident;
  QLabel*                   TextLabelTolerance;
  SMESHGUI_SpinBox*         SpinBoxTolerance;
  QPushButton*              DetectButton;
  QListWidget*              ListCoincident;
  QPushButton*              AddGroupButton;
  QPushButton*              RemoveGroupButton;
  QCheckBox*                SelectAllCB;

  QGroupBox*                GroupEdit;
  QListWidget*              ListEdit;
  QPushButton*              AddElemButton;
  QPushButton*              RemoveElemButton;
  QPushButton*              SetFirstButton;
    
  QString                   myHelpFileName;

 private slots:
  void                      ClickOnOk();
  void                      ClickOnCancel();
  bool                      ClickOnApply();
  void                      ClickOnHelp();
  void                      updateControls();
  void                      onDetect();
  void                      onAddGroup();
  void                      onRemoveGroup();
  void                      onSelectGroup();
  void                      onSelectAll( bool );
  void                      onSelectElementFromGroup();
  void                      onAddElement();
  void                      onRemoveElement();
  void                      onSetFirst();
  void                      SetEditCurrentArgument();
  void                      SelectionIntoArgument();
  void                      DeactivateActiveDialog();
  void                      ActivateThisDialog();
};

#endif // SMESHGUI_EDITMESHDLG_H
