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
//  File   : SMESHGUI_EditHypothesesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_EDIT_HYPOTHESES_H
#define DIALOGBOX_EDIT_HYPOTHESES_H

//#include "SMESH_TypeFilter.hxx"
#include "SUIT_SelectionFilter.h"
#include "LightApp_SelectionMgr.h"

// QT Includes
#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <map>
#include <string>

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QListBox;
class QListBoxItem;
class SMESHGUI;

typedef map<std::string, int> MapIOR;

//=================================================================================
// class    : SMESHGUI_EditHypothesesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EditHypothesesDlg : public QDialog
{
    Q_OBJECT

public:
    SMESHGUI_EditHypothesesDlg (SMESHGUI*,
				const char* name = 0,
				bool modal = FALSE,
				WFlags fl = 0);
    ~SMESHGUI_EditHypothesesDlg();

protected:
    virtual void closeEvent (QCloseEvent*);
    virtual void enterEvent (QEvent*);

private:
    void Init();

    void InitHypDefinition();
    void InitAlgoDefinition();
    void InitHypAssignation();
    void InitAlgoAssignation();

    void InitGeom();

    void UpdateControlState();

    bool StoreMesh();
    bool StoreSubMesh();

    bool IsOld(QListBoxItem* hypItem);

private:
    SMESHGUI*                     mySMESHGUI;
    LightApp_SelectionMgr*        mySelectionMgr;

    GEOM::GEOM_Object_var         myGeomShape;
    QLineEdit*                    myEditCurrentArgument; 

    SMESH::SMESH_Mesh_var         myMesh;
    SMESH::SMESH_subMesh_var      mySubMesh;

    //Handle(SALOME_TypeFilter)     myGeomFilter;
    //Handle(SMESH_TypeFilter)      myMeshOrSubMeshFilter;
    SUIT_SelectionFilter*         myGeomFilter;
    SUIT_SelectionFilter*         myMeshOrSubMeshFilter;

    MapIOR                        myMapOldHypos, myMapOldAlgos;
    int                           myNbModification;

    bool                          myImportedMesh;

    QGroupBox*    GroupButtons;
    QPushButton*  buttonOk;
    QPushButton*  buttonApply;
    QPushButton*  buttonCancel;

    QGroupBox*    GroupC1;
    QLabel*       TextLabelC1A1;
    QPushButton*  SelectButtonC1A1;
    QLineEdit*    LineEditC1A1;
    QLabel*       TextLabelC1A2;
    QPushButton*  SelectButtonC1A2;
    QLineEdit*    LineEditC1A2;

    QGroupBox*    GroupHypotheses;
    QLabel*       TextHypDefinition;
    QListBox*     ListHypDefinition;
    QLabel*       TextHypAssignation;
    QListBox*     ListHypAssignation;

    QGroupBox*    GroupAlgorithms;
    QLabel*       TextAlgoDefinition;
    QListBox*     ListAlgoDefinition;
    QLabel*       TextAlgoAssignation;
    QListBox*     ListAlgoAssignation;

private slots:
    void ClickOnOk();
    bool ClickOnApply();
    void ClickOnCancel();
    void SetEditCurrentArgument();
    void SelectionIntoArgument();
    void DeactivateActiveDialog();
    void ActivateThisDialog();

    void removeItem(QListBoxItem*);
    void addItem(QListBoxItem*);
};

#endif // DIALOGBOX_EDIT_HYPOTHESES_H
