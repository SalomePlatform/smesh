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
//  File   : SMESHGUI_AddMeshElementDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_ADD_FACE_H
#define DIALOGBOX_ADD_FACE_H

#include "LightApp_SelectionMgr.h"

#include "SMDSAbs_ElementType.hxx"


// QT Includes
#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI;
class SMESH_Actor;
class SMDS_Mesh;
class SVTK_ViewWindow;
class SVTK_Selector;

namespace SMESH{
  struct TElementSimulation;
}

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_AddMeshElementDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddMeshElementDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_AddMeshElementDlg( SMESHGUI*,
			        const char* = 0, 
                                SMDSAbs_ElementType ElementType = SMDSAbs_Edge,
			        int nbNodes = 2, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_AddMeshElementDlg();

private:
    void Init ();
    void closeEvent (QCloseEvent*);
    void hideEvent (QHideEvent*);                 /* ESC key */
    void enterEvent (QEvent*);                    /* mouse enter the QWidget */
    void displaySimulation();

    SMESHGUI*                   mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr*      mySelectionMgr;          /* User shape selection */
    int                         myNbOkNodes;               /* to check when arguments is defined */
    bool                        myBusy;
    SVTK_Selector*              mySelector;

    QLineEdit*                  myEditCurrentArgument;   /* Current  LineEdit */

    int                         myElementType;
    int                         myNbNodes;
    bool                        myIsPoly;

    SMESH::SMESH_Mesh_var       myMesh;
    SMESH_Actor*                myActor;
    SMESH::TElementSimulation*  mySimulation;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox   * GroupButtons;
    QPushButton * buttonOk;
    QPushButton * buttonCancel;
    QPushButton * buttonApply;
    QGroupBox   * GroupC1;
    QLabel      * TextLabelC1A1;
    QPushButton * SelectButtonC1A1;
    QLineEdit   * LineEditC1A1;

    QCheckBox   * Reverse;

private slots:

    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void CheckBox( int );
    void onTextChange(const QString&);

protected:
    QGridLayout* SMESHGUI_AddMeshElementDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;
};

#endif // DIALOGBOX_ADD_FACE_H
