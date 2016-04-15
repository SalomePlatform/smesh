// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_TransparencyDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_TRANSPARENCYDLG_H
#define SMESHGUI_TRANSPARENCYDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

class QLabel;
class QPushButton;
class QSlider;
class LightApp_SelectionMgr;
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
  SMESHGUI_TransparencyDlg( SMESHGUI* );
  ~SMESHGUI_TransparencyDlg();

private:
  void                    keyPressEvent( QKeyEvent* );

private:
  SMESHGUI*               mySMESHGUI;
  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_ViewWindow*        myViewWindow;

  QPushButton*            buttonOk;
  QPushButton*            buttonHelp;
  QLabel*                 TextLabelOpaque;
  QLabel*                 ValueLab;
  QLabel*                 TextLabelTransparent;
  QSlider*                Slider1;

  QString                 myHelpFileName;

public slots:
  void                    ClickOnOk();
  void                    ClickOnHelp();
  void                    ValueHasChanged();
  void                    SetTransparency();
  void                    onSelectionChanged();
};

#endif // SMESHGUI_TRANSPARENCYDLG_H
