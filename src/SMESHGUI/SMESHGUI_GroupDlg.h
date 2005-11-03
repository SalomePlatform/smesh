//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_GroupDlg.h
//  Author : Natalia KOPNOVA
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_GROUP_H
#define DIALOGBOX_GROUP_H

#include "LightApp_SelectionMgr.h"
//#include "SMESH_TypeFilter.hxx"
#include "SUIT_SelectionFilter.h"

// QT Includes
#include <qdialog.h>
#include <qvaluelist.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

class QLineEdit;
class QButtonGroup;
class QGroupBox;
class QListBox;
class QPushButton;
class QCheckBox;
class QWidgetStack;
class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_FilterDlg;
class SVTK_Selector;
class SVTK_ViewWindow;

//=================================================================================
// class    : SMESHGUI_GroupDlg
// purpose  :
//=================================================================================
class SMESHGUI_GroupDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_GroupDlg( SMESHGUI*,
		       const char* name = 0, 
		       SMESH::SMESH_Mesh_ptr theMesh = SMESH::SMESH_Mesh::_nil(), 
		       bool modal = FALSE, WFlags fl = 0 );
    SMESHGUI_GroupDlg( SMESHGUI*,
		       const char* name, 
		       SMESH::SMESH_Group_ptr theGroup,
		       bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_GroupDlg();

public slots:

    void onAdd();
    void onRemove();


private slots:

    void onTypeChanged(int id);
    void onGrpTypeChanged(int id);

    void onOK();
    void onClose();
    bool onApply();
    void onDeactivate();

    void onListSelectionChanged();
    void onObjectSelectionChanged();

    void onSelectSubMesh(bool on);
    void onSelectGroup(bool on);
    void onSelectGeomGroup(bool on);
    void setCurrentSelection();

    void setFilters();
    void onSort();

    void onNameChanged(const QString& text);
    void onFilterAccepted();

private:
    void initDialog(bool create);
    void init(SMESH::SMESH_Mesh_ptr theMesh);
    void init(SMESH::SMESH_Group_ptr theGroup);
    void closeEvent(QCloseEvent* e);
    void enterEvent (QEvent*);
    void hideEvent (QHideEvent*);                          /* ESC key */
    void setSelectionMode(int theMode);
    void updateButtons();

    SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */
    SMESH_Actor*                  myActor;                 /* Current mesh actor */
    int                           myGrpTypeId; /* Current group type id : standalone or group on geometry */
    int                           myTypeId;                /* Current type id = radio button id */
    QLineEdit*                    myCurrentLineEdit;       /* Current  LineEdit */
    SVTK_Selector*                mySelector;

    QPushButton*                  myMeshGroupBtn;
    QLineEdit*                    myMeshGroupLine;
    
    QButtonGroup*                 myTypeGroup;
    QLineEdit*                    myName;

    QButtonGroup*                 myGrpTypeGroup;

    QWidgetStack*                 myWGStack;
    QListBox*                     myElements;
    QPushButton*                  myFilter;

    QCheckBox*                    mySelectSubMesh;
    QPushButton*                  mySubMeshBtn;
    QLineEdit*                    mySubMeshLine;

    QCheckBox*                    mySelectGroup;
    QPushButton*                  myGroupBtn;
    QLineEdit*                    myGroupLine;

    QCheckBox*                    mySelectGeomGroup;
    QPushButton*                  myGeomGroupBtn;
    QLineEdit*                    myGeomGroupLine;

    SMESH::SMESH_Mesh_var         myMesh;
    SMESH::SMESH_Group_var        myGroup;
    QValueList<int>               myIdList;
    GEOM::GEOM_Object_var         myGeomGroup;

    int                           mySelectionMode;
    //Handle(SMESH_TypeFilter)      myMeshFilter;
    //Handle(SMESH_TypeFilter)      mySubMeshFilter;
    //Handle(SMESH_TypeFilter)      myGroupFilter;
    SUIT_SelectionFilter*         myMeshFilter;
    SUIT_SelectionFilter*         mySubMeshFilter;
    SUIT_SelectionFilter*         myGroupFilter;

    SMESHGUI_FilterDlg*           myFilterDlg;

    bool                          myCreate, myIsBusy;
};

#endif // DIALOGBOX_GROUP_H
