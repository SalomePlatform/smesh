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
//  File   : SMESHGUI_PrecisionDlg.h
//  Author : Sergey LITONIN      
//  Module : SMESH


#ifndef SMESHGUI_PrecisionDlg_H
#define SMESHGUI_PrecisionDlg_H

#include <qdialog.h>

class QSpinBox;
class QPushButton;
class QCheckBox;
class QFrame;

/*
  Class       : SMESHGUI_PrecisionDlg
  Description : Dialog to specify precision of mesh quality controls
*/

class SMESHGUI_PrecisionDlg : public QDialog
{
  Q_OBJECT

public:
                            SMESHGUI_PrecisionDlg( QWidget* parent );

  virtual                   ~SMESHGUI_PrecisionDlg();

  void                      Init();

private slots:

  void                      onOk();
  void                      onClose();
  void                      onNotUse();

private:

  QFrame*                   createButtonFrame( QWidget* );
  QFrame*                   createMainFrame( QWidget* );
  void                      closeEvent( QCloseEvent* );
 
private:

  QSpinBox*                 mySpinBox;
  QPushButton*              myOKBtn;
  QPushButton*              myCancelBtn;
  QCheckBox*                myNotUseChk;

};

#endif




















