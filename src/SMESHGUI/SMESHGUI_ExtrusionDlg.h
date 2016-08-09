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
// File   : SMESHGUI_ExtrusionDlg.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_EXTRUSIONDLG_H
#define SMESHGUI_EXTRUSIONDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QToolButton;

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
// class    : SMESHGUI_3TypesSelector
// purpose  : A widget used to select both nodes, edges and faces for
//            Extrusion and Revolution operations
//=================================================================================

class SMESHGUI_EXPORT SMESHGUI_3TypesSelector : public QWidget
{
  Q_OBJECT

    public:

  SMESHGUI_3TypesSelector( QWidget * parent = 0 );
  ~SMESHGUI_3TypesSelector();

  void Clear();
  void SetEnabled( bool enable, SMESH::ElementType type );
  bool IsAnythingSelected( SMESH::ElementType type = SMESH::ALL );
  SMESH::ElementType GetSelected( SMESH::ListOfIDSources & nodes,
                                  SMESH::ListOfIDSources & edges,
                                  SMESH::ListOfIDSources & faces );
  SMESH::SMESH_Mesh_var GetMesh() { return myMesh; }
  SMESH_Actor*         GetActor() { return myActor; }
  Handle(SALOME_InteractiveObject) GetIO() { return myIO; }
  QButtonGroup* GetButtonGroup()  { return mySelectBtnGrp; }

 signals:

  void selectionChanged();

 private slots:

  void                            selectionIntoArgument();
  void                            onTextChange( const QString& );
  void                            onSelectMesh( bool on );
  void                            setFilters();
  void                            onSelectType( int iType );

 private:

  void                             addTmpIdSource( SMESH::long_array_var& ids,
                                                   int iType, int index);

  QGroupBox*                       myGroups   [3];
  QLabel*                          myLabel    [3];
  QLineEdit*                       myLineEdit [3];
  QCheckBox*                       myMeshChk  [3];
  QPushButton*                     myFilterBtn[3];
  QButtonGroup*                    mySelectBtnGrp;
  SMESHGUI_FilterDlg*              myFilterDlg;
  SUIT_SelectionFilter*            myFilter   [3];
  SMESHGUI_IdValidator*            myIdValidator;

  bool                             myBusy;
  SMESH::SMESH_Mesh_var            myMesh;
  SMESH_Actor*                     myActor;
  Handle(SALOME_InteractiveObject) myIO;
  SMESH::ListOfIDSources_var       myIDSource[3];
  QList<SMESH::IDSource_wrap>      myTmpIDSourceList;

  LightApp_SelectionMgr*           mySelectionMgr;
  SVTK_Selector*                   mySelector;
};

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

private:
  void                             Init( bool = true );
  void                             enterEvent( QEvent* );       /* mouse enter the QWidget */
  void                             keyPressEvent( QKeyEvent* );
  int                              GetConstructorId();
  void                             getExtrusionVector(SMESH::DirStruct& aVector);
  void                             extrusionByNormal(SMESH::SMESH_MeshEditor_ptr meshEditor,
                                                     const bool                  makeGroups=false);
  bool                             getScaleParams( SMESH::double_array*& scales,
                                                   SMESH::double_array*& basePoint );
  
  bool                             isValid();
  bool                             isValuesValid();
  
  LightApp_SelectionMgr*           mySelectionMgr;        /* User shape selection */
  SVTK_Selector*                   mySelector;
  QList<double>                    myScalesList;

  // widgets
  SMESHGUI_3TypesSelector*         SelectorWdg;
  QRadioButton*                    ExtrMethod_RBut0;
  QRadioButton*                    ExtrMethod_RBut1;
  QRadioButton*                    ExtrMethod_RBut2;

  QGroupBox*                       GroupArguments;
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
  QCheckBox*                       ByAverageNormalCheck;
  QCheckBox*                       UseInputElemsOnlyCheck;
  QCheckBox*                       MakeGroupsCheck;

  QGroupBox*                       BasePointGrp;
  QPushButton*                     SelectBasePointButton;
  SMESHGUI_SpinBox*                BasePoint_XSpin;
  SMESHGUI_SpinBox*                BasePoint_YSpin;
  SMESHGUI_SpinBox*                BasePoint_ZSpin;

  QGroupBox*                       ScalesGrp;
  QListWidget*                     ScalesList;
  QToolButton*                     AddScaleButton;
  QToolButton*                     RemoveScaleButton;
  SMESHGUI_SpinBox*                ScaleSpin;
  QCheckBox*                       LinearScalesCheck;

  QGroupBox*                       GroupButtons;
  QPushButton*                     buttonOk;
  QPushButton*                     buttonCancel;
  QPushButton*                     buttonApply;
  QPushButton*                     buttonHelp;

  QString                          myHelpFileName;
  QString                          myIDs;

protected slots:
  virtual void                    onDisplaySimulation( bool );
  virtual void                    reject();
   
private slots:
  void                            CheckIsEnable();
  void                            ClickOnOk();
  bool                            ClickOnApply();
  void                            ClickOnHelp();
  void                            ClickOnRadio();
  void                            SetEditCurrentArgument();
  void                            SelectionIntoArgument();
  void                            DeactivateActiveDialog();
  void                            ActivateThisDialog();
  void                            onOpenView();
  void                            onCloseView();
  void                            OnScaleAdded();
  void                            OnScaleRemoved();

};

#endif // SMESHGUI_EXTRUSIONDLG_H
