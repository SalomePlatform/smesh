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
// File   : SMESHGUI_Preferences_ScalarBarDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes

#include "SMESHGUI_Preferences_ScalarBarDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_ScalarBarActor.h>
#include <SMESH_ControlsDef.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SalomeApp_IntSpinBox.h>

#include <QtxColorButton.h>

// Qt includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDoubleValidator>

// VTK includes
#include <vtkTextProperty.h>
#include <vtkLookupTable.h>

#define MINIMUM_WIDTH 70
#define MARGIN_SIZE   11
#define SPACING_SIZE   6

// Only one instance is allowed
SMESHGUI_Preferences_ScalarBarDlg* SMESHGUI_Preferences_ScalarBarDlg::myDlg = 0;

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties
 *
 *  Gets the only instance of "Scalar Bar Properties" dialog box
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties( SMESHGUI* theModule )
{
  if (!myDlg) {
    myDlg = new SMESHGUI_Preferences_ScalarBarDlg( theModule );
    myDlg->show();
  } else {
    myDlg->show();
    myDlg->raise();
    myDlg->activateWindow();
  }
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::SMESHGUI_Preferences_ScalarBarDlg
 *
 *  Constructor
 */
//=================================================================================================
SMESHGUI_Preferences_ScalarBarDlg::SMESHGUI_Preferences_ScalarBarDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  DEF_VER_X = 0.01;
  DEF_VER_Y = 0.10;
  DEF_VER_H = 0.80;
  DEF_VER_W = 0.10;
  DEF_HOR_X = 0.20;
  DEF_HOR_Y = 0.01;
  DEF_HOR_H = 0.12;
  DEF_HOR_W = 0.60;

  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr("SMESH_PROPERTIES_SCALARBAR") );
  setSizeGripEnabled(true);

  myActor = 0;

  /******************************************************************************/
  // Top layout
  QVBoxLayout* aTopLayout = new QVBoxLayout( this );
  aTopLayout->setSpacing( SPACING_SIZE ); aTopLayout->setMargin( MARGIN_SIZE );

  /******************************************************************************/
  // Scalar range
  myRangeGrp = new QGroupBox ( tr( "SMESH_RANGE_SCALARBAR" ), this );
  //QHBoxLayout* myRangeGrpLayout = new QHBoxLayout( myRangeGrp );
  QGridLayout* myRangeGrpLayout = new QGridLayout( myRangeGrp );
  myRangeGrpLayout->setSpacing( SPACING_SIZE ); myRangeGrpLayout->setMargin( MARGIN_SIZE );

  myMinEdit = new QLineEdit( myRangeGrp );
  myMinEdit->setMinimumWidth( MINIMUM_WIDTH );
  myMinEdit->setValidator( new QDoubleValidator( this ) );

  myMaxEdit = new QLineEdit( myRangeGrp );
  myMaxEdit->setMinimumWidth( MINIMUM_WIDTH );
  myMaxEdit->setValidator( new QDoubleValidator( this ) );

  myLogarithmicCheck = new QCheckBox (myRangeGrp);
  myLogarithmicCheck->setText(tr("SMESH_LOGARITHMIC_SCALARBAR"));
  myLogarithmicCheck->setChecked(false);

  myRangeGrpLayout->addWidget( new QLabel( tr( "SMESH_RANGE_MIN" ), myRangeGrp ), 0, 0, 1, 1 );
  myRangeGrpLayout->addWidget( myMinEdit, 0, 1, 1, 1 );
  myRangeGrpLayout->addWidget( new QLabel( tr( "SMESH_RANGE_MAX" ), myRangeGrp ), 0, 2, 1, 1 );
  myRangeGrpLayout->addWidget( myMaxEdit, 0, 3, 1, 1 );
  myRangeGrpLayout->addWidget( myLogarithmicCheck, 1, 0, 1, 4 );

  aTopLayout->addWidget( myRangeGrp );

  /******************************************************************************/
  // Text properties
  myFontGrp = new QGroupBox ( tr( "SMESH_FONT_SCALARBAR" ), this  );
  QGridLayout* myFontGrpLayout = new QGridLayout( myFontGrp );
  myFontGrpLayout->setSpacing( SPACING_SIZE ); myFontGrpLayout->setMargin( MARGIN_SIZE );

  myTitleColorBtn = new QtxColorButton( myFontGrp  );

  myTitleFontCombo = new QComboBox( myFontGrp );
  myTitleFontCombo->setMinimumWidth( MINIMUM_WIDTH );
  myTitleFontCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myTitleFontCombo->addItem( tr( "SMESH_FONT_ARIAL" ) );
  myTitleFontCombo->addItem( tr( "SMESH_FONT_COURIER" ) );
  myTitleFontCombo->addItem( tr( "SMESH_FONT_TIMES" ) );

  myTitleBoldCheck   = new QCheckBox( tr( "SMESH_FONT_BOLD" ),   myFontGrp );
  myTitleItalicCheck = new QCheckBox( tr( "SMESH_FONT_ITALIC" ), myFontGrp );
  myTitleShadowCheck = new QCheckBox( tr( "SMESH_FONT_SHADOW" ), myFontGrp );

  myLabelsColorBtn = new QtxColorButton( myFontGrp );

  myLabelsFontCombo = new QComboBox( myFontGrp );
  myLabelsFontCombo->setMinimumWidth( MINIMUM_WIDTH );
  myLabelsFontCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myLabelsFontCombo->addItem( tr( "SMESH_FONT_ARIAL" ) );
  myLabelsFontCombo->addItem( tr( "SMESH_FONT_COURIER" ) );
  myLabelsFontCombo->addItem( tr( "SMESH_FONT_TIMES" ) );

  myLabelsBoldCheck   = new QCheckBox( tr( "SMESH_FONT_BOLD" ),   myFontGrp );
  myLabelsItalicCheck = new QCheckBox( tr( "SMESH_FONT_ITALIC" ), myFontGrp );
  myLabelsShadowCheck = new QCheckBox( tr( "SMESH_FONT_SHADOW" ), myFontGrp );

  myFontGrpLayout->addWidget( new QLabel( tr( "SMESH_TITLE" ), myFontGrp ), 0, 0 );
  myFontGrpLayout->addWidget( myTitleColorBtn,    0, 1 );
  myFontGrpLayout->addWidget( myTitleFontCombo,   0, 2 );
  myFontGrpLayout->addWidget( myTitleBoldCheck,   0, 3 );
  myFontGrpLayout->addWidget( myTitleItalicCheck, 0, 4 );
  myFontGrpLayout->addWidget( myTitleShadowCheck, 0, 5 );

  myFontGrpLayout->addWidget( new QLabel( tr( "SMESH_LABELS" ), myFontGrp ), 1, 0 );
  myFontGrpLayout->addWidget( myLabelsColorBtn,    1, 1 );
  myFontGrpLayout->addWidget( myLabelsFontCombo,   1, 2 );
  myFontGrpLayout->addWidget( myLabelsBoldCheck,   1, 3 );
  myFontGrpLayout->addWidget( myLabelsItalicCheck, 1, 4 );
  myFontGrpLayout->addWidget( myLabelsShadowCheck, 1, 5 );

  aTopLayout->addWidget( myFontGrp );

  /******************************************************************************/
  // Labels & Colors
  myLabColorGrp = new QGroupBox ( tr( "SMESH_LABELS_COLORS_SCALARBAR" ), this );
  QHBoxLayout* myLabColorGrpLayout = new QHBoxLayout( myLabColorGrp );
  myLabColorGrpLayout->setSpacing( SPACING_SIZE ); myLabColorGrpLayout->setMargin( MARGIN_SIZE );

  myColorsSpin = new SalomeApp_IntSpinBox( myLabColorGrp );
  myColorsSpin->setAcceptNames( false ); // No Notebook variables allowed
  myColorsSpin->setRange( 2, 256 );
  myColorsSpin->setSingleStep( 1 );
  myColorsSpin->setMinimumWidth( MINIMUM_WIDTH );
  myColorsSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myLabelsSpin = new SalomeApp_IntSpinBox( myLabColorGrp );
  myLabelsSpin->setAcceptNames( false ); // No Notebook variables allowed
  myLabelsSpin->setRange( 2, 65 );
  myLabelsSpin->setSingleStep( 1 );
  myLabelsSpin->setMinimumWidth( MINIMUM_WIDTH );
  myLabelsSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myLabColorGrpLayout->addWidget( new QLabel( tr( "SMESH_NUMBEROFCOLORS" ), myLabColorGrp ) );
  myLabColorGrpLayout->addWidget( myColorsSpin );
  myLabColorGrpLayout->addWidget( new QLabel( tr( "SMESH_NUMBEROFLABELS" ), myLabColorGrp ) );
  myLabColorGrpLayout->addWidget( myLabelsSpin );

  aTopLayout->addWidget( myLabColorGrp );

  /******************************************************************************/
  // Orientation
  myOrientationGrp = new QGroupBox ( tr( "SMESH_ORIENTATION" ), this );
  QButtonGroup* aOrientationGrp = new QButtonGroup( this );
  QHBoxLayout* myOrientationGrpLayout = new QHBoxLayout( myOrientationGrp );
  myOrientationGrpLayout->setSpacing( SPACING_SIZE ); myOrientationGrpLayout->setMargin( MARGIN_SIZE );

  myVertRadioBtn  = new QRadioButton( tr( "SMESH_VERTICAL" ),   myOrientationGrp );
  myHorizRadioBtn = new QRadioButton( tr( "SMESH_HORIZONTAL" ), myOrientationGrp );
  myVertRadioBtn->setChecked( true );

  myOrientationGrpLayout->addWidget( myVertRadioBtn );
  myOrientationGrpLayout->addWidget( myHorizRadioBtn );
  aOrientationGrp->addButton(myVertRadioBtn);
  aOrientationGrp->addButton(myHorizRadioBtn);

  aTopLayout->addWidget( myOrientationGrp );

  /******************************************************************************/
  // Position & Size
  myOriginDimGrp = new QGroupBox ( tr("SMESH_POSITION_SIZE_SCALARBAR"), this );
  QGridLayout* myOriginDimGrpLayout = new QGridLayout( myOriginDimGrp );
  myOriginDimGrpLayout->setSpacing( SPACING_SIZE ); myOriginDimGrpLayout->setMargin( MARGIN_SIZE );

  myXSpin = new SMESHGUI_SpinBox(myOriginDimGrp);
  myXSpin->setAcceptNames( false );
  myXSpin->RangeStepAndValidator( 0.0, 1.0, 0.1, "parametric_precision" );
  myXSpin->setMinimumWidth( MINIMUM_WIDTH );
  myXSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myYSpin = new SMESHGUI_SpinBox(myOriginDimGrp);
  myYSpin->setAcceptNames( false );
  myYSpin->RangeStepAndValidator( 0.0, 1.0, 0.1, "parametric_precision" );
  myYSpin->setMinimumWidth( MINIMUM_WIDTH );
  myYSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myWidthSpin = new SMESHGUI_SpinBox(myOriginDimGrp);
  myWidthSpin->setAcceptNames( false );
  myWidthSpin->RangeStepAndValidator( 0.0, 1.0, 0.1, "parametric_precision" );
  myWidthSpin->setMinimumWidth( MINIMUM_WIDTH );
  myWidthSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myHeightSpin = new SMESHGUI_SpinBox(myOriginDimGrp);
  myHeightSpin->setAcceptNames( false );
  myHeightSpin->RangeStepAndValidator( 0.0, 1.0, 0.1, "parametric_precision" );
  myHeightSpin->setMinimumWidth( MINIMUM_WIDTH );
  myHeightSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_X_SCALARBAR" ), myOriginDimGrp ), 0, 0 );
  myOriginDimGrpLayout->addWidget( myXSpin, 0, 1 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_Y_SCALARBAR" ), myOriginDimGrp ), 0, 2 );
  myOriginDimGrpLayout->addWidget( myYSpin, 0, 3 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_WIDTH" ),  myOriginDimGrp ),  1, 0 );
  myOriginDimGrpLayout->addWidget( myWidthSpin, 1, 1 );
  myOriginDimGrpLayout->addWidget( new QLabel( tr( "SMESH_HEIGHT" ), myOriginDimGrp ), 1, 2 );
  myOriginDimGrpLayout->addWidget( myHeightSpin, 1, 3 );

  aTopLayout->addWidget( myOriginDimGrp );
  /******************************************************************************/

  // Destribution
  myDistributionGrp = new QGroupBox ( tr( "SMESH_DISTRIBUTION_SCALARBAR" ), this );
  myDistributionGrp->setCheckable(true);
  QHBoxLayout* aDistributionGrpLayout = new QHBoxLayout( myDistributionGrp );
  aDistributionGrpLayout->setSpacing( SPACING_SIZE ); aDistributionGrpLayout->setMargin( MARGIN_SIZE );

  myDistribColorGrp = new QButtonGroup( this );

  myDMonoColor  = new QRadioButton( tr( "SMESH_MONOCOLOR" ) ,  myDistributionGrp );
  myDMultiColor = new QRadioButton( tr( "SMESH_MULTICOLOR" ),  myDistributionGrp );
  myDMonoColor->setChecked( true );

  myDistribColorGrp->addButton(myDMonoColor);myDistribColorGrp->setId(myDMonoColor,1);
  myDistribColorGrp->addButton(myDMultiColor);myDistribColorGrp->setId(myDMultiColor,2);

  aDistributionGrpLayout->addWidget( myDMultiColor );
  aDistributionGrpLayout->addWidget( myDMonoColor );

  //Color of the Distribution in monocolor case:
  myDistributionColorLbl = new QLabel( tr( "SMESH_DISTRIBUTION_COLOR" ), myDistributionGrp );
  aDistributionGrpLayout->addWidget( myDistributionColorLbl );
  myMonoColorBtn = new QtxColorButton( myDistributionGrp  );
  aDistributionGrpLayout->addWidget(myMonoColorBtn);

  aTopLayout->addWidget(myDistributionGrp);

  /******************************************************************************/
  // Common buttons
  myButtonGrp = new QGroupBox( this );
  QHBoxLayout* myButtonGrpLayout = new QHBoxLayout( myButtonGrp );
  myButtonGrpLayout->setSpacing( SPACING_SIZE ); myButtonGrpLayout->setMargin( MARGIN_SIZE );

  myOkBtn = new QPushButton( tr( "SMESH_BUT_APPLY_AND_CLOSE" ), myButtonGrp );
  myOkBtn->setAutoDefault( true ); myOkBtn->setDefault( true );
  myApplyBtn = new QPushButton( tr( "SMESH_BUT_APPLY" ), myButtonGrp );
  myApplyBtn->setAutoDefault( true );
  myCancelBtn = new QPushButton( tr( "SMESH_BUT_CLOSE" ), myButtonGrp );
  myCancelBtn->setAutoDefault( true );
  myHelpBtn = new QPushButton( tr("SMESH_BUT_HELP"), myButtonGrp );
  myHelpBtn->setAutoDefault(true);

  myButtonGrpLayout->addWidget( myOkBtn );
  myButtonGrpLayout->addSpacing( 10 );
  myButtonGrpLayout->addWidget( myApplyBtn );
  myButtonGrpLayout->addSpacing( 10 );
  myButtonGrpLayout->addStretch();
  myButtonGrpLayout->addWidget( myCancelBtn );
  myButtonGrpLayout->addWidget( myHelpBtn );

  aTopLayout->addWidget( myButtonGrp );

  /***************************************************************/
  // Init
  // --> first init from preferences
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );

  QColor titleColor = mgr->colorValue("SMESH", "scalar_bar_title_color",
                                      QColor(255, 255, 255));
  myTitleColorBtn->setColor(titleColor);
  myTitleFontCombo->setCurrentIndex(0);
  if (mgr->hasValue("SMESH", "scalar_bar_title_font")) {
    QFont f = mgr->fontValue( "SMESH", "scalar_bar_title_font" );
    if( f.family()=="Arial" )
      myTitleFontCombo->setCurrentIndex(0);
    if( f.family()=="Courier" )
      myTitleFontCombo->setCurrentIndex(1);
    if( f.family()=="Times")
      myTitleFontCombo->setCurrentIndex(2);

    myTitleBoldCheck->setChecked  ( f.bold() );
    myTitleItalicCheck->setChecked( f.italic() );
    myTitleShadowCheck->setChecked( f.overline() );
  }

  QColor labelColor = mgr->colorValue("SMESH", "scalar_bar_label_color",
                                      QColor(255, 255, 255));
  myLabelsColorBtn->setColor(labelColor);
  myLabelsFontCombo->setCurrentIndex(0);
  if (mgr->hasValue("SMESH", "scalar_bar_label_font")) {
    QFont f = mgr->fontValue( "SMESH", "scalar_bar_label_font" );
    if (f.family() == "Arial")
      myLabelsFontCombo->setCurrentIndex(0);
    if (f.family() == "Courier")
      myLabelsFontCombo->setCurrentIndex(1);
    if (f.family() == "Times")
      myLabelsFontCombo->setCurrentIndex(2);

    myLabelsBoldCheck  ->setChecked( f.bold() );
    myLabelsItalicCheck->setChecked( f.italic() );
    myLabelsShadowCheck->setChecked( f.overline() );
  }

  int aNbColors = mgr->integerValue("SMESH", "scalar_bar_num_colors", 64);
  myColorsSpin->setValue(aNbColors);

  int aNbLabels = mgr->integerValue("SMESH", "scalar_bar_num_labels", 5);
  myLabelsSpin->setValue(aNbLabels);

  int aOrientation = mgr->integerValue( "SMESH", "scalar_bar_orientation", 1 );
  bool isHoriz = aOrientation == 1;
  if (isHoriz)
    myHorizRadioBtn->setChecked(true);
  else
    myVertRadioBtn->setChecked(true);
  myIniOrientation = myVertRadioBtn->isChecked();

  QString name = isHoriz ? "scalar_bar_horizontal_%1" : "scalar_bar_vertical_%1";

  myIniX = mgr->doubleValue("SMESH", name.arg( "x" ),
                            myHorizRadioBtn->isChecked() ? DEF_HOR_X : DEF_VER_X);

  myIniY = mgr->doubleValue("SMESH", name.arg( "y" ),
                            myHorizRadioBtn->isChecked() ? DEF_HOR_Y : DEF_VER_Y);

  myIniW = mgr->doubleValue("SMESH", name.arg( "width" ),
                            myHorizRadioBtn->isChecked() ? DEF_HOR_W : DEF_VER_W);

  myIniH = mgr->doubleValue("SMESH", name.arg( "height" ),
                            myHorizRadioBtn->isChecked() ? DEF_HOR_H : DEF_VER_H);

  setOriginAndSize(myIniX, myIniY, myIniW, myIniH);


  bool distributionVisibility = mgr->booleanValue("SMESH","distribution_visibility");
  myDistributionGrp->setChecked(distributionVisibility);

  int coloringType = mgr->integerValue("SMESH", "distribution_coloring_type", 0);
  if( coloringType == SMESH_MONOCOLOR_TYPE ) {
    myDMonoColor->setChecked(true);
    onDistributionChanged(myDistribColorGrp->id(myDMonoColor));
  } else {
    myDMultiColor->setChecked(true);
    onDistributionChanged(myDistribColorGrp->id(myDMultiColor));

  }

  QColor distributionColor = mgr->colorValue("SMESH", "distribution_color",
                                             QColor(255, 255, 255));
  myMonoColorBtn->setColor(distributionColor);

  // --> then init from selection if necessary
  onSelectionChanged();

  /***************************************************************/
  // Connect section
  connect( myOkBtn,             SIGNAL( clicked() ), this, SLOT( onOk() ) );
  connect( myApplyBtn,          SIGNAL( clicked() ), this, SLOT( onApply() ) );
  connect( myCancelBtn,         SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( myHelpBtn,           SIGNAL(clicked()),   this, SLOT( onHelp() ) );
  connect( myMinEdit,           SIGNAL( textChanged(const QString &) ), this, SLOT( onMinMaxChanged() ) );
  connect( myMaxEdit,           SIGNAL( textChanged(const QString &) ), this, SLOT( onMinMaxChanged() ) );
  connect( myXSpin,             SIGNAL( valueChanged( double ) ), this, SLOT( onXYChanged() ) );
  connect( myYSpin,             SIGNAL( valueChanged( double ) ), this, SLOT( onXYChanged() ) );
  connect( aOrientationGrp,     SIGNAL( buttonClicked( int ) ),   this, SLOT( onOrientationChanged() ) );
  connect( myDistributionGrp,   SIGNAL( toggled(bool) ), this, SLOT(onDistributionActivated(bool)) );
  connect( myDistribColorGrp,   SIGNAL( buttonClicked( int ) ),   this, SLOT( onDistributionChanged( int ) ) );
  connect( mySelectionMgr,      SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  connect( mySMESHGUI,          SIGNAL( SignalCloseAllDialogs() ),   this, SLOT( reject() ) );

  myHelpFileName = "scalar_bar_dlg.html";
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
    reject();
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
  // Scalar Bar properties
  if (!myActor)
    return false;
  SMESH_ScalarBarActor* myScalarBarActor = myActor->GetScalarBarActor();

  vtkTextProperty* aTitleTextPrp = myScalarBarActor->GetTitleTextProperty();
  QColor aTColor = myTitleColorBtn->color();
  aTitleTextPrp->SetColor( aTColor.red()/255., aTColor.green()/255., aTColor.blue()/255. );
  if ( myTitleFontCombo->currentIndex() == 0 )
    aTitleTextPrp->SetFontFamilyToArial();
  else if ( myTitleFontCombo->currentIndex() == 1 )
    aTitleTextPrp->SetFontFamilyToCourier();
  else
    aTitleTextPrp->SetFontFamilyToTimes();
  aTitleTextPrp->SetBold( myTitleBoldCheck->isChecked() );
  aTitleTextPrp->SetItalic( myTitleItalicCheck->isChecked() );
  aTitleTextPrp->SetShadow( myTitleShadowCheck->isChecked() );
  myScalarBarActor->SetTitleTextProperty( aTitleTextPrp );

  vtkTextProperty* aLabelsTextPrp = myScalarBarActor->GetLabelTextProperty();
  QColor aLColor = myLabelsColorBtn->color();
  aLabelsTextPrp->SetColor( aLColor.red()/255., aLColor.green()/255., aLColor.blue()/255. );
  if ( myLabelsFontCombo->currentIndex() == 0 )
    aLabelsTextPrp->SetFontFamilyToArial();
  else if ( myLabelsFontCombo->currentIndex() == 1 )
    aLabelsTextPrp->SetFontFamilyToCourier();
  else
    aLabelsTextPrp->SetFontFamilyToTimes();
  aLabelsTextPrp->SetBold( myLabelsBoldCheck->isChecked() );
  aLabelsTextPrp->SetItalic( myLabelsItalicCheck->isChecked() );
  aLabelsTextPrp->SetShadow( myLabelsShadowCheck->isChecked() );
  myScalarBarActor->SetLabelTextProperty( aLabelsTextPrp );

  myScalarBarActor->SetNumberOfLabels( myLabelsSpin->value() );

  if ( myHorizRadioBtn->isChecked() )
    myScalarBarActor->SetOrientationToHorizontal();
  else
    myScalarBarActor->SetOrientationToVertical();

  myScalarBarActor->SetPosition( myXSpin->value(), myYSpin->value() );
  myScalarBarActor->SetWidth( myWidthSpin->value() );
  myScalarBarActor->SetHeight( myHeightSpin->value() );

  // Distribution
  bool distributionTypeChanged = false, colorChanged=false;
  myScalarBarActor->SetDistributionVisibility((int)myDistributionGrp->isChecked());
  if( myDistributionGrp->isChecked() ) {
    int ColoringType = myDMultiColor->isChecked() ? SMESH_MULTICOLOR_TYPE : SMESH_MONOCOLOR_TYPE;
    distributionTypeChanged = (ColoringType != myScalarBarActor->GetDistributionColoringType());
    if (distributionTypeChanged)
      myScalarBarActor->SetDistributionColoringType(ColoringType);

    if( !myDMultiColor->isChecked() ) {
      QColor aTColor = myMonoColorBtn->color();
      double rgb[3], oldRgb[3];;
      rgb [0] = aTColor.red()/255.;
      rgb [1] = aTColor.green()/255.;
      rgb [2] = aTColor.blue()/255.;
      myScalarBarActor->GetDistributionColor(oldRgb);
      colorChanged = (rgb[0] != oldRgb[0] || rgb[1] != oldRgb[1] || rgb[2] != oldRgb[2]);
      if(colorChanged)
        myScalarBarActor->SetDistributionColor(rgb);
    }
  }

  double aMin = myMinEdit->text().toDouble();
  double aMax = myMaxEdit->text().toDouble();
  vtkLookupTable* myLookupTable =
    static_cast<vtkLookupTable*>(myScalarBarActor->GetLookupTable());
  double oldMinMax[2] = { myLookupTable->GetRange()[0], myLookupTable->GetRange()[1] };
  bool rangeChanges = ( fabs( oldMinMax[0] - aMin ) + fabs( oldMinMax[1] - aMax ) >
                        0.001 * ( aMax-aMin + oldMinMax[1]-oldMinMax[0] ));

  bool nbColorsChanged = (myColorsSpin->value() != myScalarBarActor->GetMaximumNumberOfColors());
  if(nbColorsChanged)
    myScalarBarActor->SetMaximumNumberOfColors(myColorsSpin->value());

  myLookupTable->SetRange( aMin, aMax );
  myLookupTable->SetNumberOfTableValues(myColorsSpin->value());

  bool scaleChanged = (myLogarithmicCheck->isChecked() != (myLookupTable->GetScale() == VTK_SCALE_LOG10));
  if (scaleChanged)
    myLookupTable->SetScale(myLogarithmicCheck->isChecked() ? VTK_SCALE_LOG10 : VTK_SCALE_LINEAR);

  myLookupTable->Build();

  if (nbColorsChanged || rangeChanges || scaleChanged)
    myActor->UpdateDistribution();

#ifndef DISABLE_PLOT2DVIEWER
  if( myActor->GetPlot2Histogram() &&
      (nbColorsChanged ||
       rangeChanges ||
       distributionTypeChanged ||
       colorChanged ))
    SMESH::ProcessIn2DViewers(myActor);
#endif

  SMESH::RepaintCurrentView();
  return true;
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::reject
 *
 *  Cancel button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::reject()
{
  myDlg = 0;
  QDialog::reject();
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onHelp
 *
 *  Help button slot
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                 platform)).
                             arg(myHelpFileName));
  }
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
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  if (aList.Extent() == 1) {
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    if( anIO->hasEntry() ) {
      SMESH_Actor* anActor = SMESH::FindActorByEntry(anIO->getEntry());
      if ( anActor && anActor->GetScalarBarActor() && anActor->GetControlMode() != SMESH_Actor::eNone ) {
        myActor = anActor;
        SMESH_ScalarBarActor* myScalarBarActor = myActor->GetScalarBarActor();

        if ( myScalarBarActor->GetLookupTable() ) {
          vtkLookupTable* aLookupTable = static_cast<vtkLookupTable*>(myScalarBarActor->GetLookupTable());

          double *range = aLookupTable->GetRange();
          myMinEdit->setText( QString::number( range[0],'g',12 ) );
          myMaxEdit->setText( QString::number( range[1],'g',12 ) );
          myLogarithmicCheck->setChecked(aLookupTable->GetScale() == VTK_SCALE_LOG10);
          //myLogarithmicCheck->setEnabled(range[0] > 1e-07 && range[1] > 1e-07);
          myLogarithmicCheck->setEnabled(range[0] != range[1]);
        }

        vtkTextProperty* aTitleTextPrp = myScalarBarActor->GetTitleTextProperty();
        double aTColor[3];
        aTitleTextPrp->GetColor( aTColor );
        myTitleColorBtn->setColor( QColor( (int)( aTColor[0]*255 ), (int)( aTColor[1]*255 ), (int)( aTColor[2]*255 ) ) );
        myTitleFontCombo->setCurrentIndex( aTitleTextPrp->GetFontFamily() );
        myTitleBoldCheck->setChecked( aTitleTextPrp->GetBold() );
        myTitleItalicCheck->setChecked( aTitleTextPrp->GetItalic() );
        myTitleShadowCheck->setChecked( aTitleTextPrp->GetShadow() );

        vtkTextProperty* aLabelsTextPrp = myScalarBarActor->GetLabelTextProperty();
        double aLColor[3];
        aLabelsTextPrp->GetColor( aLColor );
        myLabelsColorBtn->setColor( QColor( (int)( aLColor[0]*255 ), (int)( aLColor[1]*255 ), (int)( aLColor[2]*255 ) ) );
        myLabelsFontCombo->setCurrentIndex( aLabelsTextPrp->GetFontFamily() );
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

        int coloringType = myScalarBarActor->GetDistributionColoringType();
        myScalarBarActor->GetDistributionColor( aTColor );
        myMonoColorBtn->setColor( QColor( (int)( aTColor[0]*255 ), (int)( aTColor[1]*255 ), (int)( aTColor[2]*255 ) ) );
        if ( coloringType == SMESH_MONOCOLOR_TYPE ) {
          myDMonoColor->setChecked(true);
          onDistributionChanged(myDistribColorGrp->id(myDMonoColor));
        } else {
          myDMultiColor->setChecked(true);
          onDistributionChanged(myDistribColorGrp->id(myDMultiColor));
        }
        myDistributionGrp->setChecked((bool)myScalarBarActor->GetDistributionVisibility());
        onDistributionActivated(myScalarBarActor->GetDistributionVisibility());

        myRangeGrp->setEnabled( true );
        myFontGrp->setEnabled( true );
        myLabColorGrp->setEnabled( true );
        myOrientationGrp->setEnabled( true );
        myOriginDimGrp->setEnabled( true );
        myOkBtn->setEnabled( true );
        myApplyBtn->setEnabled( true );
        myDistributionGrp->setEnabled( true );
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
  myDistributionGrp->setEnabled( false );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onMinMaxChanged
 *
 *  Called when Scalar Range values are changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onMinMaxChanged()
{
  double aMin = myMinEdit->text().toDouble();
  double aMax = myMaxEdit->text().toDouble();
  bool isLogarithmicEnabled = (aMin > 1e-07 && aMax > 1e-07);
  myLogarithmicCheck->setChecked(isLogarithmicEnabled);
  myLogarithmicCheck->setEnabled(isLogarithmicEnabled);
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
  myWidthSpin->setMaximum( 1.0 - myXSpin->value() );
  myHeightSpin->setMaximum( 1.0 - myYSpin->value() );
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
  myWidthSpin->setMaximum( 1.0 );
  myWidthSpin->setValue( w );
  myHeightSpin->setMaximum( 1.0 );
  myHeightSpin->setValue( h );
  blockSignals( false );
  onXYChanged();
}


//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onDistributionChanged
 *
 *  Called when coloring type of the distribution is changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onDistributionChanged( int id ) {

  bool isActive = myDistribColorGrp->id(myDMonoColor) == id;

  myMonoColorBtn->setEnabled(isActive);
  myDistributionColorLbl->setEnabled(isActive);
}
//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::onDistributionActivated
 *
 *  Called when distribution group check box is changed
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::onDistributionActivated(bool on) {
  if(on) {
    if(myDMonoColor->isChecked())
      onDistributionChanged(myDistribColorGrp->id(myDMonoColor)  );
    else if(myDMultiColor->isChecked())
      onDistributionChanged(myDistribColorGrp->id(myDMultiColor) );
  }
  else {
    myMonoColorBtn->setEnabled(false);
    myDistributionColorLbl->setEnabled(false);
  }
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
  initScalarBarFromResources();

  int aOrientation = myVertRadioBtn->isChecked();
  if ( aOrientation == myIniOrientation )
    setOriginAndSize( myIniX, myIniY, myIniW, myIniH );
  else
    setOriginAndSize( aOrientation ? DEF_VER_X : DEF_HOR_X,
                      aOrientation ? DEF_VER_Y : DEF_HOR_Y,
                      aOrientation ? DEF_VER_W : DEF_HOR_W,
                      aOrientation ? DEF_VER_H : DEF_HOR_H );
}

//=================================================================================================
/*!
 *  SMESHGUI_Preferences_ScalarBarDlg::initScalarBarFromResources()
 *
 *  Rereading vertical and horizontal default positions from resources.
 */
//=================================================================================================
void SMESHGUI_Preferences_ScalarBarDlg::initScalarBarFromResources()
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QString name;
  if (mgr){
    // initialize from resoources

    // horizontal
    name = QString("scalar_bar_horizontal_%1");
    if (mgr->hasValue("SMESH", name.arg( "x" )))
      DEF_HOR_X = mgr->doubleValue("SMESH", name.arg( "x" ));
    if (mgr->hasValue("SMESH", name.arg( "y" )))
      DEF_HOR_Y = mgr->doubleValue("SMESH", name.arg( "y" ));
    if (mgr->hasValue("SMESH", name.arg( "width" )))
      DEF_HOR_W = mgr->doubleValue("SMESH", name.arg( "width" ));
    if (mgr->hasValue("SMESH", name.arg( "height" )))
      DEF_HOR_H = mgr->doubleValue("SMESH", name.arg( "height" ));

    // vertical
    name = QString("scalar_bar_vertical_%1");
    if (mgr->hasValue("SMESH", name.arg( "x" )))
      DEF_VER_X = mgr->doubleValue("SMESH", name.arg( "x" ));
    if (mgr->hasValue("SMESH", name.arg( "y" )))
      DEF_VER_Y = mgr->doubleValue("SMESH", name.arg( "y" ));
    if (mgr->hasValue("SMESH", name.arg( "width" )))
      DEF_VER_W = mgr->doubleValue("SMESH", name.arg( "width" ));
    if (mgr->hasValue("SMESH", name.arg( "height" )))
      DEF_VER_H = mgr->doubleValue("SMESH", name.arg( "height" ));
  }
}
