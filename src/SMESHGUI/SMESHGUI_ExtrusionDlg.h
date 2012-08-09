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
// File   : SMESHGUI_ExtrusionDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_EXTRUSIONDLG_H
#define SMESHGUI_EXTRUSIONDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI_FilterDlg;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;
class SalomeApp_IntSpinBox;

//=================================================================================
// class    : SMESHGUI_ExtrusionDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ExtrusionDlg : public SMESHGUI_PreviewDlg
{
  Q_OBJECT

public:
  SMESHGUI_ExtrusionDlg( SMESHGUI* );
  ~SMESHGUI_ExtrusionDlg();

  void                             reject();

private:
  void                             Init( bool = true );
  void                             enterEvent( QEvent* );       /* mouse enter the QWidget */
  void                             closeEvent( QCloseEvent* );
  void                             keyPressEvent( QKeyEvent* );
  int                              GetConstructorId();
  void                             getExtrusionVector(SMESH::DirStruct& aVector);
  
  bool                             isValid();
  bool                             isValuesValid();
  
  SMESHGUI_IdValidator*            myIdValidator;
  LightApp_SelectionMgr*           mySelectionMgr;        /* User shape selection */
  QWidget*                         myEditCurrentArgument; /* Current  argument editor */
  int                              myNbOkElements;        /* to check when elements are defined */
  SVTK_Selector*                   mySelector;

  SMESH::SMESH_IDSource_var        mySelectedObject;

  bool                             myBusy;
  SMESH::SMESH_Mesh_var            myMesh;
  SMESH::long_array_var            myElementsId;
  SMESH_Actor*                     myActor;
  Handle(SALOME_InteractiveObject) myIO;
  SUIT_SelectionFilter*            myMeshOrSubMeshOrGroupFilter;

  // widgets
  QGroupBox*                       ConstructorsBox;
  QButtonGroup*                    GroupConstructors;
  QRadioButton*                    RadioButton0;
  QRadioButton*                    RadioButton1;
  QRadioButton*                    RadioButton2;
  QRadioButton*                    RadioButton3;
  QRadioButton*                    RadioButton4;

  QGroupBox*                       GroupArguments;
  QGroupBox*                       GroupDimensions;
  QLabel*                          TextLabelElements;
  QPushButton*                     SelectElementsButton;
  QLineEdit*                       LineEditElements;
  QCheckBox*                       CheckBoxMesh;
  QLabel*                          TextLabelVector;
  QLabel*                          TextLabelDistance;
  QPushButton*                     SelectVectorButton;
  QLabel*                          TextLabelDx;
  SMESHGUI_SpinBox*                SpinBox_Dx;
  QLabel*                          TextLabelDy;
  SMESHGUI_SpinBox*                SpinBox_Dy;
  QLabel*                          TextLabelDz;
  SMESHGUI_SpinBox*                SpinBox_Dz;
  QLabel*                          TextLabelVx;
  SMESHGUI_SpinBox*                SpinBox_Vx;
  QLabel*                          TextLabelVy;
  SMESHGUI_SpinBox*                SpinBox_Vy;
  QLabel*                          TextLabelVz;
  SMESHGUI_SpinBox*                SpinBox_Vz;
  QLabel*                          TextLabelDist;
  SMESHGUI_SpinBox*                SpinBox_VDist;
  QLabel*                          TextLabelNbSteps;
  SalomeApp_IntSpinBox*            SpinBox_NbSteps;
  QCheckBox*                       MakeGroupsCheck;

  QGroupBox*                       GroupButtons;
  QPushButton*                     buttonOk;
  QPushButton*                     buttonCancel;
  QPushButton*                     buttonApply;
  QPushButton*                     buttonHelp;

  QString                          myHelpFileName;
  QString                          myIDs;

  QPushButton*                     myFilterBtn;
  SMESHGUI_FilterDlg*              myFilterDlg;

protected slots:
  virtual void              onDisplaySimulation( bool );
   
private slots:
  void                            ConstructorsClicked( int );
  void                            CheckIsEnable();
  void                            ClickOnOk();
  bool                            ClickOnApply();
  void                            ClickOnCancel();
  void                            ClickOnHelp();
  void                            ClickOnRadio();
  void                            SetEditCurrentArgument();
  void                            SelectionIntoArgument();
  void                            DeactivateActiveDialog();
  void                            ActivateThisDialog();
  void                            onTextChange( const QString& );
  void                            onSelectMesh( bool );
  void                            setFilters();
};

#endif // SMESHGUI_EXTRUSIONDLG_H
