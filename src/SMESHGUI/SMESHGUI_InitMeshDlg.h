//  File      : SMESHGUI_InitMeshDlg.h
//  Created   : Mon May 27 10:23:17 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef DIALOGBOX_INIT_MESH_H
#define DIALOGBOX_INIT_MESH_H

#include "SALOME_Selection.h"
#include "SALOME_TypeFilter.hxx"
#include "SMESH_TypeFilter.hxx"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(GEOM_Shape)
#include CORBA_SERVER_HEADER(SMESH_Gen)

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


//=================================================================================
// class    : SMESHGUI_InitMeshDlg
// purpose  :
//=================================================================================
class SMESHGUI_InitMeshDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_InitMeshDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_InitMeshDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*                     mySMESHGUI ;
    SALOME_Selection*             mySelection ;
             
    GEOM::GEOM_Shape_var                myGeomShape ;
    int                           myConstructorId ; 
    QLineEdit*                    myEditCurrentArgument; 

    QString                       myNameMesh ;

    Handle(SALOME_TypeFilter)     myGeomFilter;
    Handle(SMESH_TypeFilter)      myHypothesisFilter;
    Handle(SMESH_TypeFilter)      myAlgorithmFilter;

    SALOME_ListIO                 HypoList;
    SALOME_ListIO                 AlgoList;

    bool                          myOkHypothesis;
    bool                          myOkAlgorithm;

    SMESH::SMESH_Hypothesis_var   myHypothesis;
    SMESH::SMESH_Hypothesis_var   myAlgorithm;

    SMESH::SMESH_Mesh_var         myMesh;

    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox*    GroupButtons;
    QPushButton*  buttonOk;
    QPushButton*  buttonCancel;
    QPushButton*  buttonApply;
    QGroupBox*    GroupC1;
    QLabel*       TextLabel_NameMesh ;
    QLineEdit*    LineEdit_NameMesh ;
    QLabel*       TextLabelC1A1;
    QPushButton*  SelectButtonC1A1;
    QLineEdit*    LineEditC1A1;

    QLabel*       TextLabelC1A1Hyp;
    QPushButton*  SelectButtonC1A1Hyp;
    QLineEdit*    LineEditC1A1Hyp;

    QLabel*       TextLabelC1A1Algo;
    QPushButton*  SelectButtonC1A1Algo;
    QLineEdit*    LineEditC1A1Algo;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;

protected:
    QGridLayout* SMESHGUI_InitMeshDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;
};

#endif // DIALOGBOX_INIT_MESH_H
