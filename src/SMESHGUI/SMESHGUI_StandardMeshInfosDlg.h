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
//  File   : SMESHGUI_StandardMeshInfosDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef SMESHGUI_STANDARDMESHINFOSDLG_H
#define SMESHGUI_STANDARDMESHINFOSDLG_H

#include "SMESH_TypeFilter.hxx"

// QT Includes
#include <qdialog.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextBrowser;
class SALOME_Selection;

class SMESHGUI_StandardMeshInfosDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_StandardMeshInfosDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_StandardMeshInfosDlg();

protected:
    void closeEvent( QCloseEvent* e );
    void windowActivationChange( bool oldActive );
    void DumpMeshInfos();

private slots:
    void onSelectionChanged();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog();
    void onStartSelection();

private:
    SALOME_Selection*     mySelection; 
    bool                  myStartSelection;
    bool                  myIsActiveWindow;
    
    Handle(SMESH_TypeFilter)      myMeshFilter;

    QLabel*       myNameLab;
    QPushButton*  mySelectBtn;
    QLineEdit*    myMeshLine;
    
    QTextBrowser* myInfo;
    
    QGroupBox*    myMeshGroup;
    QGroupBox*    myInfoGroup;
    
    QGroupBox*    myButtonsGroup;
    QPushButton*  myOkBtn;
};

#endif // SMESHGUI_STANDARDMESHINFOSDLG_H
