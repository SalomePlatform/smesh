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
//  File   : SMESHGUI_EditHypothesesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_EDIT_HYPOTHESES_H
#define DIALOGBOX_EDIT_HYPOTHESES_H

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
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <map>
#include <string>

using namespace std;

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QListBox;
class QListBoxItem;
class SMESHGUI;


//=================================================================================
// class    : SMESHGUI_EditHypothesesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EditHypothesesDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_EditHypothesesDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_EditHypothesesDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    void InitHypDefinition();
    void InitAlgoDefinition();
    void InitHypAssignation();
    void InitAlgoAssignation();

    void InitGeom();

    SMESHGUI*                     mySMESHGUI ;
    SALOME_Selection*             mySelection ;
             
    GEOM::GEOM_Shape_var          myGeomShape ;
    int                           myConstructorId ; 
    QLineEdit*                    myEditCurrentArgument; 

    SMESH::SMESH_Mesh_var         myMesh;
    SMESH::SMESH_subMesh_var      mySubMesh;

    Handle(SALOME_TypeFilter)     myGeomFilter;
    Handle(SMESH_TypeFilter)      myMeshOrSubMeshFilter;

    map<string,string>            mapNameIOR;

    SALOME_ListIO                 HypoList;
    SALOME_ListIO                 AlgoList;

    bool                          myOkHypothesis;
    bool                          myOkAlgorithm;

    SMESH::SMESH_Hypothesis_var   myHypothesis;
    SMESH::SMESH_Hypothesis_var   myAlgorithm;

    SMESH::ListOfHypothesis_var   myLHypothesis;
    SMESH::ListOfHypothesis_var   myLAlgorithm;
 
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

    QLabel*       TextLabelC1A2;
    QPushButton*  SelectButtonC1A2;
    QLineEdit*    LineEditC1A2;

    QGroupBox* GroupHypotheses;
    QLabel* TextHypDefinition;
    QListBox* ListHypDefinition;
    QLabel* TextHypAssignation;
    QListBox* ListHypAssignation;

    QGroupBox* GroupAlgorithms;
    QLabel* TextAlgoDefinition;
    QListBox* ListAlgoDefinition;
    QLabel* TextAlgoAssignation;
    QListBox* ListAlgoAssignation;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnCancel();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;

    void removeItem(QListBoxItem*);
    void addItem(QListBoxItem*);

protected:
    QGridLayout* SMESHGUI_EditHypothesesDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupC1Layout;

    QGridLayout* grid_3;
    QGridLayout* grid_4;

    QHBoxLayout* hbox_2;
    QHBoxLayout* hbox_3;

    QVBoxLayout* vbox;
    QVBoxLayout* vbox_2;
    QVBoxLayout* vbox_3;
    QVBoxLayout* vbox_4;

};

#endif // DIALOGBOX_EDIT_HYPOTHESES_H
