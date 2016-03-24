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
// File   : SMESHGUI_RevolutionDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_REVOLUTIONDLG_H
#define SMESHGUI_REVOLUTIONDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// Qt includes
#include <QMap>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class LightApp_SelectionMgr;
class QAction;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QRadioButton;
class SALOME_Actor;
class SMESHGUI;
class SMESHGUI_3TypesSelector;
class SMESHGUI_FilterDlg;
class SMESHGUI_IdValidator;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SMESH_LogicalFilter;
class SVTK_Selector;
class SalomeApp_IntSpinBox;

//=================================================================================
// class    : SMESHGUI_RevolutionDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_RevolutionDlg : public SMESHGUI_PreviewDlg
{ 
  Q_OBJECT

public:
  SMESHGUI_RevolutionDlg( SMESHGUI* );
  ~SMESHGUI_RevolutionDlg();

private:
  enum {NONE_SELECT, POINT_SELECT, FACE_SELECT};
  
  void                      Init( bool = true);
  void                      enterEvent( QEvent* );           /* mouse enter the QWidget */
  void                      keyPressEvent( QKeyEvent* );
  int                       GetConstructorId();
  bool                      IsAxisOk();
  
  bool                      isValid();
  
  LightApp_SelectionMgr*    mySelectionMgr;          /* User shape selection */
  SVTK_Selector*            mySelector;
  QWidget*                  myEditCurrentArgument;   /* Current  argument */

  SMESHGUI_3TypesSelector*  SelectorWdg;
  QGroupBox*                GroupButtons;
  QPushButton*              buttonOk;
  QPushButton*              buttonCancel;
  QPushButton*              buttonApply;
  QPushButton*              buttonHelp;
  QGroupBox*                GroupArguments;
  QGroupBox*                GroupAxis;
  QCheckBox*                MakeGroupsCheck;
  QGroupBox*                GroupAngleBox;
  QButtonGroup*             GroupAngle;
  QRadioButton*             RadioButton3;
  QRadioButton*             RadioButton4;
  
  QLabel*                   TextLabelPoint;
  QPushButton*              SelectPointButton;
  QLabel*                   TextLabelX;
  SMESHGUI_SpinBox*         SpinBox_X;
  QLabel*                   TextLabelY;
  SMESHGUI_SpinBox*         SpinBox_Y;
  QLabel*                   TextLabelZ;
  SMESHGUI_SpinBox*         SpinBox_Z;
  QLabel*                   TextLabelVector;
  QPushButton*              SelectVectorButton;
  QLabel*                   TextLabelDX;
  SMESHGUI_SpinBox*         SpinBox_DX;
  QLabel*                   TextLabelDY;
  SMESHGUI_SpinBox*         SpinBox_DY;
  QLabel*                   TextLabelDZ;
  SMESHGUI_SpinBox*         SpinBox_DZ;
  
  QLabel*                   TextLabelAngle;
  SMESHGUI_SpinBox*         SpinBox_Angle;
  QLabel*                   TextLabelNbSteps;
  SalomeApp_IntSpinBox*     SpinBox_NbSteps;
  QLabel*                   TextLabelTolerance;
  SMESHGUI_SpinBox*         SpinBox_Tolerance;

  QMenu*                    SelectVectorMenu;
  QMap<QAction*,int>        myMenuActions;
  int                       myVectorDefinition;

  
  QString                   myHelpFileName;

protected slots:
  virtual void              onDisplaySimulation( bool );
  virtual void              reject();
   
private slots:
  void                      CheckIsEnable();
  void                      ClickOnOk();
  bool                      ClickOnApply();
  void                      ClickOnHelp();
  void                      SetEditCurrentArgument();
  void                      SelectionIntoArgument();
  void                      DeactivateActiveDialog();
  void                      ActivateThisDialog();
  void                      onAngleTextChange( const QString& );
  void                      onSelectVectorMenu( QAction* );
  void                      onSelectVectorButton();
  void                      onOpenView();
  void                      onCloseView();
};

#endif // SMESHGUI_REVOLUTIONDLG_H
