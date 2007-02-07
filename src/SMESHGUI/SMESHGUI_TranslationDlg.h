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
//  File   : SMESHGUI_TranslationDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_TRANSLATION_H
#define DIALOGBOX_TRANSLATION_H

#include "SMESH_SMESHGUI.hxx"

#include "LightApp_SelectionMgr.h"

#include "SMESH_LogicalFilter.hxx"

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
class SMESHGUI_SpinBox;
class SMESH_Actor;
class SVTK_ViewWindow;
class SVTK_Selector;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)


//=================================================================================
// class    : SMESHGUI_TranslationDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_TranslationDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_TranslationDlg( SMESHGUI*,
			     const char* name = 0,
			     bool modal = FALSE,
			     WFlags fl = 0);
    ~SMESHGUI_TranslationDlg();

private:
    void Init (bool ResetControls = true);
    void closeEvent (QCloseEvent*);
    void enterEvent (QEvent*);                             /* mouse enter the QWidget */
    void hideEvent (QHideEvent*);                          /* ESC key */
    void keyPressEvent(QKeyEvent*);
    int GetConstructorId();

    SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr*        mySelectionMgr;          /* User shape selection */
    QString                       myElementsId;
    int                           myNbOkElements;          /* to check when elements are defined */

    SVTK_Selector*                mySelector;

    QWidget*                      myEditCurrentArgument;

    bool                          myBusy;
    SMESH::SMESH_Mesh_var         myMesh;
    SMESH_Actor*                  myActor;
    SMESH_LogicalFilter*          myMeshOrSubMeshOrGroupFilter;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* RadioButton1;
    QRadioButton* RadioButton2;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QPushButton* buttonHelp;
    QGroupBox* GroupArguments;
    QLabel* TextLabelElements;
    QPushButton* SelectElementsButton;
    QLineEdit* LineEditElements;
    QCheckBox* CheckBoxMesh;
    QLabel* TextLabel1;
    QPushButton* SelectButton1;
    QLabel* TextLabel1_1;
    SMESHGUI_SpinBox* SpinBox1_1;
    QLabel* TextLabel1_2;
    SMESHGUI_SpinBox* SpinBox1_2;
    QLabel* TextLabel1_3;
    SMESHGUI_SpinBox* SpinBox1_3;
    QLabel* TextLabel2;
    QPushButton* SelectButton2;
    QLabel* TextLabel2_1;
    SMESHGUI_SpinBox* SpinBox2_1;
    QLabel* TextLabel2_2;
    SMESHGUI_SpinBox* SpinBox2_2;
    QLabel* TextLabel2_3;
    SMESHGUI_SpinBox* SpinBox2_3;
    QCheckBox* CheckBoxCopy;

    QString myHelpFileName;
   
    private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void ClickOnHelp();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void onTextChange(const QString&);
    void onSelectMesh(bool toSelectMesh);
    
protected:
    QGridLayout* SMESHGUI_TranslationDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupArgumentsLayout;
};

#endif // DIALOGBOX_TRANSLATION_H
