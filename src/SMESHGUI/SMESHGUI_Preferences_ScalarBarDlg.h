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

#include <qdialog.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QRadioButton;
class QSpinBox;
class SMESHGUI;

class QtxDblSpinBox;

class LightApp_SelectionMgr;

class SMESH_Actor;

class SMESHGUI_Preferences_ScalarBarDlg : public QDialog
{ 
  Q_OBJECT

public:
  ~SMESHGUI_Preferences_ScalarBarDlg();
  static void ScalarBarPreferences( SMESHGUI* );
  static void ScalarBarProperties ( SMESHGUI* );

protected:
  SMESHGUI_Preferences_ScalarBarDlg( SMESHGUI*, bool, bool modal = FALSE );
  static SMESHGUI_Preferences_ScalarBarDlg* myDlg;
  void closeEvent( QCloseEvent* e );
  void setOriginAndSize( const double x, const double y, const double w, const double h );
  void initScalarBarFromResources();

protected slots:
  void onOk();
  bool onApply();
  void onCancel();
  void onTitleColor();
  void onLabelsColor();
  void onSelectionChanged();
  void onXYChanged();
  void onOrientationChanged();

private:
  SMESHGUI*                mySMESHGUI;
  LightApp_SelectionMgr*   mySelectionMgr;
  SMESH_Actor*             myActor;
  double                   myIniX, myIniY, myIniW, myIniH;
  int                      myIniOrientation;
  double DEF_VER_X,DEF_VER_Y,DEF_VER_H,DEF_VER_W;
  double DEF_HOR_X,DEF_HOR_Y,DEF_HOR_H,DEF_HOR_W;

  QGroupBox*         myRangeGrp;
  QLineEdit*         myMinEdit;
  QLineEdit*         myMaxEdit;

  QGroupBox*         myFontGrp;
  QToolButton*       myTitleColorBtn;
  QComboBox*         myTitleFontCombo;
  QCheckBox*         myTitleBoldCheck;
  QCheckBox*         myTitleItalicCheck;
  QCheckBox*         myTitleShadowCheck;
  QToolButton*       myLabelsColorBtn;
  QComboBox*         myLabelsFontCombo;
  QCheckBox*         myLabelsBoldCheck;
  QCheckBox*         myLabelsItalicCheck;
  QCheckBox*         myLabelsShadowCheck;
  
  QGroupBox*         myLabColorGrp;
  QSpinBox*          myColorsSpin;
  QSpinBox*          myLabelsSpin;

  QButtonGroup*      myOrientationGrp;
  QRadioButton*      myVertRadioBtn;
  QRadioButton*      myHorizRadioBtn;

  QGroupBox*         myOriginDimGrp;
  QtxDblSpinBox*     myXSpin;
  QtxDblSpinBox*     myYSpin;
  QtxDblSpinBox*     myWidthSpin;
  QtxDblSpinBox*     myHeightSpin;

  QGroupBox*         myButtonGrp;
  QPushButton*       myOkBtn;
  QPushButton*       myApplyBtn;
  QPushButton*       myCancelBtn;
};

#endif // SMESHGUI_PREFERENCES_SCALARBARDLG_H
