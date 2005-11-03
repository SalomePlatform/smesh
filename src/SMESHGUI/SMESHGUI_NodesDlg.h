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
//  File   : SMESHGUI_NodesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_NODES_H
#define DIALOGBOX_NODES_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

class SMESHGUI;
class SVTK_Selector;
class SVTK_ViewWindow;
class SMESHGUI_SpinBox;
class LightApp_SelectionMgr;

namespace SMESH{
  struct TNodeSimulation;
}

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_NodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_NodesDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_NodesDlg (SMESHGUI* theModule, 
		       const char* name = 0,
		       bool modal = FALSE,
		       WFlags fl = 0);

    ~SMESHGUI_NodesDlg();

private:
    LightApp_SelectionMgr*  mySelectionMgr;
    SVTK_Selector*                mySelector;
    SMESHGUI*               mySMESHGUI;

    SMESH::SMESH_Mesh_var   myMesh;
    SMESH::TNodeSimulation* mySimulation;

    void Init ();
    void enterEvent(QEvent* e);
    void closeEvent(QCloseEvent*);
    void hideEvent (QHideEvent *);                 /* ESC key */

    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox* GroupCoordinates;
    SMESHGUI_SpinBox* SpinBox_X;
    SMESHGUI_SpinBox* SpinBox_Y;
    SMESHGUI_SpinBox* SpinBox_Z;

    QLabel* TextLabel_X;
    QLabel* TextLabel_Y;
    QLabel* TextLabel_Z;

    QGroupBox* GroupButtons;
    QPushButton* buttonApply;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

private slots:

    void ClickOnOk();
    void ClickOnCancel();
    bool ClickOnApply();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void SelectionIntoArgument() ;
    void ValueChangedInSpinBox( double newValue ) ;

protected:
    QGridLayout* SMESHGUI_NodesDlgLayout;    
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupCoordinatesLayout;
    QGridLayout* GroupButtonsLayout;
};

#endif // DIALOGBOX_NODES_H
