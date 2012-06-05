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
// File   : SMESHGUI_ExtrusionAlongPathDlg.h
// Author : Vadim SANDLER, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_EXTRUSIONALONGPATHDLG_H
#define SMESHGUI_EXTRUSIONALONGPATHDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_PreviewDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QLabel;
class QToolButton;
class QLineEdit;
class QCheckBox;
class QListWidget;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_IdValidator;
class SMESHGUI_SpinBox;
class SMESHGUI_FilterDlg;
class SVTK_Selector;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;

//=================================================================================
// class    : SMESHGUI_ExtrusionAlongPathDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_ExtrusionAlongPathDlg : public SMESHGUI_PreviewDlg
{
  Q_OBJECT

  class SetBusy;
  friend class SetBusy;

public:
  SMESHGUI_ExtrusionAlongPathDlg( SMESHGUI* );
  ~SMESHGUI_ExtrusionAlongPathDlg();

  bool                      eventFilter( QObject*, QEvent* );

private:
  void                      Init( bool = true );
  void                      enterEvent( QEvent* );                           /* mouse enter the QWidget */
  void                      keyPressEvent( QKeyEvent* );
  int                       GetConstructorId();
  void                      SetEditCurrentArgument( QToolButton* );

  bool                      isValid();
  bool                      isValuesValid();
  
  SMESH::long_array_var     getSelectedElements();
  SMESH::double_array_var   getAngles();

  void                      updateLinearAngles();
  
  SMESHGUI_IdValidator*     myIdValidator;
  LightApp_SelectionMgr*    mySelectionMgr;        /* User shape selection */
  SVTK_Selector*            mySelector;

  QWidget*                  myEditCurrentArgument; /* Current  argument */

  bool                      myBusy;
  SMESH::SMESH_Mesh_var     myMesh;
  SMESH_Actor*              myMeshActor;
  SMESH::SMESH_IDSource_var myIDSource;
  //SMESH::SMESH_Mesh_var     myPathMesh;
  SMESH::SMESH_IDSource_var myPath;
  //GEOM::GEOM_Object_var     myPathShape;
  SUIT_SelectionFilter*     myElementsFilter;
  SUIT_SelectionFilter*     myPathMeshFilter;
  int                       myType;
  QList<double>             myAnglesList;

  // widgets
  QGroupBox*                ConstructorsBox;
  QButtonGroup*             GroupConstructors;
  QRadioButton*             Elements1dRB;
  QRadioButton*             Elements2dRB;

  QGroupBox*                GroupArguments;
  QLabel*                   ElementsLab;
  QToolButton*              SelectElementsButton;
  QLineEdit*                ElementsLineEdit;
  QCheckBox*                MeshCheck;
  QGroupBox*                PathGrp;
  QToolButton*              SelectPathMeshButton;
  QLineEdit*                PathMeshLineEdit;
  //QToolButton*              SelectPathShapeButton;
  //QLineEdit*                PathShapeLineEdit;
  QToolButton*              SelectStartPointButton;
  QLineEdit*                StartPointLineEdit;
  QCheckBox*                LinearAnglesCheck;
  QGroupBox*                AnglesGrp;
  QListWidget*              AnglesList;
  QToolButton*              AddAngleButton;
  QToolButton*              RemoveAngleButton;
  SMESHGUI_SpinBox*         AngleSpin;
  QGroupBox*                BasePointGrp;
  QToolButton*              SelectBasePointButton;
  SMESHGUI_SpinBox*         XSpin;
  SMESHGUI_SpinBox*         YSpin;
  SMESHGUI_SpinBox*         ZSpin;
  QCheckBox*                MakeGroupsCheck;

  QGroupBox*                GroupButtons;
  QPushButton*              OkButton;
  QPushButton*              ApplyButton;
  QPushButton*              CloseButton;
  QPushButton*              HelpButton;

  QString                   myHelpFileName;

  QPushButton*              myFilterBtn;
  SMESHGUI_FilterDlg*       myFilterDlg;
   
protected slots:
  void                      reject();
  virtual void              onDisplaySimulation( bool );

private slots:
  void                      ConstructorsClicked( int );
  void                      ClickOnOk();
  bool                      ClickOnApply();
  void                      ClickOnHelp();
  void                      SetEditCurrentArgument();
  void                      SelectionIntoArgument();
  void                      DeactivateActiveDialog();
  void                      ActivateThisDialog();
  void                      onTextChange( const QString& );
  void                      onSelectMesh();
  void                      OnAngleAdded();
  void                      OnAngleRemoved();
  void                      setFilters();
};

#endif // SMESHGUI_EXTRUSIONALONGPATHDLG_H
