//  File      : SMESHGUI_Preferences_ScalarBarDlg.h
//  Created   : Tue Jun 11 17:23:15 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESHGUI_PREFERENCES_SCALARBARDLG_H
#define SMESHGUI_PREFERENCES_SCALARBARDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class SMESHGUI_Preferences_ScalarBarDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_Preferences_ScalarBarDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_Preferences_ScalarBarDlg();

    QButtonGroup* ButtonGroup_Orientation;
    QRadioButton* RadioHoriz;
    QRadioButton* RadioVert;
    QGroupBox* Properties;
    QLabel* NumberLabels;
    QLabel* NumberColors;
    QSpinBox* SpinBoxLabels;
    QSpinBox* SpinBoxColors;
    QLabel* TextLabel2;
    QComboBox* ComboBox1;
    QCheckBox* Shadow;
    QCheckBox* Italic;
    QCheckBox* Bold;
    QGroupBox* GroupBox5;
    QLineEdit* LineEditWidth;
    QLineEdit* LineEditHeight;
    QLabel* Height;
    QLabel* Width;
    QPushButton* buttonCancel;
    QPushButton* buttonOk;

protected:
    QGridLayout* grid;
    QGridLayout* grid_2;
    QGridLayout* grid_3;
    QGridLayout* grid_4;
    QGridLayout* grid_5;
    QGridLayout* grid_6;
    QGridLayout* grid_7;
    QGridLayout* grid_8;
    QGridLayout* grid_9;
    QGridLayout* grid_10;
    QGridLayout* grid_11;
    QGridLayout* grid_12;
    QGridLayout* grid_13;
    QGridLayout* grid_14;
    QGridLayout* grid_15;
};

#endif // SMESHGUI_PREFERENCES_SCALARBARDLG_H
