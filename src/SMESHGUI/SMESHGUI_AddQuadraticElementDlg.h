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
// File   : SMESHGUI_AddMeshElementDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_ADDQUADRATICELEMENTDLG_H
#define SMESHGUI_ADDQUADRATICELEMENTDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMDSAbs_ElementType.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QComboBox;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QTableWidget;
class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;

namespace SMESH
{
  struct TElementSimulationQuad;
}

//=================================================================================
// class    : SMESHGUI_AddQuadraticElementDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_AddQuadraticElementDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_AddQuadraticElementDlg( SMESHGUI*, const SMDSAbs_EntityType );
  ~SMESHGUI_AddQuadraticElementDlg();
  
private:
  typedef QList<SMESH::SMESH_GroupBase_var> GrpList;

  void                        Init();
  void                        enterEvent( QEvent* );       /* mouse enter the QWidget */
  void                        keyPressEvent( QKeyEvent* );
  void                        displaySimulation();
  void                        UpdateTable( bool = true );
  bool                        IsValid();
  void                        updateButtons();

  bool                        isValid();
  
  SMESHGUI*                   mySMESHGUI;       /* Current SMESHGUI object */
  LightApp_SelectionMgr*      mySelectionMgr;   /* User shape selection */
  int                         myNbCorners;      /* The required number of corners */
  int                         myNbMidFaceNodes;
  int                         myNbCenterNodes;
  bool                        myBusy;
  SVTK_Selector*              mySelector;
  
  SMESH::SMESH_Mesh_var       myMesh;
  SMESH_Actor*                myActor;
  SMESH::TElementSimulationQuad* mySimulation;
  QString                     myEntry;
  GrpList                     myGroups;
  
  SMDSAbs_EntityType          myGeomType;
  QLineEdit*                  myCurrentLineEdit;
  
  QGroupBox*                  GroupConstructors;
  QRadioButton*               myRadioButton1;
  
  QGroupBox*                  GroupArguments;
  QPushButton*                myCornerSelectButton;
  QLineEdit*                  myCornerNodes;
  QLabel*                     myMidFaceLabel;
  QPushButton*                myMidFaceSelectButton;
  QLineEdit*                  myMidFaceNodes;
  QLabel*                     myCenterLabel;
  QPushButton*                myCenterSelectButton;
  QLineEdit*                  myCenterNode;
  QTableWidget*               myTable;
  QCheckBox*                  myReverseCB;
  
  QGroupBox*                  GroupGroups;
  QLabel*                     TextLabel_GroupName;
  QComboBox*                  ComboBox_GroupName;

  QGroupBox*                  GroupButtons;
  QPushButton*                buttonOk;
  QPushButton*                buttonCancel;
  QPushButton*                buttonApply;
  QPushButton*                buttonHelp;
  
  QString                     myHelpFileName;
  
protected slots:
  virtual void                reject();

private slots:
  void                        onTextChange( const QString& );
  void                        onCellTextChange( int, int );
  void                        onReverse( int );
  void                        onCellDoubleClicked( int, int );
  
  void                        ClickOnOk();
  bool                        ClickOnApply();
  void                        ClickOnHelp();
  void                        SetCurrentSelection();
  void                        SelectionIntoArgument();
  void                        DeactivateActiveDialog();
  void                        ActivateThisDialog();
  void                        onOpenView();
  void                        onCloseView();
};

#endif // SMESHGUI_ADDQUADRATICELEMENTDLG_H
