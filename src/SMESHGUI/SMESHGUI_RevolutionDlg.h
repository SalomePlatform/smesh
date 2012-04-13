// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SalomeApp_IntSpinBox;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI;
class SMESHGUI_FilterDlg;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SMESH_LogicalFilter;
class SALOME_Actor;
class SMESHGUI_MeshEditPreview;
class QMenu;
class QAction;

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

  void                      reject();

private:
  enum {NONE_SELECT, POINT_SELECT, FACE_SELECT};
  
  void                      Init( bool = true);
  void                      closeEvent( QCloseEvent* );
  void                      enterEvent( QEvent* );           /* mouse enter the QWidget */
  void                      keyPressEvent( QKeyEvent* );
  int                       GetConstructorId();
  bool                      IsAxisOk();
  
  bool                      isValid();
  
  SMESHGUI_IdValidator*     myIdValidator;
  LightApp_SelectionMgr*    mySelectionMgr;          /* User shape selection */
  int                       myNbOkElements;          /* to check when elements are defined */
  QString                   myElementsId;
  QWidget*                  myEditCurrentArgument;   /* Current  argument */
  SVTK_Selector*            mySelector;
  Handle(SALOME_InteractiveObject) myIO;
  
  SMESH::SMESH_IDSource_var mySelectedObject;

  bool                      myBusy;
  SMESH::SMESH_Mesh_var     myMesh;
  SMESH_Actor*              myActor;
  SMESH_LogicalFilter*      myMeshOrSubMeshOrGroupFilter;
  SMESHGUI_MeshEditPreview* mySimulation;
  SALOME_Actor*             myPreviewActor;

  QGroupBox*                ConstructorsBox;
  QButtonGroup*             GroupConstructors;
  QRadioButton*             RadioButton1;
  QRadioButton*             RadioButton2;
  QGroupBox*                GroupButtons;
  QPushButton*              buttonOk;
  QPushButton*              buttonCancel;
  QPushButton*              buttonApply;
  QPushButton*              buttonHelp;
  QGroupBox*                GroupArguments;
  QGroupBox*                GroupAxis;
  QLabel*                   TextLabelElements;
  QPushButton*              SelectElementsButton;
  QLineEdit*                LineEditElements;
  QCheckBox*                CheckBoxMesh;
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
  QString                   myIDs;
  
  QPushButton*              myFilterBtn;
  SMESHGUI_FilterDlg*       myFilterDlg;

protected slots:
  virtual void              onDisplaySimulation( bool );
   
private slots:
  void                      ConstructorsClicked( int );
  void                      ClickOnOk();
  void                      ClickOnCancel();
  bool                      ClickOnApply();
  void                      ClickOnHelp();
  void                      SetEditCurrentArgument();
  void                      SelectionIntoArgument();
  void                      DeactivateActiveDialog();
  void                      ActivateThisDialog();
  void                      onTextChange( const QString& );
  void                      onAngleTextChange( const QString& );
  void                      onSelectMesh( bool );
  void                      onVectorChanged();
  void                      onSelectVectorMenu( QAction* );
  void                      onSelectVectorButton();
  void                      setFilters();
};

#endif // SMESHGUI_REVOLUTIONDLG_H
