//  File      : SMESHGUI_Preferences_ColorDlg.h
//  Created   : Mon Jun 17 19:11:46 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESHGUI_PREFERENCES_COLORDLG_H
#define SMESHGUI_PREFERENCES_COLORDLG_H

// SALOME Includes
#include "QAD_Config.h"
#include "QAD_Settings.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)

// QT Includes
#include <qdialog.h>

class QButtonGroup;
class QFrame;
class QLabel;
class QPushButton;
class QSpinBox;
class QColor;
class SMESHGUI;

class SMESHGUI_Preferences_ColorDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_Preferences_ColorDlg( QWidget* parent = 0, const char* name = 0 );
    ~SMESHGUI_Preferences_ColorDlg();

    void   SetColor(int type, QColor color);
    QColor GetColor(int type);
    void   SetIntValue(int type, int value);
    int    GetIntValue(int type);

private:
    void closeEvent( QCloseEvent* e ) ;
    void Init();

private slots:
//  void RadioButtonClicked(int radioButtonId);
    void ClickOnOk();
    void ClickOnCancel();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;

    void SelectFillColor();
    void SelectBackFaceColor();
    void SelectOutlineColor();
    void SelectNodeColor();

private:
    SMESH::SMESH_Gen_var  myCompMesh ;                
    SMESHGUI*             mySMESHGUI ;            

    QButtonGroup*         ButtonGroup1;
    QLabel*               TextLabel_Fill;
    QPushButton*          btnFillColor;
    QLabel*               TextLabel_Outine;
    QPushButton*          btnOutlineColor;
    QLabel*               TextLabel_Width;
    QSpinBox*             SpinBox_Width;
    QLabel*               TextLabel_BackFace;
    QPushButton*          btnBackFaceColor;
    QLabel*               TextLabel_ShrinkCoeff;
    QSpinBox*             SpinBox_Shrink;
    QButtonGroup*         ButtonGroup2;
    QLabel*               TextLabel_Nodes_Color;
    QPushButton*          btnNodeColor;
    QLabel*               TextLabel_Nodes_Size;
    QSpinBox*             SpinBox_Nodes_Size;
    QButtonGroup*         GroupButtons;
    QPushButton*          buttonOk;
    QPushButton*          buttonCancel;
};

#endif // SMESHGUI_PREFERENCES_COLORDLG_H
