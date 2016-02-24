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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MergeDlg.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MergeDlg_H
#define SMESHGUI_MergeDlg_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// OCCT includes
#include <gp_XYZ.hxx>
#include <TColStd_MapOfInteger.hxx>

// STL includes
#include <list>
#include <vector>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class LightApp_SelectionMgr;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class SMESHGUI;
class SMESHGUI_IdPreview;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SUIT_SelectionFilter;
class SVTK_Selector;

namespace SMESH
{
  struct TIdPreview;
}

//=================================================================================
// class    : SMESHGUI_MergeDlg
// purpose  : 
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_MergeDlg : public QDialog
{
  Q_OBJECT;

public:
  SMESHGUI_MergeDlg( SMESHGUI*, int );
  ~SMESHGUI_MergeDlg();

  static QPixmap IconFirst();

private:
  void                      Init();
  void                      enterEvent( QEvent* );              /* mouse enter the QWidget */
  void                      keyPressEvent( QKeyEvent* );
  void                      onEditGroup();
  bool                      isKeepNodesIDsSelection();
  bool                      isNewKeepNodesGroup( const char* entry );

  void                      FindGravityCenter( TColStd_MapOfInteger&,
                                               std::vector<int>& , 
                                               std::list<gp_XYZ>& );
  // add the centers of gravity of ElemsIdMap elements to the GrCentersXYZ list

private:
  typedef QList<SMESH::SMESH_GroupBase_var> GrpList;

  SMESHGUI*                 mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*    mySelectionMgr; /* User shape selection */
  SVTK_Selector*            mySelector;
  
  QWidget*                  myEditCurrentArgument;

  SMESH::SMESH_Mesh_var     myMesh;
  SMESH::SMESH_IDSource_var mySubMeshOrGroup;
  SMESH_Actor*              myActor;
  SUIT_SelectionFilter*     myMeshOrSubMeshOrGroupFilter;
  SUIT_SelectionFilter*     mySubMeshOrGroupFilter;

  SMESHGUI_IdPreview*       myIdPreview;

  int                       myAction;
  bool                      myIsBusy;
  int                       myTypeId; // manual(1) or automatic(0)

  // Widgets

  QGroupBox*                GroupButtons;
  QPushButton*              buttonOk;
  QPushButton*              buttonCancel;
  QPushButton*              buttonApply;
  QPushButton*              buttonHelp;

  QGroupBox*                GroupMesh;
  QLabel*                   TextLabelName;
  QPushButton*              SelectMeshButton;
  QLineEdit*                LineEditMesh;

  QWidget*                  NodeSpecWidget;
  SMESHGUI_SpinBox*         SpinBoxTolerance;
  QCheckBox*                SeparateCornersAndMedium;

  QGroupBox*                GroupCoincident;
  //QWidget*                  GroupCoincidentWidget;
  QPushButton*              DetectButton;
  QListWidget*              ListCoincident;
  QPushButton*              AddGroupButton;
  QPushButton*              RemoveGroupButton;
  QCheckBox*                SelectAllCB;
  QCheckBox*                ShowIDs;

  QGroupBox*                GroupEdit;
  QListWidget*              ListEdit;
  QPushButton*              AddElemButton;
  QPushButton*              RemoveElemButton;
  QPushButton*              SetFirstButton;

  QGroupBox*                GroupExclude;
  QListWidget*              ListExclude;

  QGroupBox*                GroupKeep;
  QButtonGroup*             KeepFromButGroup;
  QPushButton*              SelectKeepNodesButton;
  QPushButton*              AddKeepNodesButton;
  QPushButton*              RemoveKeepNodesButton;
  QListWidget*              KeepList;

  QGroupBox*                TypeBox;
  QButtonGroup*             GroupType;
    
  QString                   myHelpFileName;

  QString                   myEntry;
  GrpList                   myGroups;

protected slots:
  virtual void              reject();

 private slots:
  void                      ClickOnOk();
  bool                      ClickOnApply();
  void                      ClickOnHelp();
  void                      updateControls();
  void                      onDetect();
  void                      onAddKeepNode();
  void                      onRemoveKeepNode();
  void                      onSelectKeepNode();
  void                      onKeepNodeSourceChanged(int);
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
  void                      onTypeChanged(int);
  void                      onOpenView();
  void                      onCloseView();
};

#endif // SMESHGUI_MergeDlg_H
