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
//  File   : SMESHGUI_Preferences_ScalarBarDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
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
