//  File      : SMESHGUI_EditScalarBarDlg.h
//  Created   : Wed Jun 12 12:01:39 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESHGUI_EDITSCALARBARDLG_H
#define SMESHGUI_EDITSCALARBARDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;

class SMESHGUI;

class SMESHGUI_EditScalarBarDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_EditScalarBarDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_EditScalarBarDlg();

    QPushButton* PushButtonUpdateView;
    QLineEdit* LineEditMax;
    QLineEdit* LineEditMin;
    QLabel* TextLabelMax;
    QLabel* TextLabelMin;
    QFrame* Line1;

    SMESHGUI *mySMESHGUI;

public slots:
    void updateView();
    
protected:
    QGridLayout* grid;
    QGridLayout* grid_2;
    QGridLayout* grid_3;
    QGridLayout* grid_4;
    QGridLayout* grid_5;
    QGridLayout* grid_6;
};

#endif // SMESHGUI_EDITSCALARBARDLG_H
