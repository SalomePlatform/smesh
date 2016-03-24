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
// File   : SMESHGUI_CopyMeshDlg.h
//
#ifndef SMESHGUI_CopyMeshDLG_H
#define SMESHGUI_CopyMeshDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class SMESHGUI;
class SMESHGUI_IdValidator;
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;

//=================================================================================
// class    : SMESHGUI_CopyMeshDlg
// purpose  : copy some elements or a mesh object into a new mesh
//=================================================================================

class SMESHGUI_EXPORT SMESHGUI_CopyMeshDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_CopyMeshDlg( SMESHGUI* );
  ~SMESHGUI_CopyMeshDlg();

private:
  void                   Init( bool = true );
  void                   enterEvent( QEvent* );           /* mouse enter the QWidget */
  void                   keyPressEvent( QKeyEvent* );
  int                    GetConstructorId();
  void                   setNewMeshName();

  bool                   isValid();

  void                   setIsApplyAndClose( const bool theFlag );
  bool                   isApplyAndClose() const;

  SMESHGUI*              mySMESHGUI;              /* Current SMESHGUI object */
  SMESHGUI_IdValidator*  myIdValidator;
  LightApp_SelectionMgr* mySelectionMgr;          /* User shape selection */
  int                    myNbOkElements;          /* to check when elements are defined */

  SVTK_Selector*         mySelector;

  bool                   myBusy;
  SMESH::SMESH_Mesh_var  myMesh;
  SMESH_Actor*           myActor;
  SUIT_SelectionFilter*  myIdSourceFilter;

  SMESH::SMESH_IDSource_var mySelectedObject;

  QGroupBox*             ConstructorsBox;
  QGroupBox*             GroupArguments;
  QGroupBox*             GroupButtons;
  
  QPushButton*           buttonOk;
  QPushButton*           buttonCancel;
  QPushButton*           buttonApply;
  QPushButton*           buttonHelp;

  QLabel*                myTextLabelElements;
  QLineEdit*             myLineEditElements;
  QLineEdit*             myMeshNameEdit;
  QCheckBox*             myIdSourceCheck;
  QCheckBox*             myCopyGroupsCheck;
  QCheckBox*             myKeepIdsCheck;

  QPushButton*           myFilterBtn;
  SMESHGUI_FilterDlg*    myFilterDlg;
   
  QString                myHelpFileName;

  bool                   myIsApplyAndClose;

protected slots:
  virtual void           reject();

private slots:
  void                   ClickOnOk();
  bool                   ClickOnApply();
  void                   ClickOnHelp();
  void                   SelectionIntoArgument();
  void                   DeactivateActiveDialog();
  void                   ActivateThisDialog();
  void                   onTextChange( const QString& );
  void                   onSelectIdSource( bool );
  void                   setFilters();
  void                   onOpenView();
  void                   onCloseView();
};

#endif // SMESHGUI_CopyMeshDLG_H
