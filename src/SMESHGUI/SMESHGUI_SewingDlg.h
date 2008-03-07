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
//  File   : SMESHGUI_SewingDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_SEWING_H
#define DIALOGBOX_SEWING_H

#include "SMESH_SMESHGUI.hxx"

#include "LightApp_SelectionMgr.h"

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
class SMESHGUI;
class SMESH_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)


//=================================================================================
// class    : SMESHGUI_SewingDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_SewingDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_SewingDlg( SMESHGUI*,
			const char* name = 0,
			bool modal = FALSE,
			WFlags fl = 0);
    ~SMESHGUI_SewingDlg();

private:
    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * );                          /* mouse enter the QWidget */
    void hideEvent ( QHideEvent * );                       /* ESC key */
    void keyPressEvent(QKeyEvent*);
    int  GetConstructorId();
    bool IsValid();
    
    SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */
    int                           myOk1, myOk2, myOk3, myOk4, myOk5, myOk6;    
    QLineEdit*                    myEditCurrentArgument;   /* Current  LineEdit */
    SVTK_Selector*                mySelector;
    
    bool                          myBusy;
    SMESH::SMESH_Mesh_var         myMesh;
    SMESH_Actor*                  myActor;
        
    QButtonGroup* GroupConstructors;
    QRadioButton* RadioButton1;
    QRadioButton* RadioButton2;
    QRadioButton* RadioButton3;
    QRadioButton* RadioButton4;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QPushButton* buttonHelp;
    QGroupBox* GroupArguments;
    QGroupBox* SubGroup1;
    QGroupBox* SubGroup2;
    QLabel* TextLabel1;
    QLabel* TextLabel2;
    QLabel* TextLabel3;
    QLabel* TextLabel4;
    QLabel* TextLabel5;
    QLabel* TextLabel6;
    QPushButton* SelectButton1;
    QPushButton* SelectButton2;
    QPushButton* SelectButton3;
    QPushButton* SelectButton4;
    QPushButton* SelectButton5;
    QPushButton* SelectButton6;
    QLineEdit* LineEdit1;
    QLineEdit* LineEdit2;
    QLineEdit* LineEdit3;
    QLineEdit* LineEdit4;
    QLineEdit* LineEdit5;
    QLineEdit* LineEdit6;
    QCheckBox* CheckBoxMerge;
    QCheckBox* CheckBoxPolygons;
    QCheckBox* CheckBoxPolyedrs;

    QString myHelpFileName;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    bool ClickOnApply();
    void ClickOnHelp();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument(bool isSelectionChanged = true) ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void onTextChange(const QString&);
    
protected:
    QGridLayout* SMESHGUI_SewingDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupArgumentsLayout;
};

#endif // DIALOGBOX_SEWING_H
