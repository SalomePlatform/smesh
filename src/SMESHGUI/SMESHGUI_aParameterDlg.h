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

#include <list>
#include "SMESHGUI_aParameter.h"

class QLabel;
class QPushButton;
class SMESHGUI;
class QWidget;


//=================================================================================
// class    : SMESHGUI_aParameterDlg
// purpose  :
//=================================================================================
class SMESHGUI_aParameterDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_aParameterDlg( std::list<SMESHGUI_aParameterPtr> params,
			    QWidget*                          parent = 0,
			    QString                           title  = QString::null,
			    bool                              modal  = TRUE );

    ~SMESHGUI_aParameterDlg();

    /* Parameter function */
    static bool Parameters( std::list<SMESHGUI_aParameterPtr> params, const char *aTitle);

protected:
    void init();

private slots:
    void ClickOnOk();

private:
    SMESHGUI*       mySMESHGUI ;

    QPushButton*    myButtonOk;
    QPushButton*    myButtonCancel;

    std::list<QWidget*>               mySpinList;
    std::list<SMESHGUI_aParameterPtr> myParamList;
};

#endif // SMESHGUI_aParameterDlg.h
