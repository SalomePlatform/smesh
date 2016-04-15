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
// File   : SMESHGUI_NodesDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_NODESDLG_H
#define SMESHGUI_NODESDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;

class SMESHGUI;
class SVTK_Selector;
class SMESHGUI_SpinBox;
class LightApp_SelectionMgr;

namespace SMESH 
{
  struct TNodeSimulation;
}

//=================================================================================
// class    : SMESHGUI_NodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_NodesDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_NodesDlg( SMESHGUI* );
  ~SMESHGUI_NodesDlg();

private:
  typedef QList<SMESH::SMESH_GroupBase_var> GrpList;
  
  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_Selector*          mySelector;
  SMESHGUI*               mySMESHGUI;
  
  SMESH::SMESH_Mesh_var   myMesh;
  SMESH::TNodeSimulation* mySimulation;
  QString                 myEntry;
  GrpList                 myGroups;

  void                    Init();
  void                    enterEvent( QEvent* );
  void                    keyPressEvent( QKeyEvent* );
  
  bool                    isValid();

  QGroupBox*              GroupConstructors;
  QRadioButton*           Constructor1;
  QGroupBox*              GroupCoordinates;
  SMESHGUI_SpinBox*       SpinBox_X;
  SMESHGUI_SpinBox*       SpinBox_Y;
  SMESHGUI_SpinBox*       SpinBox_Z;
  
  QLabel*                 TextLabel_X;
  QLabel*                 TextLabel_Y;
  QLabel*                 TextLabel_Z;

  QGroupBox*              GroupGroups;
  QLabel*                 TextLabel_GroupName;
  QComboBox*              ComboBox_GroupName;

  QGroupBox*              GroupButtons;
  QPushButton*            buttonApply;
  QPushButton*            buttonOk;
  QPushButton*            buttonCancel;
  QPushButton*            buttonHelp;

  QString                 myHelpFileName;

protected slots:
  virtual void            reject();

private slots:
  void                    ClickOnOk();
  bool                    ClickOnApply();
  void                    ClickOnHelp();
  void                    DeactivateActiveDialog();
  void                    ActivateThisDialog();
  void                    SelectionIntoArgument();
  void                    ValueChangedInSpinBox( double );
  void                    onOpenView();
  void                    onCloseView();
};

#endif // SMESHGUI_NODESDLG_H
