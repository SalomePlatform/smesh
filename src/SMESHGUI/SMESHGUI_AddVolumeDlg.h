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
//  File   : SMESHGUI_AddVolumeDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_ADD_VOLUME_H
#define DIALOGBOX_ADD_VOLUME_H

#include "SALOME_Selection.h"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>

// Open CASCADE Includes
#include <TColStd_MapOfInteger.hxx>

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

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_AddVolumeDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddVolumeDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_AddVolumeDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, 
			     int nbNode = 4, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_AddVolumeDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                          /* mouse enter the QWidget */

    SMESHGUI*                     mySMESHGUI ;              /* Current SMESHGUI object */
    SALOME_Selection*             mySelection ;             /* User shape selection */
    bool                          myOkNodes ;               /* to check when arguments is defined */

    int                           myNodes ;
    int                           myConstructorId ;         /* Current constructor id = radio button id */
    QLineEdit*                    myEditCurrentArgument;    /* Current  LineEdit */

    SMESH::SMESH_Mesh_var         myMesh;
    TColStd_MapOfInteger          myMapIndex;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QGroupBox* GroupC1;
    QLabel* TextLabelC1A1;
    QPushButton* SelectButtonC1A1;
    QLineEdit* LineEditC1A1;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

protected:
    QGridLayout* SMESHGUI_AddVolumeDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;
};

#endif // DIALOGBOX_ADD_VOLUME_H
