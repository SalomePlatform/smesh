//  File      : SMESHGUI_MeshInfosDlg.h
//  Created   : Sat Jun 08 15:31:16 2002
//  Author    : Nicolas BARBEROU

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : EADS CCR 2002 2002
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
