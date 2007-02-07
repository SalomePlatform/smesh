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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_TransparencyDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_TRANSPARENCYDLG_H
#define DIALOGBOX_TRANSPARENCYDLG_H

#include "SMESH_SMESHGUI.hxx"

// QT Includes
#include <qdialog.h>

class QLabel;
class QPushButton;
class QSlider;
class LightApp_SelectionMgr;
class SVTK_Selector;
class SVTK_ViewWindow;
class SMESHGUI;

//=================================================================================
// class    : SMESHGUI_TransparencyDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_TransparencyDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_TransparencyDlg( SMESHGUI*,
			      const char* name = 0,
			      bool modal = false,
			      WFlags fl = 0 );

    ~SMESHGUI_TransparencyDlg();

private:
    void keyPressEvent(QKeyEvent*);

private :

  SMESHGUI*               mySMESHGUI;
  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_ViewWindow*        myViewWindow;

  QPushButton*      buttonOk;
  QPushButton*      buttonHelp;
  QLabel*           TextLabelOpaque;
  QLabel*           ValueLab;
  QLabel*           TextLabelTransparent;
  QSlider*          Slider1;

  QString           myHelpFileName;

public slots:
      
    void ClickOnOk();
    void ClickOnHelp();
    void ValueHasChanged();
    void SetTransparency();
    void onSelectionChanged();
};

#endif // DIALOGBOX_TRANSPARENCYDLG_H
