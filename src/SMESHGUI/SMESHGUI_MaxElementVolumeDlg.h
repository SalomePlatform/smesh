//  File      : SMESHGUI_MaxElementVolumeDlg.h
//  Created   : Wed Jun 12 21:18:15 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef DIALOGBOX_MAX_ELEMENT_VOLUME_H
#define DIALOGBOX_MAX_ELEMENT_VOLUME_H

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
// class    : SMESHGUI_MaxElementVolumeDlg
// purpose  :
//=================================================================================
class SMESHGUI_MaxElementVolumeDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_MaxElementVolumeDlg( QWidget* parent = 0, const char* name = 0,  bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_MaxElementVolumeDlg();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*             mySMESHGUI ;

    QString               myNameHypothesis ;
    double                myMaxElementVolume ;
    bool                  myOkNameHypothesis ;
    bool                  myOkMaxElementVolume ;

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
    QLabel* TextLabel_MaxElementVolume ;
    QLineEdit* LineEdit_NameHypothesis ;
    SMESHGUI_SpinBox* SpinBox_MaxElementVolume ;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void TextChangedInLineEdit(const QString& newText) ;
  
protected:
    QGridLayout* SMESHGUI_MaxElementVolumeDlgLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupC1Layout;
    QGridLayout* GroupC2Layout;
};

#endif // DIALOGBOX_MAX_ELEMENT_VOLUME_H
