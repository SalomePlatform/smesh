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
//  File   : SMESHGUI_aParameterDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_aParameterDlg_H
#define SMESHGUI_aParameterDlg_H

// QT Includes
#include <qdialog.h>

class QLabel;
class QPushButton;
class QSpinBox;
class SMESHGUI;
class QAD_SpinBoxDbl;

//=================================================================================
// class    : SMESHGUI_aParameterDlg
// purpose  :
//=================================================================================
class SMESHGUI_aParameterDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_aParameterDlg( QWidget*     parent   = 0,
			    QString      title    = QString::null,
			    QString      label    = QString::null,
			    const double bottom   = -1E6,
			    const double top      = +1E6,
			    const int    decimals = 3,
			    bool         modal    = TRUE );
    SMESHGUI_aParameterDlg( QWidget*     parent   = 0,
			    QString      title    = QString::null,
			    QString      label    = QString::null,
			    const int    bottom   = 0,
			    const int    top      = 1000,
			    bool         modal    = TRUE );

    ~SMESHGUI_aParameterDlg();

    void   setValue( const double val );
    void   setValue( const int    val );
    double getDblValue();
    int    getIntValue();

protected:
    void init( bool isDouble );

private:
    SMESHGUI*       mySMESHGUI ;

    QPushButton*    myButtonOk;
    QPushButton*    myButtonCancel;
    QLabel*         myTextLabel;
    QSpinBox*       myIntSpin;
    QAD_SpinBoxDbl* myDblSpin;
};

#endif // SMESHGUI_aParameterDlg.h
