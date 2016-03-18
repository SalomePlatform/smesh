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
// File   : SMESHGUI_SewingDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SEWINGDLG_H
#define SMESHGUI_SEWINGDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include <vector>

class QButtonGroup;
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
class SalomeApp_IntSpinBox;
class QListWidget;
class QListWidgetItem;

//=================================================================================
// class    : SMESHGUI_SewingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SewingDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_SewingDlg( SMESHGUI* );
  ~SMESHGUI_SewingDlg();

private:
  void                    Init();
  void                    enterEvent( QEvent* );                /* mouse enter the QWidget */
  void                    keyPressEvent( QKeyEvent* );
  int                     GetConstructorId();
  bool                    IsValid();
  void                    UpdateButtons();

  SMESHGUI*               mySMESHGUI;              /* Current SMESHGUI object */
  LightApp_SelectionMgr*  mySelectionMgr;          /* User shape selection */
  int                     myOk1, myOk2, myOk3, myOk4, myOk5, myOk6;    
  QLineEdit*              myEditCurrentArgument;   /* Current  LineEdit */
  SVTK_Selector*          mySelector;
  
  bool                    myBusy;
  SMESH::SMESH_Mesh_var   myMesh;
  SMESH_Actor*            myActor;
  
  QGroupBox*              ConstructorsBox;
  QButtonGroup*           GroupConstructors;
  QRadioButton*           RadioButton1;
  QRadioButton*           RadioButton2;
  QRadioButton*           RadioButton3;
  QRadioButton*           RadioButton4;
  QGroupBox*              GroupButtons;
  QPushButton*            buttonOk;
  QPushButton*            buttonCancel;
  QPushButton*            buttonApply;
  QPushButton*            buttonHelp;
  QGroupBox*              GroupArguments;
  QGroupBox*              SubGroup1;
  QGroupBox*              SubGroup2;
  QLabel*                 TextLabel1;
  QLabel*                 TextLabel2;
  QLabel*                 TextLabel3;
  QLabel*                 TextLabel4;
  QLabel*                 TextLabel5;
  QLabel*                 TextLabel6;
  QPushButton*            SelectButton1;
  QPushButton*            SelectButton2;
  QPushButton*            SelectButton3;
  QPushButton*            SelectButton4;
  QPushButton*            SelectButton5;
  QPushButton*            SelectButton6;
  QLineEdit*              LineEdit1;
  QLineEdit*              LineEdit2;
  QLineEdit*              LineEdit3;
  QLineEdit*              LineEdit4;
  QLineEdit*              LineEdit5;
  QLineEdit*              LineEdit6;
  QCheckBox*              CheckBoxMerge;
  QCheckBox*              CheckBoxPolygons;
  QCheckBox*              CheckBoxPolyedrs;

  QWidget*                SewFreeBordersWidget;
  QGroupBox*              ModeGroup;
  QButtonGroup*           ModeButGrp;
  //QPushButton*            SelectMeshButton;
  QLineEdit*              LineEditMesh;

  SMESHGUI_SpinBox*       SpinBoxTolerance;
  QCheckBox*              AutoSewCheck;

  QWidget*                GroupCoincidentWidget;
  QListWidget*            ListCoincident;
  QPushButton*            DetectButton;
  QPushButton*            RemoveGroupButton;
  QCheckBox*              SelectAllCheck;

  QListWidget*            ListEdit;
  QButtonGroup*           MoveBorderEndsButGrp;
  QLineEdit*              BorderEndLine[2];
  QPushButton*            SwapBut;
  QPushButton*            SetFirstButton;
  QPushButton*            RemoveElemButton;
  SalomeApp_IntSpinBox*   StepSpin;

  QString                 myHelpFileName;


  struct BorderGroupDisplayer;
  std::vector< BorderGroupDisplayer* > myBorderDisplayers;
  SMESH::CoincidentFreeBorders_var     myBorders;
  int                                  myCurGroupIndex;
  int                                  myCurPartIndex;
  int                                  myStoredRepresentation;
  unsigned int                         myStoredEntityMode;

  bool                    haveBorders();
  QString                 getGroupText( int groupIndex );
  QString                 getPartText( const SMESH::FreeBorderPart& part );
  void                    showGroup( QListWidgetItem* item );
  bool                    setCurrentGroup();
  bool                    setCurrentPart();
  void                    onGroupChange(bool partChange=false);
  void                    setDisplayMode();
  void                    restoreDisplayMode();


 protected slots:
  virtual void            reject();

 private slots:
  void                    ConstructorsClicked( int );
  void                    onModeChange( int );
  void                    onAutoSew( int );
  void                    onDetectClicked();
  void                    onRemoveGroupClicked();
  void                    onSelectGroup();
  void                    onSelectAll(int);
  void                    onSelectBorderPartFromGroup();
  void                    onSetFirstClicked();
  void                    onRemoveElemClicked();
  void                    onMoveBorderEnd(int);
  void                    onSwapClicked();
  void                    ClickOnOk();
  bool                    ClickOnApply();
  void                    ClickOnHelp();
  void                    SetEditCurrentArgument();
  void                    SelectionIntoArgument( bool = true );
  void                    DeactivateActiveDialog();
  void                    ActivateThisDialog();
  void                    onTextChange( const QString& );
  void                    onOpenView();
  void                    onCloseView();
};

#endif // SMESHGUI_SEWINGDLG_H
