//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_ExtrusionAlongPathDlg.h
//  Author : Vadim SANDLER
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_EXTRUSION_PATH_H
#define DIALOGBOX_EXTRUSION_PATH_H

#include "LightApp_SelectionMgr.h"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>

class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QLabel;
class QToolButton;
class QLineEdit;
class QCheckBox;
class QListBox;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;
class SVTK_ViewWindow;
class SVTK_Selector;


// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_ExtrusionAlongPathDlg
// purpose  :
//=================================================================================
class SMESHGUI_ExtrusionAlongPathDlg : public QDialog
{
  Q_OBJECT

  class SetBusy {
    public:
      SetBusy (SMESHGUI_ExtrusionAlongPathDlg* _dlg) { myDlg = _dlg; myDlg->myBusy = true; }
      ~SetBusy() { myDlg->myBusy = false; }
    private:
      SMESHGUI_ExtrusionAlongPathDlg* myDlg;
  };
  friend class SetBusy;

public:
  SMESHGUI_ExtrusionAlongPathDlg (SMESHGUI*,
				  bool modal = FALSE);
  ~SMESHGUI_ExtrusionAlongPathDlg();

  bool eventFilter (QObject* object, QEvent* event);

private:
  void Init (bool ResetControls = true);
  void enterEvent (QEvent*);                           /* mouse enter the QWidget */
  int  GetConstructorId();
  void SetEditCurrentArgument (QToolButton* button);

  SMESHGUI*                     mySMESHGUI;            /* Current SMESHGUI object */
  LightApp_SelectionMgr*        mySelectionMgr;        /* User shape selection */
  SVTK_Selector*                mySelector;

  QWidget*                      myEditCurrentArgument; /* Current  argument */

  bool                          myBusy;
  SMESH::SMESH_Mesh_var         myMesh;
  SMESH_Actor*                  myMeshActor;
  SMESH::SMESH_IDSource_var     myIDSource;
  SMESH::SMESH_Mesh_var         myPathMesh;
  GEOM::GEOM_Object_var         myPathShape;
  SUIT_SelectionFilter*         myElementsFilter;
  SUIT_SelectionFilter*         myPathMeshFilter;
  int                           myType;

  // widgets
  QButtonGroup*     GroupConstructors;
  QRadioButton*     Elements1dRB;
  QRadioButton*     Elements2dRB;

  QGroupBox*        GroupArguments;
  QLabel*           ElementsLab;
  QToolButton*      SelectElementsButton;
  QLineEdit*        ElementsLineEdit;
  QCheckBox*        MeshCheck;
  QGroupBox*        PathGrp;
  QLabel*           PathMeshLab;
  QToolButton*      SelectPathMeshButton;
  QLineEdit*        PathMeshLineEdit;
  QLabel*           PathShapeLab;
  QToolButton*      SelectPathShapeButton;
  QLineEdit*        PathShapeLineEdit;
  QLabel*           StartPointLab;
  QToolButton*      SelectStartPointButton;
  QLineEdit*        StartPointLineEdit;
  QCheckBox*        AnglesCheck;
  QGroupBox*        AnglesGrp;
  QListBox*         AnglesList;
  QToolButton*      AddAngleButton;
  QToolButton*      RemoveAngleButton;
  SMESHGUI_SpinBox* AngleSpin;
  QCheckBox*        BasePointCheck;
  QGroupBox*        BasePointGrp;
  QToolButton*      SelectBasePointButton;
  QLabel*           XLab;
  SMESHGUI_SpinBox* XSpin;
  QLabel*           YLab;
  SMESHGUI_SpinBox* YSpin;
  QLabel*           ZLab;
  SMESHGUI_SpinBox* ZSpin;

  QGroupBox*        GroupButtons;
  QPushButton*      OkButton;
  QPushButton*      ApplyButton;
  QPushButton*      CloseButton;

protected slots:
  void reject();

private slots:
  void ConstructorsClicked (int);
  void ClickOnOk();
  bool ClickOnApply();
  void SetEditCurrentArgument();
  void SelectionIntoArgument();
  void DeactivateActiveDialog();
  void ActivateThisDialog();
  void onTextChange (const QString&);
  void onSelectMesh();
  void onAnglesCheck();
  void onBasePointCheck();
  void OnAngleAdded();
  void OnAngleRemoved();
};

#endif // DIALOGBOX_EXTRUSION_PATH_H
