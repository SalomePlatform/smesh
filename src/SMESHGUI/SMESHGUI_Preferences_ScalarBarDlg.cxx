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
//  File   : SMESHGUI_Preferences_ScalarBarDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_Preferences_ScalarBarDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvalidator.h>
#include <qcolordialog.h>

#include <vtkTextProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>

#include "QAD_SpinBoxDbl.h"
#include "QAD_Config.h"
#include "SALOME_Selection.h"
#include "SMESHGUI.h"
#include "SMESH_Actor.h"

#define MINIMUM_WIDTH 70
#define MARGIN_SIZE   11
#define SPACING_SIZE   6

#define DEF_VER_X  0.01
#define DEF_VER_Y  0.10
#define DEF_VER_H  0.80
#define DEF_VER_W  0.10
#define DEF_HOR_X  0.20
#define DEF_HOR_Y  0.01
#define DEF_HOR_H  0.12
#define DEF_HOR_W  0.60

using namespace std;

// Only one instance is allowed
SMESHGUI_Preferences_ScalarBarDlg* SMESHGUI_Preferences_ScalarBarDlg::myDlg = 0;

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties
 *
 *  Gets the only instance of "Scalar Bar Properties" dialog box
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties(QWidget* parent, 
							    SALOME_Selection* Sel)
{
  if ( !myDlg ) {
    myDlg = new SMESHGUI_Preferences_ScalarBarDlg( parent, Sel, false );
    myDlg->show();
  }
  else {
    myDlg->show();
    myDlg->setActiveWindow();
    myDlg->raise();
    myDlg->setFocus();
  }
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::ScalarBarPreferences
 *
 *  Opens "Scalar Bar Preferences" dialog box
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::ScalarBarPreferences( QWidget* parent )
{
  SMESHGUI_Preferences_ScalarBarDlg* aDlg = new SMESHGUI_Preferences_ScalarBarDlg( parent, 0, true );
  aDlg->exec();
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::SMESHGUI_Preferences_ScalarBarDlg
 *
 *  Constructor
 */
//=================================================================================================
SMESHGUI_Preferences_ScalarBarDlg::SMESHGUI_Preferences_ScalarBarDlg(QWidget* parent, 
								     SALOME_Selection* Sel, 
								     bool modal)
     : QDialog( parent, 0, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose )
{
  setName( "SMESHGUI_Preferences_ScalarBarDlg" );
  setCaption( Sel ? tr( "SMESH_PROPERTIES_SCALARBAR" ) : tr( "SMESH_PREFERENCES_SCALARBAR" ) );
  setSizeGripEnabled( TRUE );

  mySelection = Sel;
  myActor = 0;

  /******************************************************************************/
  // Top layout
  QGridLayout* aTopLayout = new QGridLayout( this );
  aTopLayout->setSpacing( SPACING_SIZE ); aTopLayout->setMargin( MARGIN_SIZE );
  int aRow = 0;

  /******************************************************************************/
  // Scalar range
  if ( mySelection ) {
    myRangeGrp = new QGroupBox ( tr( "SMESH_RANGE_SCALARBAR" ), this, "myRangeGrp" );
    myRangeGrp->setColumnLayout( 0, Qt::Vertical );
    myRangeGrp->layout()->setSpacing( 0 ); myRangeGrp->layout()->setMargin( 0 );
    QGridLayout* myRangeGrpLayout = new QGridLayout( myRangeGrp->layout() );
    myRangeGrpLayout->setAlignment( Qt::AlignTop );
    myRangeGrpLayout->setSpacing( SPACING_SIZE ); myRangeGrpLayout->setMargin( MARGIN_SIZE );
    
    myMinEdit = new QLineEdit( myRangeGrp, "myMinEdit" );
    myMinEdit->setMinimumWidth( MINIMUM_WIDTH );
    myMinEdit->setValidator( new QDoubleValidator( this ) );

    myMaxEdit = new QLineEdit( myRangeGrp, "myMaxEdit" );
    myMaxEdit->setMinimumWidth( MINIMUM_WIDTH );
    myMaxEdit->setValidator( new QDoubleValidator( this ) );

    myRangeGrpLayout->addWidget( new QLabel( tr( "SMESH_RANGE_MIN" ), myRangeGrp, "myMinLab" ), 0, 0 );
    myRangeGrpLayout->addWidget( myMinEdit, 0, 1 );
    myRangeGrpLayout->addWidget( new QLabel( tr( "SMESH_RANGE_MAX" ), myRangeGrp, "myMaxLab" ), 0, 2 );
    myRangeGrpLayout->addWidget( myMaxEdit, 0, 3 );

    aTopLayout->addWidget( myRangeGrp, aRow, 0 );
    aRow++;
  }

  /******************************************************************************/
  // Text properties
  myFontGrp = new QGroupBox ( tr( "SMESH_FONT_SCALARBAR" ), this, "myFontGrp" );
  myFontGrp->setColumnLayout( 0, Qt::Vertical );
  myFontGrp->layout()->setSpacing( 0 ); myFontGrp->layout()->setMargin( 0 );
  QGridLayout* myFontGrpLayout = new QGridLayout( myFontGrp->layout() );
  myFontGrpLayout->setAlignment( Qt::AlignTop );
  myFontGrpLayout->setSpacing( SPACING_SIZE ); myFontGrpLayout->setMargin( MARGIN_SIZE );
    
  myTitleColorBtn = new QToolButton( myFontGrp, "myTitleColorBtn" );
  
  myTitleFontCombo = new QComboBox( false, myFontGrp, "myTitleFontCombo" );
  myTitleFontCombo->setMinimumWidth( MINIMUM_WIDTH );
  myTitleFontCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myTitleFontCombo->insertItem( tr( "SMESH_FONT_ARIAL" ) );
  myTitleFontCombo->insertItem( tr( "SMESH_FONT_COURIER" ) );
  myTitleFontCombo->insertItem( tr( "SMESH_FONT_TIMES" ) );
  
  myTitleBoldCheck   = new QCheckBox( tr( "SMESH_FONT_BOLD" ),   myFontGrp, "myTitleBoldCheck" );
  myTitleItalicCheck = new QCheckBox( tr( "SMESH_FONT_ITALIC" ), myFontGrp, "myTitleItalicCheck" );
  myTitleShadowCheck = new QCheckBox( tr( "SMESH_FONT_SHADOW" ), myFontGrp, "myTitleShadowCheck" );

  myLabelsColorBtn = new QToolButton( myFontGrp, "myLabelsColorBtn" );
  
  myLabelsFontCombo = new QComboBox( false, myFontGrp, "myLabelsFontCombo" );
  myLabelsFontCombo->setMinimumWidth( MINIMUM_WIDTH );
  myLabelsFontCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myLabelsFontCombo->insertItem( tr( "SMESH_FONT_ARIAL" ) );
  myLabelsFontCombo->insertItem( tr( "SMESH_FONT_COURIER" ) );
  myLabelsFontCombo->insertItem( tr( "SMESH_FONT_TIMES" ) );
  
  myLabelsBoldCheck   = new QCheckBox( tr( "SMESH_FONT_BOLD" ),   myFontGrp, "myLabelsBoldCheck" );
  myLabelsItalicCheck = new QCheckBox( tr( "SMESH_FONT_ITALIC" ), myFontGrp, "myLabelsItalicCheck" );
  myLabelsShadowCheck = new QCheckBox( tr( "SMESH_FONT_SHADOW" ), myFontGrp, "myLabelsShadowCheck" );

  myFontGrpLayout->addWidget( new QLabel( tr( "SMESH_TITLE" ), myFontGrp, "myFontTitleLab" ), 0, 0 );
  myFontGrpLayout->addWidget( myTitleColorBtn,    0, 1 );
  myFontGrpLayout->addWidget( myTitleFontCombo,   0, 2 );
  myFontGrpLayout->addWidget( myTitleBoldCheck,   0, 3 );
  myFontGrpLayout->addWidget( myTitleItalicCheck, 0, 4 );
  myFontGrpLayout->addWidget( myTitleShadowCheck, 0, 5 );
  
  myFontGrpLayout->addWidget( new QLabel( tr( "SMESH_LABELS" ), myFontGrp, "myFontLabelsLab" ), 1, 0 );
  myFontGrpLayout->addWidget( myLabelsColorBtn,    1, 1 );
  myFontGrpLayout->addWidget( myLabelsFontCombo,   1, 2 );
  myFontGrpLayout->addWidget( myLabelsBoldCheck,   1, 3 );
  myFontGrpLayout->addWidget( myLabelsItalicCheck, 1, 4 );
  myFontGrpLayout->addWidget( myLabelsShadowCheck, 1, 5 );

  aTopLayout->addWidget( myFontGrp, aRow, 0 );
  aRow++;

  /******************************************************************************/
  // Labels & Colors
  myLabColorGrp = new QGroupBox ( tr( "SMESH_LABELS_COLORS_SCALARBAR" ), this, "myLabColorGrp" );
  myLabColorGrp->setColumnLayout( 0, Qt::Vertical );
  myLabColorGrp->layout()->setSpacing( 0 ); myLabColorGrp->layout()->setMargin( 0 );
  QGridLayout* myLabColorGrpLayout = new QGridLayout( myLabColorGrp->layout() );
  myLabColorGrpLayout->setAlignment( Qt::AlignTop );
  myLabColorGrpLayout->setSpacing( SPACING_SIZE ); myLabColorGrpLayout->setMargin( MARGIN_SIZE );
    
  myColorsSpin = new QSpinBox( 2, 256, 1, myLabColorGrp, "myColorsSpin" );
  myColorsSpin->setMinimumWidth( MINIMUM_WIDTH );
  myColorsSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myLabelsSpin = new QSpinBox( 2, 65, 1, myLabColorGrp, "myLabelsSpin" );
  myLabelsSpin->setMinimumWidth( MINIMUM_WIDTH );
  myLabelsSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myLabColorGrpLayout->addWidget( new QLabel( tr( "SMESH_NUMBEROFCOLORS" ), myLabColorGrp, "myNbColorLab" ), 0, 0 );
  myLabColorGrpLayout->addWidget( myColorsSpin, 0, 1 );
  myLabColorGrpLayout->addWidget( new QLabel( tr( "SMESH_NUMBEROFLABELS" ), myLabColorGrp, "myNbLabsLab" ), 0, 2 );
  myLabColorGrpLayout->addWidget( myLabelsSpin, 0, 3 );

  aTopLayout->addWidget( myLabColorGrp, aRow, 0 );
  aRow++;

  /******************************************************************************/
  // Orientation
  myOrientationGrp = new QButtonGroup ( tr( "SMESH_ORIENTATION" ), this, "myOrientationGrp" );
  myOrientationGrp->setColumnLayout( 0, Qt::Vertical );
  myOrientationGrp->layout()->setSpacing( 0 ); myOrientationGrp->layout()->setMargin( 0 );
  QGridLayout* myOrientationGrpLayout = new QGridLayout( myOrientationGrp->layout() );
  myOrientationGrpLayout->setAlignment( Qt::AlignTop );
  myOrientationGrpLayout->setSpacing( SPACING_SIZE ); myOrientationGrpLayout->setMargin( MARGIN_SIZE );
    
  myVertRadioBtn  = new QRadioButton( tr( "SMESH_VERTICAL" ),   myOrientationGrp, "myVertRadioBtn" );
  myHorizRadioBtn = new QRadioButton( tr( "SMESH_HORIZONTAL" ), myOrientationGrp, "myHorizRadioBtn" );
  myVertRadioBtn->setChecked( true );

  myOrientationGrpLayout->addWidget( myVertRadioBtn,  0, 0 );
  myOrientationGrpLayout->addWidget( myHorizRadioBtn, 0, 1 );

  aTopLayout->addWidget( myOrientationGrp, aRow, 0 );
  aRow++;

  /******************************************************************************/
  // Position & Size
  myOriginDimGrp = new QGroupBox ( tr( "SMESH_POSITION_SIZE_SCALARBAR" ), this, "myOriginDimGrp" );
  myOriginDimGrp->setColumnLayout( 0, Qt::Vertical );
  myOriginDimGrp->layout()->setSpacing( 0 ); myOriginDimGrp->layout()->setMargin( 0 );
  QGridLayout* myOriginDimGrpLayout = new QGridLayout( myOriginDimGrp->layout() );
  myOriginDimGrpLayout->setAlignment( Qt::AlignTop );
  myOriginDimGrpLayout->setSpacing( SPACING_SIZE ); myOriginDimGrpLayout->setMargin( MARGIN_SIZE );
    
  myXSpin = new QAD_SpinBoxDbl( myOriginDimGrp, 0.0, 1.0, 0.1 );
  myXSpin->setMinimumWidth( MINIMUM_WIDTH );
  myXSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myYSpin = new QAD_SpinBoxDbl( myOriginDimGrp, 0.0, 1.0, 0.1 );
  myYSpin->setMinimumWidth( MINIMUM_WIDTH );
  myYSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myWidthSpin = new QAD_SpinBoxDbl( myOriginDimGrp, 0.0, 1.0, 0.1 );
  myWidthSpin->setMinimumWidth( MINIMUM_WIDTH );
  myWidthSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myHeightSpin = new QAD_SpinBoxDbl( myOriginDimGrp, 0.0, 1.0, 0.1 );
  myHeightSpin->setMinimumWidth( MINIMUM_WIDTH );
  myHeightSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_X_SCALARBAR" ), myOriginDimGrp, "myXLab" ), 0, 0 );
  myOriginDimGrpLayout->addWidget( myXSpin, 0, 1 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_Y_SCALARBAR" ), myOriginDimGrp, "myYLab" ), 0, 2 );
  myOriginDimGrpLayout->addWidget( myYSpin, 0, 3 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_WIDTH" ),  myOriginDimGrp, "myWidthLab" ),  1, 0 );
  myOriginDimGrpLayout->addWidget( myWidthSpin, 1, 1 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_HEIGHT" ), myOriginDimGrp, "myHeightLab" ), 1, 2 );
  myOriginDimGrpLayout->addWidget( myHeightSpin, 1, 3 );

  aTopLayout->addWidget( myOriginDimGrp, aRow, 0 );
  aRow++;

  /***************************************************************/
  // Common buttons
  myButtonGrp = new QGroupBox( this, "myButtonGrp" );
  myButtonGrp->setColumnLayout(0, Qt::Vertical );
  myButtonGrp->layout()->setSpacing( 0 ); myButtonGrp->layout()->setMargin( 0 );
  QHBoxLayout* myButtonGrpLayout = new QHBoxLayout( myButtonGrp->layout() );
  myButtonGrpLayout->setAlignment( Qt::AlignTop );
  myButtonGrpLayout->setSpacing( SPACING_SIZE ); myButtonGrpLayout->setMargin( MARGIN_SIZE );

  myOkBtn = new QPushButton( tr( "SMESH_BUT_OK" ), myButtonGrp, "myOkBtn" );
  myOkBtn->setAutoDefault( TRUE ); myOkBtn->setDefault( TRUE );
  myButtonGrpLayout->addWidget( myOkBtn );
  if ( mySelection ) {
    myApplyBtn = new QPushButton( tr( "SMESH_BUT_APPLY" ), myButtonGrp, "myApplyBtn" );
    myApplyBtn->setAutoDefault( TRUE );
    myButtonGrpLayout->addWidget( myApplyBtn );
  }
  myButtonGrpLayout->addStretch();
  myCancelBtn = new QPushButton( tr( "SMESH_BUT_CANCEL" ), myButtonGrp, "myCancelBtn" );
  if ( mySelection )
    myCancelBtn->setText( tr( "SMESH_BUT_CLOSE" ) );
  myCancelBtn->setAutoDefault( TRUE );
  myButtonGrpLayout->addWidget( myCancelBtn );

  aTopLayout->addWidget( myButtonGrp, aRow, 0 );

  /***************************************************************/
  // Init
  // --> first init from preferences
  QColor titleColor( 255, 255, 255 );
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarTitleColor" ) ) {
    QStringList aTColor = QStringList::split(  ":", QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleColor" ), false );
    titleColor = QColor( ( aTColor.count() > 0 ? aTColor[0].toInt() : 255 ), 
			 ( aTColor.count() > 1 ? aTColor[1].toInt() : 255 ), 
			 ( aTColor.count() > 2 ? aTColor[2].toInt() : 255 ) );
  }
  myTitleColorBtn->setPaletteBackgroundColor( titleColor );
  myTitleFontCombo->setCurrentItem( 0 );
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarTitleFont" ) ) {
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Arial" )
      myTitleFontCombo->setCurrentItem( 0 );
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Courier" )
      myTitleFontCombo->setCurrentItem( 1 );
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Times" )
      myTitleFontCombo->setCurrentItem( 2 );
  }
  myTitleBoldCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleBold" ) == "true" );
  myTitleItalicCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleItalic" ) == "true" );
  myTitleShadowCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleShadow" ) == "true" );

  QColor labelColor( 255, 255, 255 );
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarLabelColor" ) ) {
    QStringList aLColor = QStringList::split( ":", QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelColor" ), false );
    labelColor = QColor( ( aLColor.count() > 0 ? aLColor[0].toInt() : 255 ), 
			 ( aLColor.count() > 1 ? aLColor[1].toInt() : 255 ), 
			 ( aLColor.count() > 2 ? aLColor[2].toInt() : 255 ) );
  }
  myLabelsColorBtn->setPaletteBackgroundColor( labelColor );
  myLabelsFontCombo->setCurrentItem( 0 );
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarLabelFont" ) ) {
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Arial" )
      myLabelsFontCombo->setCurrentItem( 0 );
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Courier" )
      myLabelsFontCombo->setCurrentItem( 1 );
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Times" )
      myLabelsFontCombo->setCurrentItem( 2 );
  }
  myLabelsBoldCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelBold" ) == "true" );
  myLabelsItalicCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelItalic" ) == "true" );
  myLabelsShadowCheck->setChecked( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelShadow" ) == "true" );

  int aNbColors = 64;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarNbOfColors" ) )
    aNbColors = QAD_CONFIG->getSetting( "SMESH:ScalarBarNbOfColors" ).toInt();
  myColorsSpin->setValue( aNbColors );
  int aNbLabels = 5;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarNbOfLabels" ) )
    aNbLabels = QAD_CONFIG->getSetting( "SMESH:ScalarBarNbOfLabels" ).toInt();
  myLabelsSpin->setValue( aNbLabels );

  QString aOrientation = QAD_CONFIG->getSetting( "SMESH:ScalarBarOrientation" );
  if ( aOrientation == "Horizontal" )
    myHorizRadioBtn->setChecked( true );
  else
    myVertRadioBtn->setChecked( true );
  myIniOrientation = myVertRadioBtn->isChecked();

  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarXPosition" ) )
    myIniX = QAD_CONFIG->getSetting( "SMESH:ScalarBarXPosition" ).toDouble();
  else
    myIniX = myHorizRadioBtn->isChecked() ? DEF_HOR_X : DEF_VER_X;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarYPosition" ) )
    myIniY = QAD_CONFIG->getSetting( "SMESH:ScalarBarYPosition" ).toDouble();
  else
    myIniY = myHorizRadioBtn->isChecked() ? DEF_HOR_Y : DEF_VER_Y;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarWidth" ) )
    myIniW = QAD_CONFIG->getSetting( "SMESH:ScalarBarWidth" ).toDouble();
  else
    myIniW = myHorizRadioBtn->isChecked() ? DEF_HOR_W : DEF_VER_W;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarHeight" ) )
    myIniH = QAD_CONFIG->getSetting( "SMESH:ScalarBarHeight" ).toDouble();
  else
    myIniH = myHorizRadioBtn->isChecked() ? DEF_HOR_H : DEF_VER_H;
  setOriginAndSize( myIniX, myIniY, myIniW, myIniH );

  if ( mySelection ) {
    // --> then init from selection if necessary
    onSelectionChanged();
  }
  
  /***************************************************************/
  // Connect section
  connect( myTitleColorBtn,     SIGNAL( clicked() ), this, SLOT( onTitleColor() ) );
  connect( myLabelsColorBtn,    SIGNAL( clicked() ), this, SLOT( onLabelsColor() ) );
  connect( myOkBtn,             SIGNAL( clicked() ), this, SLOT( onOk() ) );
  connect( myCancelBtn,         SIGNAL( clicked() ), this, SLOT( onCancel() ) );
  connect( myXSpin,             SIGNAL( valueChanged( double ) ), this, SLOT( onXYChanged() ) );
  connect( myYSpin,             SIGNAL( valueChanged( double ) ), this, SLOT( onXYChanged() ) );
  connect( myOrientationGrp,    SIGNAL( clicked( int ) ), this, SLOT( onOrientationChanged() ) );
  if ( mySelection ) {
    connect( myApplyBtn,        SIGNAL( clicked() ), this, SLOT( onApply() ) );
    connect( mySelection,       SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  }
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalCloseAllDialogs() ), this, SLOT( onCancel() ) ) ;
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::~SMESHGUI_Preferences_ScalarBarDlg
 *
 *  Destructor
 */
//=================================================================================================
SMESHGUI_Preferences_ScalarBarDlg::~SMESHGUI_Preferences_ScalarBarDlg()
{
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onOk
 *
 *  OK button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onOk()
{
  if ( onApply() )
    onCancel();
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onApply
 *
 *  Apply button slot
 */
//=================================================================================================
bool SMESHGUI_Preferences_ScalarBarDlg::onApply()
{
  if ( mySelection ) {
    // Scalar Bar properties
    if ( !myActor )
      return false;
    vtkScalarBarActor* myScalarBarActor = myActor->GetScalarBarActor();

    vtkTextProperty* aTitleTextPrp = myScalarBarActor->GetTitleTextProperty();
    QColor aTColor = myTitleColorBtn->paletteBackgroundColor();
    aTitleTextPrp->SetColor( aTColor.red()/255., aTColor.green()/255., aTColor.blue()/255. );
    if ( myTitleFontCombo->currentItem() == 0 )
      aTitleTextPrp->SetFontFamilyToArial();
    else if ( myTitleFontCombo->currentItem() == 1 )
      aTitleTextPrp->SetFontFamilyToCourier();
    else
      aTitleTextPrp->SetFontFamilyToTimes();
    aTitleTextPrp->SetBold( myTitleBoldCheck->isChecked() );
    aTitleTextPrp->SetItalic( myTitleItalicCheck->isChecked() );
    aTitleTextPrp->SetShadow( myTitleShadowCheck->isChecked() );
    myScalarBarActor->SetTitleTextProperty( aTitleTextPrp );

    vtkTextProperty* aLabelsTextPrp = myScalarBarActor->GetLabelTextProperty();
    QColor aLColor = myLabelsColorBtn->paletteBackgroundColor();
    aLabelsTextPrp->SetColor( aLColor.red()/255., aLColor.green()/255., aLColor.blue()/255. );
    if ( myLabelsFontCombo->currentItem() == 0 )
      aLabelsTextPrp->SetFontFamilyToArial();
    else if ( myLabelsFontCombo->currentItem() == 1 )
      aLabelsTextPrp->SetFontFamilyToCourier();
    else
      aLabelsTextPrp->SetFontFamilyToTimes();
    aLabelsTextPrp->SetBold( myLabelsBoldCheck->isChecked() );
    aLabelsTextPrp->SetItalic( myLabelsItalicCheck->isChecked() );
    aLabelsTextPrp->SetShadow( myLabelsShadowCheck->isChecked() );
    myScalarBarActor->SetLabelTextProperty( aLabelsTextPrp );

    myScalarBarActor->SetNumberOfLabels( myLabelsSpin->value() );
    myScalarBarActor->SetMaximumNumberOfColors( myColorsSpin->value() );

    if ( myHorizRadioBtn->isChecked() )
      myScalarBarActor->SetOrientationToHorizontal();
    else
      myScalarBarActor->SetOrientationToVertical();

    myScalarBarActor->SetPosition( myXSpin->value(), myYSpin->value() );
    myScalarBarActor->SetWidth( myWidthSpin->value() );
    myScalarBarActor->SetHeight( myHeightSpin->value() );

    double aMin = myMinEdit->text().toDouble();
    double aMax = myMaxEdit->text().toDouble();
    myScalarBarActor->GetLookupTable()->SetRange( aMin, aMax );
    SMESH::RepaintCurrentView();
  }
  else {
    // Scalar Bar preferences
    QColor titleColor = myTitleColorBtn->paletteBackgroundColor();
    QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleColor",  QString().sprintf( "%d:%d:%d", titleColor.red(), titleColor.green(), titleColor.blue() ) );
    if ( myTitleFontCombo->currentItem() == 0 )
      QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleFont", "Arial" );
    else if ( myTitleFontCombo->currentItem() == 1 )
      QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleFont", "Courier" );
    else
      QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleFont", "Times" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleBold",   myTitleBoldCheck->isChecked() ?   "true" : "false" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleItalic", myTitleItalicCheck->isChecked() ? "true" : "false" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarTitleShadow", myTitleShadowCheck->isChecked() ? "true" : "false" );

    QColor labelColor = myLabelsColorBtn->paletteBackgroundColor();
    QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelColor",  QString().sprintf( "%d:%d:%d", labelColor.red(), labelColor.green(),labelColor. blue() ) );
    if ( myLabelsFontCombo->currentItem() == 0 )
      QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelFont", "Arial" );
    else if ( myLabelsFontCombo->currentItem() == 1 )
      QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelFont", "Courier" );
    else
      QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelFont", "Times" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelBold",   myLabelsBoldCheck->isChecked() ?   "true" : "false" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelItalic", myLabelsItalicCheck->isChecked() ? "true" : "false" );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarLabelShadow", myLabelsShadowCheck->isChecked() ? "true" : "false" );

    QAD_CONFIG->addSetting( "SMESH:ScalarBarNbOfColors", myColorsSpin->value() );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarNbOfLabels", myLabelsSpin->value() );

    QAD_CONFIG->addSetting( "SMESH:ScalarBarOrientation", myHorizRadioBtn->isChecked() ? "Horizontal" : "Vertical" );
    
    QAD_CONFIG->addSetting( "SMESH:ScalarBarXPosition", myXSpin->value() );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarYPosition", myYSpin->value() );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarWidth",     myWidthSpin->value() );
    QAD_CONFIG->addSetting( "SMESH:ScalarBarHeight",    myHeightSpin->value() );
  }
  return true;
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onCancel
 *
 *  Cancel button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onCancel()
{
  close();
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onTitleColor
 *
 *  Change Title color button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onTitleColor()
{
  QColor aColor = myTitleColorBtn->paletteBackgroundColor();
  aColor = QColorDialog::getColor( aColor, this );
  if ( aColor.isValid() ) 
    myTitleColorBtn->setPaletteBackgroundColor( aColor );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onLabelsColor
 *
 *  Change Labels color button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onLabelsColor()
{
  QColor aColor = myLabelsColorBtn->paletteBackgroundColor();
  aColor = QColorDialog::getColor( aColor, this );
  if ( aColor.isValid() ) 
    myLabelsColorBtn->setPaletteBackgroundColor( aColor );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onSelectionChanged
 *
 *  Called when selection changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onSelectionChanged()
{
  if( mySelection ) {
    if ( mySelection->IObjectCount() == 1 ) {
      Handle(SALOME_InteractiveObject) anIO = mySelection->firstIObject();
      if( anIO->hasEntry() ) {
	SMESH_Actor* anActor = SMESH::FindActorByEntry(anIO->getEntry());
	if ( anActor && anActor->GetScalarBarActor() && anActor->GetControlMode() != SMESH_Actor::eNone ) {
	  myActor = anActor;
	  vtkScalarBarActor* myScalarBarActor = myActor->GetScalarBarActor();
	  
	  if ( myScalarBarActor->GetLookupTable() ) {
	    float *range = myScalarBarActor->GetLookupTable()->GetRange();
	    myMinEdit->setText( QString::number( range[0] ) );
	    myMaxEdit->setText( QString::number( range[1] ) );
	  }

	  vtkTextProperty* aTitleTextPrp = myScalarBarActor->GetTitleTextProperty();
	  float aTColor[3];
	  aTitleTextPrp->GetColor( aTColor );
	  myTitleColorBtn->setPaletteBackgroundColor( QColor( (int)( aTColor[0]*255 ), (int)( aTColor[1]*255 ), (int)( aTColor[2]*255 ) ) );
	  myTitleFontCombo->setCurrentItem( aTitleTextPrp->GetFontFamily() );
	  myTitleBoldCheck->setChecked( aTitleTextPrp->GetBold() );
	  myTitleItalicCheck->setChecked( aTitleTextPrp->GetItalic() );
	  myTitleShadowCheck->setChecked( aTitleTextPrp->GetShadow() );

	  vtkTextProperty* aLabelsTextPrp = myScalarBarActor->GetLabelTextProperty();
	  float aLColor[3];
	  aLabelsTextPrp->GetColor( aLColor );
	  myLabelsColorBtn->setPaletteBackgroundColor( QColor( (int)( aLColor[0]*255 ), (int)( aLColor[1]*255 ), (int)( aLColor[2]*255 ) ) );
	  myLabelsFontCombo->setCurrentItem( aLabelsTextPrp->GetFontFamily() );
	  myLabelsBoldCheck->setChecked( aLabelsTextPrp->GetBold() );
	  myLabelsItalicCheck->setChecked( aLabelsTextPrp->GetItalic() );
	  myLabelsShadowCheck->setChecked( aLabelsTextPrp->GetShadow() );

	  myLabelsSpin->setValue( myScalarBarActor->GetNumberOfLabels() );
	  myColorsSpin->setValue( myScalarBarActor->GetMaximumNumberOfColors() );

	  if ( myScalarBarActor->GetOrientation() == VTK_ORIENT_VERTICAL )
	    myVertRadioBtn->setChecked( true );
	  else
	    myHorizRadioBtn->setChecked( true );
	  myIniOrientation = myVertRadioBtn->isChecked();
	  
	  myIniX = myScalarBarActor->GetPosition()[0];
	  myIniY = myScalarBarActor->GetPosition()[1];
	  myIniW = myScalarBarActor->GetWidth();
	  myIniH = myScalarBarActor->GetHeight();
	  setOriginAndSize( myIniX, myIniY, myIniW, myIniH );

	  myRangeGrp->setEnabled( true );
	  myFontGrp->setEnabled( true );
	  myLabColorGrp->setEnabled( true );
	  myOrientationGrp->setEnabled( true );
	  myOriginDimGrp->setEnabled( true );
	  myOkBtn->setEnabled( true );
	  myApplyBtn->setEnabled( true );
	  return;
	}
      }
    }
    myActor = 0;
    myRangeGrp->setEnabled( false );
    myFontGrp->setEnabled( false );
    myLabColorGrp->setEnabled( false );
    myOrientationGrp->setEnabled( false );
    myOriginDimGrp->setEnabled( false );
    myOkBtn->setEnabled( false );
    myApplyBtn->setEnabled( false );
  }
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::closeEvent
 *
 *  Close event handler
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::closeEvent( QCloseEvent* e )
{
  if ( mySelection ) // "Properties" dialog box
    myDlg = 0;
  QDialog::closeEvent( e );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onXYChanged
 *
 *  Called when X, Y values are changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onXYChanged()
{
  myWidthSpin->setMaxValue( 1.0 - myXSpin->value() );
  myHeightSpin->setMaxValue( 1.0 - myYSpin->value() );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::setOriginAndSize
 *
 *  Called when X, Y values are changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::setOriginAndSize( const double x,
							  const double y,
							  const double w,
							  const double h )
{
  blockSignals( true );
  myXSpin->setValue( x );
  myYSpin->setValue( y );
  myWidthSpin->setMaxValue( 1.0 );
  myWidthSpin->setValue( w );
  myHeightSpin->setMaxValue( 1.0 );
  myHeightSpin->setValue( h );
  blockSignals( false );
  onXYChanged();
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onOrientationChanged
 *
 *  Called when orientation is changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onOrientationChanged() 
{
  int aOrientation = myVertRadioBtn->isChecked();
  if ( aOrientation == myIniOrientation )
    setOriginAndSize( myIniX, myIniY, myIniW, myIniH );
  else
    setOriginAndSize( aOrientation ? DEF_VER_X : DEF_HOR_X, 
		      aOrientation ? DEF_VER_Y : DEF_HOR_Y, 
		      aOrientation ? DEF_VER_W : DEF_HOR_W, 
		      aOrientation ? DEF_VER_H : DEF_HOR_H );
}
