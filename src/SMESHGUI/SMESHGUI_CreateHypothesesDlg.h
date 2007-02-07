//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_CreateHypothesesDlg.h
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_CREATE_HYPOTHESES_H
#define DIALOGBOX_CREATE_HYPOTHESES_H

#include "SMESH_SMESHGUI.hxx"

// QT Includes
#include <qvariant.h>
#include <qdialog.h>
#include <qstringlist.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <map>
#include <string>

class QButtonGroup;
class QGroupBox;
class QPushButton;
class QListView;
class QListViewItem;
class SMESHGUI;

//=================================================================================
// class    : SMESHGUI_CreateHypothesesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_CreateHypothesesDlg : public QDialog
{
    Q_OBJECT

public:
    SMESHGUI_CreateHypothesesDlg (SMESHGUI*,
				  const char* name = 0,
				  bool modal = FALSE,
				  bool isAlgo = FALSE);
    ~SMESHGUI_CreateHypothesesDlg ();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    void InitAlgoDefinition();

    SMESHGUI*     mySMESHGUI;
    bool          myIsAlgo;

    QGroupBox*    GroupButtons;
    QPushButton*  buttonCancel;
    QPushButton*  buttonApply;

    QGroupBox* GroupAlgorithms;
    QListView* ListAlgoDefinition;

private slots:

    void ClickOnCancel();
    void ClickOnApply();
    void ActivateThisDialog() ;

    void onSelectionChanged();
    void onDoubleClicked(QListViewItem*);
};

#endif // DIALOGBOX_CREATE_HYPOTHESES_H
