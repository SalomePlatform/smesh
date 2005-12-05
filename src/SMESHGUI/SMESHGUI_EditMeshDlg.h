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
// See http://www.salome-platform.org/
//

#ifndef DIALOGBOX_GETMESH_H
#define DIALOGBOX_GETMESH_H

#include "LightApp_SelectionMgr.h"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>

// Open CASCADE Includes

class QGridLayout;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_EditMeshDlg
// purpose  : 
//=================================================================================
class SMESHGUI_EditMeshDlg : public QDialog
{
  Q_OBJECT;

 public:
  SMESHGUI_EditMeshDlg (SMESHGUI  * theModule,
			const char* title,
			const char* icon,
			int         theAction);
  ~SMESHGUI_EditMeshDlg();

 private:
  void Init();
  void closeEvent (QCloseEvent*);
  void enterEvent (QEvent*);              /* mouse enter the QWidget */
  void hideEvent  (QHideEvent*);          /* ESC key */

 private:
  SMESHGUI*               mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*  mySelectionMgr; /* User shape selection */

  SMESH::SMESH_Mesh_var   myMesh;
  SUIT_SelectionFilter*   myMeshFilter;

  int myAction;

  // Widgets
  QButtonGroup* GroupConstructors;
  QRadioButton* Constructor1;

  QGroupBox* GroupButtons;
  QPushButton* buttonOk;
  QPushButton* buttonCancel;
  QPushButton* buttonApply;

  QGroupBox* GroupMesh;
  QLabel* TextLabelMesh;
  QPushButton* SelectButton;
  QLineEdit* LineEditMesh;

  //protected:
  QGridLayout* DlgLayout;
  QGridLayout* GroupConstructorsLayout;
  QGridLayout* GroupButtonsLayout;
  QGridLayout* GroupMeshLayout;

 private slots:
  void ClickOnOk();
  void ClickOnCancel();
  void ClickOnApply();
  void SelectionIntoArgument();
  void DeactivateActiveDialog();
  void ActivateThisDialog();
};

#endif // DIALOGBOX_GETMESH_H
