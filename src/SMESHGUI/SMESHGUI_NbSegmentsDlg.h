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
//  File   : SMESHGUI_NbSegmentsDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_NB_SEGMENTS_H
#define DIALOGBOX_NB_SEGMENTS_H

// QT Includes
#include <qvariant.h>
#include <qdialog.h>
#include <qvalidator.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QRadioButton;
class SMESHGUI;


//=================================================================================
// class    : SMESHGUI_NbSegmentsDlg
// purpose  :
//=================================================================================
class SMESHGUI_NbSegmentsDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_NbSegmentsDlg( QWidget* parent = 0, const char* name = 0,  bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_NbSegmentsDlg();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*             mySMESHGUI ;

    QString               myNameHypothesis ;
    double                myNbSeg ;
    bool                  myOkNameHypothesis ;
    QDoubleValidator      *myVa ; 

    int                   myConstructorId ;
    QLineEdit*            myEditCurrentArgument; 

    QGroupBox* GroupButtons;
    QPushButton* buttonApply;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox* GroupC1;
    QLabel* TextLabel_NameHypothesis ;
    QLabel* TextLabel_NbSeg ;
    QLineEdit* LineEdit_NameHypothesis ;
    QSpinBox* SpinBox_NbSeg ;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void DeactivateActiveDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;
    void ActivateThisDialog() ;
  
protected:
    QGridLayout* SMESHGUI_NbSegmentsDlgLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupC1Layout;
    QGridLayout* GroupC2Layout;
};

#endif // DIALOGBOX_NB_SEGMENTS_H
