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
//  File   : SMESHGUI_MeshInfosDlg.h
//  Author : Nicolas BARBEROU
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_MESHINFOSDLG_H
#define SMESHGUI_MESHINFOSDLG_H

#include "SALOME_Selection.h"
#include "QAD_Study.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

// QT Includes
#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QLabel;
class QPushButton;
class SMESHGUI;

class SMESHGUI_MeshInfosDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_MeshInfosDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_MeshInfosDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                /* mouse enter the QWidget */
    void DumpMeshInfos();

    SMESH::SMESH_Gen_var  myCompMesh ;                
    SMESHGUI*             mySMESHGUI ;            
    SMESH::SMESH_Mesh_var myMesh ;            
    SALOME_Selection*     mySelection ; 
    SALOMEDS::Study_var   myStudy;

    QGroupBox* GroupBox1;
    QLabel* TextLabel12;
    QLabel* TextLabel11;
    QLabel* TextLabel13;
    QLabel* TextLabel14;
    QGroupBox* GroupBox2;
    QLabel* TextLabel21;
    QLabel* TextLabel22;
    QLabel* TextLabel23;
    QLabel* TextLabel24;
    QGroupBox* GroupBox3;
    QLabel* TextLabel31;
    QLabel* TextLabel32;
    QLabel* TextLabel33;
    QLabel* TextLabel34;
    QPushButton* buttonOk;

private slots:

    void ClickOnOk();
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

protected:
    QVBoxLayout* SMESHGUI_MeshInfosDlgLayout;
    QHBoxLayout* Layout1;

};

#endif // SMESHGUI_MESHINFOSDLG_H
