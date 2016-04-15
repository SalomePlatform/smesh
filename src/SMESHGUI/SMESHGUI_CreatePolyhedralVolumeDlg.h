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
// File   : SMESHGUI_CreatePolyhedralVolumeDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_CREATEPOLYHEDRALVOLUMEDLG_H
#define SMESHGUI_CREATEPOLYHEDRALVOLUMEDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QComboBox;
class QGroupBox;
class QListWidget;
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
  struct TPolySimulation;
}

//=================================================================================
// class    : SMESHGUI_CreatePolyhedralVolumeDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_CreatePolyhedralVolumeDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_CreatePolyhedralVolumeDlg( SMESHGUI* );
  ~SMESHGUI_CreatePolyhedralVolumeDlg();

private:
  typedef QList<SMESH::SMESH_GroupBase_var> GrpList;

  void                     Init();
  void                     enterEvent( QEvent* );          /* mouse enter the QWidget */
  void                     keyPressEvent( QKeyEvent* );
  int                      GetConstructorId();
  void                     displaySimulation();
    
  bool                     isValid();
  
  int                      checkEditLine( bool = true ); /*! Checking for indices, return 1 if all ok, esle -1*/
    
  SMESHGUI*                mySMESHGUI;                   /* Current SMESHGUI object */
  LightApp_SelectionMgr*   mySelectionMgr;               /* User shape selection */
  SVTK_Selector*           mySelector;
  QString                  myIDs;
  int                      myNbOkElements;               /* to check when elements are defined */

  QLineEdit*               myEditCurrentArgument;
   
  SMESH::SMESH_Mesh_var    myMesh;
  SMESH_Actor*             myActor;
  SMESH::TPolySimulation*  mySimulation;
  QString                  myEntry;
  GrpList                  myGroups;
  
  QGroupBox*               ConstructorsBox;
  QButtonGroup*            GroupConstructors;
  QRadioButton*            RadioButton1;
  QRadioButton*            RadioButton2;
  QCheckBox*               Preview;
  QGroupBox*               GroupGroups;
  QLabel*                  TextLabel_GroupName;
  QComboBox*               ComboBox_GroupName;
  QGroupBox*               GroupButtons;
  QPushButton*             buttonOk;
  QPushButton*             buttonCancel;
  QPushButton*             buttonApply;
  QPushButton*             buttonHelp;
  QGroupBox*               GroupContent;
  QLabel*                  TextLabelIds;
  QPushButton*             SelectElementsButton;
  QLineEdit*               LineEditElements;
  QListWidget*             myFacesByNodes;
  QLabel*                  myFacesByNodesLabel;
  QPushButton*             AddButton;
  QPushButton*             RemoveButton;

  QString                  myHelpFileName;
       
public slots:
  void                     onAdd();
  void                     onRemove();

protected slots:
  virtual void             reject();

private slots:
  void                     ConstructorsClicked( int );
  void                     ClickOnPreview( bool );
  void                     ClickOnOk();
  void                     ClickOnApply();
  void                     ClickOnHelp();
  void                     SetEditCurrentArgument();
  void                     SelectionIntoArgument();
  void                     DeactivateActiveDialog();
  void                     ActivateThisDialog();
  void                     onTextChange( const QString& );
  void                     onListSelectionChanged();
  void                     onOpenView();
  void                     onCloseView();
};

#endif // SMESHGUI_CREATEPOLYHEDRALVOLUMEDLG_H
