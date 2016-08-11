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

class QComboBox;
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
class SMESHGUI_SpinBox;

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
  SMESHGUI_AddMeshElementDlg( SMESHGUI*, SMDSAbs_EntityType = SMDSEntity_Edge );
  ~SMESHGUI_AddMeshElementDlg();
  
private:
  void                        Init();
  void                        enterEvent( QEvent* );        /* mouse enter the QWidget */
  void                        keyPressEvent( QKeyEvent* );
  void                        displaySimulation();
  
  bool                        isValid();
  
  typedef QList<SMESH::SMESH_GroupBase_var> GrpList;
  
  SMESHGUI*                   mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*      mySelectionMgr;          /* User shape selection */
  int                         myNbOkNodes;             /* to check when arguments is defined */
  bool                        myBusy;
  SVTK_Selector*              mySelector;
  
  QLineEdit*                  myEditCurrentArgument;   /* Current  LineEdit */
  
  SMDSAbs_EntityType          myGeomType;
  int                         myElementType;
  int                         myNbNodes;
  bool                        myIsPoly;
  
  SMESH::SMESH_Mesh_var       myMesh;
  SMESH_Actor*                myActor;
  SMESH::TElementSimulation*  mySimulation;
  QString                     myEntry;
  GrpList                     myGroups;
  
  QGroupBox*                  GroupConstructors;
  QRadioButton*               Constructor1;
  QGroupBox*                  GroupGroups;
  QLabel*                     TextLabel_GroupName;
  QComboBox*                  ComboBox_GroupName;
  QGroupBox*                  GroupButtons;
  QPushButton*                buttonOk;
  QPushButton*                buttonCancel;
  QPushButton*                buttonApply;
  QPushButton*                buttonHelp;
  QGroupBox*                  GroupC1;
  QLabel*                     TextLabelC1A1;
  QPushButton*                SelectButtonC1A1;
  QLineEdit*                  LineEditC1A1;
  QCheckBox*                  ReverseOrDulicate;
  SMESHGUI_SpinBox*           DiameterSpinBox;
  
  QString                     myHelpFileName;
  
protected slots:
  virtual void                reject();
          void                onDiameterChanged();

private slots:
  void                        ClickOnOk();
  void                        ClickOnApply();
  void                        ClickOnHelp();
  void                        SetEditCurrentArgument();
  void                        SelectionIntoArgument();
  void                        DeactivateActiveDialog();
  void                        ActivateThisDialog();
  void                        CheckBox( int );
  void                        onTextChange( const QString& );
  void                        onOpenView();
  void                        onCloseView();
};

#endif // SMESHGUI_ADDMESHELEMENTDLG_H
