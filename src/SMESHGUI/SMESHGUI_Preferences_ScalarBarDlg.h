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
// File   : SMESHGUI_Preferences_ScalarBarDlg.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.

#ifndef SMESHGUI_PREFERENCES_SCALARBARDLG_H
#define SMESHGUI_PREFERENCES_SCALARBARDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QToolButton;
class QRadioButton;
class QButtonGroup;
class QLabel;

class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;
class SalomeApp_IntSpinBox;
class QtxColorButton;
class LightApp_SelectionMgr;

class SMESHGUI_EXPORT SMESHGUI_Preferences_ScalarBarDlg : public QDialog
{ 
  Q_OBJECT

private:
  SMESHGUI_Preferences_ScalarBarDlg( SMESHGUI* );

  static SMESHGUI_Preferences_ScalarBarDlg* myDlg;

public:
  ~SMESHGUI_Preferences_ScalarBarDlg();

  static void              ScalarBarProperties( SMESHGUI* );

  void                     setOriginAndSize( const double,
                                             const double,
                                             const double,
                                             const double );
  void                     initScalarBarFromResources();

protected slots:
  virtual void             reject();

protected slots:
  void                     onOk();
  bool                     onApply();
  void                     onHelp();
  void                     onSelectionChanged();
  void                     onXYChanged();
  void                     onMinMaxChanged();
  void                     onOrientationChanged();
  void                     onDistributionChanged( int );
  void                     onDistributionActivated( bool );

private:
  SMESHGUI*                mySMESHGUI;
  LightApp_SelectionMgr*   mySelectionMgr;
  SMESH_Actor*             myActor;
  double                   myIniX, myIniY, myIniW, myIniH;
  int                      myIniOrientation;
  double                   DEF_VER_X, DEF_VER_Y, DEF_VER_H, DEF_VER_W;
  double                   DEF_HOR_X, DEF_HOR_Y, DEF_HOR_H, DEF_HOR_W;

  QGroupBox*               myRangeGrp;
  QLineEdit*               myMinEdit;
  QLineEdit*               myMaxEdit;
  QCheckBox*               myLogarithmicCheck;

  QGroupBox*               myFontGrp;
  QtxColorButton*          myTitleColorBtn;
  QComboBox*               myTitleFontCombo;
  QCheckBox*               myTitleBoldCheck;
  QCheckBox*               myTitleItalicCheck;
  QCheckBox*               myTitleShadowCheck;
  QtxColorButton*          myLabelsColorBtn;
  QComboBox*               myLabelsFontCombo;
  QCheckBox*               myLabelsBoldCheck;
  QCheckBox*               myLabelsItalicCheck;
  QCheckBox*               myLabelsShadowCheck;
  
  QGroupBox*               myLabColorGrp;
  SalomeApp_IntSpinBox*    myColorsSpin;
  SalomeApp_IntSpinBox*    myLabelsSpin;

  QGroupBox*               myOrientationGrp;
  QRadioButton*            myVertRadioBtn;
  QRadioButton*            myHorizRadioBtn;

  QGroupBox*               myOriginDimGrp;
  SMESHGUI_SpinBox*        myXSpin;
  SMESHGUI_SpinBox*        myYSpin;
  SMESHGUI_SpinBox*        myWidthSpin;
  SMESHGUI_SpinBox*        myHeightSpin;

  QGroupBox*               myDistributionGrp;
  QRadioButton*            myDMonoColor;
  QRadioButton*            myDMultiColor;
  QtxColorButton*          myMonoColorBtn;
  QLabel*                  myDistributionColorLbl;

  QGroupBox*               myButtonGrp;
  QButtonGroup*            myDistribColorGrp;
  QPushButton*             myOkBtn;
  QPushButton*             myApplyBtn;
  QPushButton*             myCancelBtn;
  QPushButton*             myHelpBtn;

  QString                  myHelpFileName;
};

#endif // SMESHGUI_PREFERENCES_SCALARBARDLG_H
