//  File      : SMESHGUI_NodesDlg.h
//  Created   : Tue May 14 21:36:44 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef DIALOGBOX_NODES_H
#define DIALOGBOX_NODES_H

#include "SALOME_Selection.h"

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
class SMESHGUI_SpinBox;

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
    SMESHGUI_NodesDlg( QWidget* parent = 0, const char* name = 0,
		       SALOME_Selection* Sel = 0,
		       bool modal = FALSE,
		       WFlags fl = 0 );

    ~SMESHGUI_NodesDlg();
    
private :

    SALOME_Selection* mySelection ;
    SMESHGUI*         myMeshGUI ;

    SMESH::SMESH_Mesh_var myMesh;

    void Init(SALOME_Selection* Sel) ;
    void enterEvent(QEvent* e);
    void closeEvent(QCloseEvent* e) ;

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
    void ClickOnApply();
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

