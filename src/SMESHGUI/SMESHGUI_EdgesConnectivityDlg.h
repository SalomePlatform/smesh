//  File      : SMESHGUI_EdgesConnectivityDlg.h
//  Created   : Tue Jun 25 21:22:54 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef DIALOGBOX_EDGES_CONNECTIVITY_H
#define DIALOGBOX_EDGES_CONNECTIVITY_H

#include "SALOME_Selection.h"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>

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
class SMESHGUI_SpinBox;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_EdgesConnectivityDlg
// purpose  :
//=================================================================================
class SMESHGUI_EdgesConnectivityDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_EdgesConnectivityDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_EdgesConnectivityDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*                     mySMESHGUI ;
    SALOME_Selection*             mySelection ;
    bool                          myOkActor ;
    int                           myConstructorId ; 
    QLineEdit*                    myEditCurrentArgument; 
    
    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox*    GroupButtons;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;
    QPushButton*  buttonApply;
    QGroupBox*    GroupC1;
    QLabel*       TextLabelC1A1;
    QPushButton*  SelectButtonC1A1;
    QLineEdit*    LineEditC1A1;

    QCheckBox*    BoundaryEdges;
    QCheckBox*    ManifoldEdges;
    QCheckBox*    NonManifoldEdges;
    QCheckBox*    FeatureEdges;

    QLabel*       FeatureAngle;
    SMESHGUI_SpinBox* SpinBox_FeatureAngle ;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

    void CheckBox( int );

protected:
    QGridLayout* SMESHGUI_EdgesConnectivityDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;

    QHBoxLayout* hbox_2;
    QVBoxLayout* vbox;
    QVBoxLayout* vbox_2;
};

#endif // DIALOGBOX_EDGES_CONNECTIVITY_H
