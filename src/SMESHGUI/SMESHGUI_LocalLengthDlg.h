//  File      : SMESHGUI_LocalLengthDlg.h
//  Created   : Mon May 27 11:38:37 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef DIALOGBOX_LOCAL_LENGTH_H
#define DIALOGBOX_LOCAL_LENGTH_H

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
class QPushButton;
class QRadioButton;
class SMESHGUI;
class SMESHGUI_SpinBox;


//=================================================================================
// class    : SMESHGUI_LocalLengthDlg
// purpose  :
//=================================================================================
class SMESHGUI_LocalLengthDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_LocalLengthDlg( QWidget* parent = 0, const char* name = 0,  bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_LocalLengthDlg();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*             mySMESHGUI ;

    QString               myNameHypothesis ;
    double                myLength ;
    bool                  myOkNameHypothesis ;
    bool                  myOkLength ;

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
    QLabel* TextLabel_Length ;
    QLineEdit* LineEdit_NameHypothesis ;
    SMESHGUI_SpinBox* SpinBox_Length ;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;
  
protected:
    QGridLayout* SMESHGUI_LocalLengthDlgLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupC1Layout;
    QGridLayout* GroupC2Layout;
};

#endif // DIALOGBOX_LOCAL_LENGTH_H
