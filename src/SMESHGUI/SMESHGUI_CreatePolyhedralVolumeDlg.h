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
//  File   : SMESHGUI_CreatePolyhedralVolumeDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_CREATEPOLYHEDRAL_H
#define DIALOGBOX_CREATEPOLYHEDRAL_H

// QT Includes
#include <qdialog.h>

class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QListBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class SVTK_ViewWindow;
class LightApp_SelectionMgr;

namespace SMESH{
  struct TPolySimulation;
}

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_CreatePolyhedralVolumeDlg
// purpose  :
//=================================================================================
class SMESHGUI_CreatePolyhedralVolumeDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_CreatePolyhedralVolumeDlg( SMESHGUI*, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_CreatePolyhedralVolumeDlg();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                          /* mouse enter the QWidget */
    void hideEvent ( QHideEvent * );                        /* ESC key */
    int  GetConstructorId();
    void displaySimulation();
    
    SMESHGUI*                     mySMESHGUI ;              /* Current SMESHGUI object */
    LightApp_SelectionMgr*        mySelectionMgr ;             /* User shape selection */
    SVTK_Selector*                mySelector;
    QString                       myIDs;
    int                           myNbOkElements ;            /* to check when elements are defined */

    QLineEdit*                    myEditCurrentArgument;
   
    SMESH::SMESH_Mesh_var         myMesh;
    SMESH_Actor*                  myActor;
    SMESH::TPolySimulation*       mySimulation;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* RadioButton1;
    QRadioButton* RadioButton2;
    QCheckBox* Preview;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QGroupBox* GroupContent;
    QLabel* TextLabelIds;
    QPushButton* SelectElementsButton;
    QLineEdit* LineEditElements;
    QListBox* myFacesByNodes;
    QLabel* myFacesByNodesLabel;
    QPushButton* AddButton;
    QPushButton* RemoveButton;
       
    public slots:

    void onAdd();
    void onRemove();

    private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnPreview(bool theToggled);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void onTextChange(const QString&);
    void onListSelectionChanged();
    
protected:
    QGridLayout* SMESHGUI_CreatePolyhedralVolumeDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupContentLayout;
};

#endif // DIALOGBOX_CREATEPOLYHEDRAL_H
