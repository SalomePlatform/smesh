//  File      : SMESHGUI_NbSegmentsDlg.h
//  Created   : Mon May 27 11:38:37 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
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
