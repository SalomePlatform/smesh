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
//  File   : SMESHGUI_Preferences_ColorDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_PREFERENCES_COLORDLG_H
#define SMESHGUI_PREFERENCES_COLORDLG_H

#include "SMESH_SMESHGUI.hxx"

// SALOME Includes
//#include "SUIT_Config.h"
//#include "SUIT_Settings.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

// QT Includes
#include <qdialog.h>

class QButtonGroup;
class QFrame;
class QLabel;
class QPushButton;
class QSpinBox;
class QColor;
class SMESHGUI;

class SMESHGUI_EXPORT SMESHGUI_Preferences_ColorDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_Preferences_ColorDlg( SMESHGUI*, const char* name = 0 );
    ~SMESHGUI_Preferences_ColorDlg();

    void   SetColor(int type, QColor color);
    QColor GetColor(int type);
    void   SetIntValue(int type, int value);
    int    GetIntValue(int type);

private:
    void closeEvent( QCloseEvent* e ) ;
    void Init();

private slots:
//  void RadioButtonClicked(int radioButtonId);
    void ClickOnOk();
    void ClickOnCancel();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

    void SelectFillColor();
    void SelectBackFaceColor();
    void SelectOutlineColor();
    void SelectNodeColor();

private:
    SMESH::SMESH_Gen_var  myCompMesh ;                
    SMESHGUI*             mySMESHGUI ;            

    QButtonGroup*         ButtonGroup1;
    QLabel*               TextLabel_Fill;
    QPushButton*          btnFillColor;
    QLabel*               TextLabel_Outine;
    QPushButton*          btnOutlineColor;
    QLabel*               TextLabel_Width;
    QSpinBox*             SpinBox_Width;
    QLabel*               TextLabel_BackFace;
    QPushButton*          btnBackFaceColor;
    QLabel*               TextLabel_ShrinkCoeff;
    QSpinBox*             SpinBox_Shrink;
    QButtonGroup*         ButtonGroup2;
    QLabel*               TextLabel_Nodes_Color;
    QPushButton*          btnNodeColor;
    QLabel*               TextLabel_Nodes_Size;
    QSpinBox*             SpinBox_Nodes_Size;
    QButtonGroup*         GroupButtons;
    QPushButton*          buttonOk;
    QPushButton*          buttonCancel;
};

#endif // SMESHGUI_PREFERENCES_COLORDLG_H
