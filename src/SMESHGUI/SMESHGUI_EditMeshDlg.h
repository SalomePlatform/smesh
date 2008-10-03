// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
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

#ifndef SMESHGUI_EditMeshDlg_H
#define SMESHGUI_EditMeshDlg_H

#include "SMESH_SMESHGUI.hxx"

#include "LightApp_SelectionMgr.h"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>

// Open CASCADE Includes
#include <gp_XYZ.hxx>

class QGridLayout;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QListBox;
class SMESHGUI;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SALOME_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;

namespace SMESH{
  struct TIdPreview;
}

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

//=================================================================================
// class    : SMESHGUI_EditMeshDlg
// purpose  : 
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_EditMeshDlg : public QDialog
{
  Q_OBJECT;

 public:
  SMESHGUI_EditMeshDlg (SMESHGUI  * theModule,
			int         theAction);
  ~SMESHGUI_EditMeshDlg();

 private:
  void Init();
  void closeEvent (QCloseEvent*);
  void enterEvent (QEvent*);              /* mouse enter the QWidget */
  void hideEvent  (QHideEvent*);          /* ESC key */
  void keyPressEvent(QKeyEvent*);
  void onEditGroup();

  void FindGravityCenter(TColStd_MapOfInteger & ElemsIdMap, 
    std::list< gp_XYZ > & GrCentersXYZ);
  // add the centers of gravity of ElemsIdMap elements to the GrCentersXYZ list

 private:
  SMESHGUI*               mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*  mySelectionMgr; /* User shape selection */
  SVTK_Selector*          mySelector;
  
  QWidget*                myEditCurrentArgument;

  SMESH::SMESH_Mesh_var     myMesh;
  SMESH::SMESH_IDSource_var mySubMeshOrGroup;
  SMESH_Actor*              myActor;
  SUIT_SelectionFilter*     myMeshOrSubMeshOrGroupFilter;

  SMESH::TIdPreview*        myIdPreview;

  int myAction;
  bool myIsBusy;

  // Widgets
  QButtonGroup*     GroupConstructors;
  QRadioButton*     RadioButton;

  QGroupBox*        GroupButtons;
  QPushButton*      buttonOk;
  QPushButton*      buttonCancel;
  QPushButton*      buttonApply;
  QPushButton*      buttonHelp;

  QGroupBox*        GroupMesh;
  QLabel*           TextLabelName;
  QPushButton*      SelectMeshButton;
  QLineEdit*        LineEditMesh;

  QGroupBox*        GroupCoincident;
  QLabel*           TextLabelTolerance;
  SMESHGUI_SpinBox* SpinBoxTolerance;
  QPushButton*      DetectButton;
  QListBox*         ListCoincident;
  QPushButton*      AddGroupButton;
  QPushButton*      RemoveGroupButton;
  QCheckBox*        SelectAllCB;

  QGroupBox*        GroupEdit;
  QListBox*         ListEdit;
  QPushButton*      AddElemButton;
  QPushButton*      RemoveElemButton;
  QPushButton*      SetFirstButton;
    
  //protected:
  QGridLayout* DlgLayout;
  QGridLayout* GroupConstructorsLayout;
  QGridLayout* GroupButtonsLayout;
  QGridLayout* GroupMeshLayout;
  QGridLayout* GroupCoincidentLayout;
  QGridLayout* GroupEditLayout;

  QString myHelpFileName;

 private slots:
  void ClickOnOk();
  void ClickOnCancel();
  bool ClickOnApply();
  void ClickOnHelp();
  void updateControls();
  void onDetect();
  void onAddGroup();
  void onRemoveGroup();
  void onSelectGroup();
  void onSelectAll(bool isToggled);
  void onSelectElementFromGroup();
  void onAddElement();
  void onRemoveElement();
  void onSetFirst();
  void SetEditCurrentArgument();
  void SelectionIntoArgument() ;
  void DeactivateActiveDialog() ;
  void ActivateThisDialog() ;

};

#endif // SMESHGUI_EditMeshDlg_H
