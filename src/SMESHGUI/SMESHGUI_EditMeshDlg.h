
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
