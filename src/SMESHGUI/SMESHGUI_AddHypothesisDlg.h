//  File      : SMESHGUI_AddHypothesisDlg.h
//  Created   : Wed May 22 19:05:13 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESHGUI
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef DIALOGBOX_ADD_HYPOTHESIS_H
#define DIALOGBOX_ADD_HYPOTHESIS_H

#include "SALOME_Selection.h"
#include "SALOME_TypeFilter.hxx"
#include "SMESH_TypeFilter.hxx"
#include "SALOME_ListIO.hxx"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

// QT Includes
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


//=================================================================================
// class    : SMESHGUI_AddHypothesisDlg
// purpose  :
//=================================================================================
class SMESHGUI_AddHypothesisDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_AddHypothesisDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_AddHypothesisDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                /* mouse enter the QWidget */

    SMESH::SMESH_Gen_var  myCompMesh ;                
    SMESHGUI*             mySMESHGUI ;            
    SALOME_Selection*     mySelection ;          

    SMESH::SMESH_Mesh_var       myMesh;
    SMESH::SMESH_subMesh_var    mySubMesh;

    SMESH::SMESH_Hypothesis_var myHypothesis;

    Handle(SMESH_TypeFilter)  myHypothesisFilter;
    Handle(SMESH_TypeFilter)  myMeshOrSubMeshFilter;

    bool                      myOkMesh ;
    bool                      myOkSubMesh ;
    bool                      myOkHypothesis;

    int                   myConstructorId ;       /* Current constructor id = radio button id */
    QLineEdit*            myEditCurrentArgument;  /* Current LineEdit */    

    SALOME_ListIO HypoList;

    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;

    QGroupBox* GroupConstructor1;
    QLineEdit* LineEditC1A1Hyp;
    QLineEdit* LineEditC1A2Mesh;
    QPushButton* SelectButtonC1A1Hyp;
    QPushButton* SelectButtonC1A2Mesh;
    QLabel* TextLabelC1A2Mesh;
    QLabel* TextLabelC1A1Hyp;

    QGroupBox* GroupButtons;
    QPushButton* buttonApply;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void LineEditReturnPressed() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

protected:
    QGridLayout* SMESHGUI_AddHypothesisDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupConstructor1Layout;
};

#endif // DIALOGBOX_ADD_HYPOTHESIS_H
