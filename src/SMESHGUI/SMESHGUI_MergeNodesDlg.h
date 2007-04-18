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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_MergeNodesDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_MERGENODES_H
#define DIALOGBOX_MERGENODES_H

#include "SMESH_SMESHGUI.hxx"

#include "LightApp_SelectionMgr.h"
//#include "SMESH_TypeFilter.hxx"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>

class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QListView;
class QListViewItem;
class QListBox;
class SMESHGUI;
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;


// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)


//=================================================================================
// class    : SMESHGUI_MergeNodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_MergeNodesDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_MergeNodesDlg( SMESHGUI*,
			    const char* name = 0,
			    bool modal = FALSE,
			    WFlags fl = 0);
    ~SMESHGUI_MergeNodesDlg();

private:
    void Init();
    void closeEvent( QCloseEvent* e );
    void enterEvent ( QEvent * );                           /* mouse enter the QWidget */
    void hideEvent ( QHideEvent * );                        /* ESC key */
    void keyPressEvent( QKeyEvent* e );
    void onEditNodesGroup();

    SMESHGUI*                     mySMESHGUI;               /* Current SMESHGUI object */
    LightApp_SelectionMgr*        mySelectionMgr;           /* User shape selection */
    SVTK_Selector*                mySelector;

    QWidget*                      myEditCurrentArgument;

    SMESH::SMESH_Mesh_var         myMesh;
    SMESH::SMESH_IDSource_var     mySubMeshOrGroup;
    SMESH_Actor*                  myActor;
    //Handle(SMESH_TypeFilter)      myMeshOrSubMeshFilter;
    SUIT_SelectionFilter*         myMeshOrSubMeshOrGroupFilter;

    QButtonGroup*     GroupConstructors;
    QRadioButton*     RadioButton1;
    QGroupBox*        GroupButtons;
    QPushButton*      buttonOk;
    QPushButton*      buttonCancel;
    QPushButton*      buttonApply;
    QPushButton*      buttonHelp;
    QGroupBox*        GroupMesh;
    QGroupBox*        GroupCoincident;
    QGroupBox*        GroupEdit;
    QLabel*           TextLabelName;
    QLabel*           TextLabelTolerance;
    QLineEdit*        LineEditMesh;
    QPushButton*      SelectMeshButton;
    QPushButton*      DetectButton;
    QPushButton*      AddButton;
    QPushButton*      RemoveButton;
    QCheckBox*        SelectAllCB;
    QListView*        ListCoincident;
    QListBox*         ListEdit;
    SMESHGUI_SpinBox* SpinBoxTolerance;

    QString           myHelpFileName;
   
private slots:

    void ClickOnOk();
    void ClickOnCancel();
    bool ClickOnApply();
    void ClickOnHelp();
    void updateControls();
    void onDetect();
    void onSelectNodesGroup();
    void onSelectAll(bool isToggled);
    void onSelectNodesFromGroup();
    void onAdd();
    void onRemove();
    void SetEditCurrentArgument();
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    
protected:
    QGridLayout* SMESHGUI_MergeNodesDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupMeshLayout;
    QGridLayout* GroupCoincidentLayout;
    QGridLayout* GroupEditLayout;
};

#endif // DIALOGBOX_MERGENODES_H
