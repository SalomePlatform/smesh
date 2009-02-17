//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_Preferences_ColorDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_PREFERENCES_COLORDLG_H
#define SMESHGUI_PREFERENCES_COLORDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

class QSpinBox;
class SMESHGUI;
class QtxIntSpinBox;
class QtxColorButton;

class SMESHGUI_EXPORT SMESHGUI_Preferences_ColorDlg : public QDialog
{ 
  Q_OBJECT
    
public:
  SMESHGUI_Preferences_ColorDlg( SMESHGUI* );
  ~SMESHGUI_Preferences_ColorDlg();

  void                  SetColor( int, const QColor& );
  QColor                GetColor( int );
  void                  SetIntValue( int, int );
  int                   GetIntValue( int );

protected:
  void                  closeEvent( QCloseEvent* );

private slots:
  void                  ClickOnOk();
  void                  ClickOnCancel();
  void                  DeactivateActiveDialog();
  void                  ActivateThisDialog();
  
private:
  SMESHGUI*             mySMESHGUI;            
  
  QtxColorButton*       btnFillColor;
  QtxColorButton*       btnBackFaceColor;
  QtxColorButton*       btnOutlineColor;
  QSpinBox*             SpinBox_Width;
  QtxIntSpinBox*        SpinBox_Shrink;
  QtxColorButton*       btnNodeColor;
  QSpinBox*             SpinBox_Nodes_Size;
};

#endif // SMESHGUI_PREFERENCES_COLORDLG_H
