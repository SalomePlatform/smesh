//  File      : SMESHGUI_MoveNodesDlg.h
//  Created   : Thu Jun 20 22:30:22 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef DIALOGBOX_MOVE_NODES_H
#define DIALOGBOX_MOVE_NODES_H

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
class SMESHGUI;
class SMESHGUI_SpinBox;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

// VTK Include
#include <vtkActor.h>

//=================================================================================
// class    : SMESHGUI_MoveNodesDlg
// purpose  :
//=================================================================================
class SMESHGUI_MoveNodesDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_MoveNodesDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_MoveNodesDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                          /* mouse enter the QWidget */

    SMESHGUI*                     mySMESHGUI ;              /* Current SMESHGUI object */
    SALOME_Selection*             mySelection ;             /* User shape selection */
    bool                          myOkNodes ;               /* to check when arguments is defined */
    int                           myConstructorId ;         /* Current constructor id = radio button id */
    int                           myIdnode;
    QLineEdit*                    myEditCurrentArgument;    /* Current  LineEdit */

    SMESH::SMESH_Mesh_var         myMesh;
    TColStd_MapOfInteger          myMapIndex;

    vtkActor                      *mySimulationActor;
        
    QGroupBox* GroupCoordinates;
    QLabel* TextLabel_X;
    QLabel* TextLabel_Y;
    QLabel* TextLabel_Z;
    SMESHGUI_SpinBox* SpinBox_X;
    SMESHGUI_SpinBox* SpinBox_Y;
    SMESHGUI_SpinBox* SpinBox_Z;

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
    void ValueChangedInSpinBox( double newValue ) ;

protected:
    QGridLayout* SMESHGUI_MoveNodesDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;
    QGridLayout* GroupCoordinatesLayout;
};

#endif // DIALOGBOX_MOVE_NODES_H
