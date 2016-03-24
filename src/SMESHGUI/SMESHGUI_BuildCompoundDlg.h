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
// File   : SMESHGUI_BuildCompoundDlg.h
// Author : Alexander KOVALEV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_BUILDCOMPOUNDDLG_H
#define SMESHGUI_BUILDCOMPOUNDDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QComboBox;
class SMESHGUI;
class SMESHGUI_SpinBox;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;

//=================================================================================
// class    : SMESHGUI_BuildCompoundDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_BuildCompoundDlg : public QDialog
{
  Q_OBJECT

public:

  SMESHGUI_BuildCompoundDlg( SMESHGUI* );
  ~SMESHGUI_BuildCompoundDlg();

public:
  static QString          GetDefaultName( const QString& );

private:
  void                    Init();
  void                    enterEvent( QEvent* );             /* mouse enter the QWidget */
  void                    keyPressEvent( QKeyEvent* );

  bool                    isValid();

  void                    setIsApplyAndClose( const bool theFlag );
  bool                    isApplyAndClose() const;
  
private:
  SMESHGUI*               mySMESHGUI;     /* Current SMESHGUI object */
  LightApp_SelectionMgr*  mySelectionMgr; /* User shape selection */

  SMESH::SMESH_IDSource_var  myMesh;
  SUIT_SelectionFilter*      myMeshFilter;
  SMESH::ListOfIDSources_var myMeshArray;

  // Widgets
  QGroupBox*              GroupConstructors;
  QRadioButton*           Constructor1;

  QGroupBox*              GroupButtons;
  QPushButton*            buttonOk;
  QPushButton*            buttonCancel;
  QPushButton*            buttonApply;
  QPushButton*            buttonHelp;

  QGroupBox*              GroupName;
  QLabel*                 TextLabelName;
  QLineEdit*              LineEditName;

  QGroupBox*              GroupArgs;
  QLabel*                 TextLabelMeshes;
  QPushButton*            SelectButton;
  QLineEdit*              LineEditMeshes;
  QLabel*                 TextLabelUnion;
  QComboBox*              ComboBoxUnion;
  QCheckBox*              CheckBoxCommon;
  QCheckBox*              CheckBoxMerge;
  QLabel*                 TextLabelTol;
  SMESHGUI_SpinBox*       SpinBoxTol;

  QString                 myHelpFileName;

  bool                    myIsApplyAndClose;

protected slots:
  virtual void            reject();

private slots:
  void                    ClickOnOk();
  bool                    ClickOnApply();
  void                    ClickOnHelp();
  void                    SelectionIntoArgument();
  void                    DeactivateActiveDialog();
  void                    ActivateThisDialog();
  void                    onSelectMerge( bool );
};

#endif // SMESHGUI_BUILDCOMPOUNDDLG_H
