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
//  File   : SMESHGUI_aParameterDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_aParameterDlg.h"
#include "SMESHGUI.h"
#include "QAD_SpinBoxDbl.h"
#include "QAD_Tools.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvalidator.h>

//====================================================================================== 
// function : SMESHGUI_aParameterDlg()
// purpose  : Constructs a SMESHGUI_aParametertDlg for double values
//
//  parent    : parent widget
//  title     : is the title for the user in dialog box
//  label     : text label for the value
//
//  bottom     : the minimal value to be entered
//  top        : the maximum value to be entered
//  decimals   : number of decimals to be entered
//
//  The dialog will by default be modal, unless you set 'modal' to
//  false when constructing dialog
// 
//====================================================================================== 
SMESHGUI_aParameterDlg::SMESHGUI_aParameterDlg( QWidget*     parent,
					        QString      title,
					        QString      label,
					        const double bottom,
					        const double top,
					        const int    decimals,
					        bool         modal ) 
: QDialog( parent, "MyParameterDialog", modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
  myIntSpin( 0 ), myDblSpin( 0 )
{
  /* creating widgets */
  init( true );
  /* title */
  setCaption( title );
  /* caption */
  myTextLabel->setText( label );
  /* range */
  myDblSpin->setRange( bottom, top );
  ((QDoubleValidator*)(myDblSpin->validator()))->setRange( bottom, top, decimals );

  /* Move widget on the botton right corner of main widget */
  QAD_Tools::centerWidget( this, parent );
}

//====================================================================================== 
// function : SMESHGUI_aParameterDlg()
// purpose  : Constructs a SMESHGUI_aParametertDlg for int values
//
//  parent    : parent widget
//  title     : is the title for the user in dialog box
//  label     : text label for the value
//
//  bottom     : the minimal value to be entered
//  top        : the maximum value to be entered
//
//  The dialog will by default be modal, unless you set 'modal' to
//  false when constructing dialog
// 
//====================================================================================== 
SMESHGUI_aParameterDlg::SMESHGUI_aParameterDlg( QWidget*     parent,
					        QString      title,
					        QString      label,
					        const int    bottom,
					        const int    top,
					        bool         modal ) 
: QDialog( parent, "MyParameterDialog", modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
  myIntSpin( 0 ), myDblSpin( 0 )
{
  /* creating widgets */
  init( false );
  /* title */
  setCaption( title );
  /* caption */
  myTextLabel->setText( label );
  /* range */
  myIntSpin->setRange( bottom, top );
  ((QIntValidator*)(myIntSpin->validator()))->setRange( bottom, top );

  /* Move widget on the botton right corner of main widget */
  QAD_Tools::centerWidget( this, parent );
}

//====================================================================================== 
// function : SMESHGUI_aParameterDlg::init()
// purpose  : creates dialog's layout
//====================================================================================== 
void SMESHGUI_aParameterDlg::init( bool isDouble )
{
  setSizeGripEnabled( TRUE );
  
  QGridLayout* topLayout = new QGridLayout( this );
  topLayout->setMargin( 11 ); topLayout->setSpacing( 6 );

  /***************************************************************/
  QGroupBox* GroupC1 = new QGroupBox( this, "GroupC1" );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  /* Label */
  /* aTitle1 : text prompt on left of edit line */
  myTextLabel = new QLabel( GroupC1, "TextLabel" );
  GroupC1Layout->addWidget( myTextLabel, 0, 0 );
  /* Spin box */
  if ( isDouble ) {
    myIntSpin = 0;
    myDblSpin = new QAD_SpinBoxDbl( GroupC1 );
    myDblSpin->setPrecision( 12 );
    myDblSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    myDblSpin->setMinimumSize( 150, 0 );
    GroupC1Layout->addWidget( myDblSpin, 0, 1 );
  }
  else {
    myDblSpin = 0;
    myIntSpin = new QSpinBox( GroupC1 );
    myIntSpin->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    myIntSpin->setMinimumSize( 150, 0 );
    GroupC1Layout->addWidget( myIntSpin, 0, 1 );
  }
  
  /***************************************************************/
  QGroupBox* GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  /* Ok button */
  myButtonOk = new QPushButton( GroupButtons, "buttonOk" );
  myButtonOk->setText( tr("SMESH_BUT_OK") );
  myButtonOk->setAutoDefault( TRUE );
  myButtonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( myButtonOk, 0, 0 );
  /* add spacer between buttons */
  GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 1 );
  /* Cancel button */
  myButtonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  myButtonCancel->setText( tr("SMESH_BUT_CANCEL") );
  myButtonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( myButtonCancel, 0, 2 );
  
  /***************************************************************/
  topLayout->addWidget( GroupC1,      0, 0);
  topLayout->addWidget( GroupButtons, 1, 0);

  /* signals and slots connections */
  connect( myButtonOk,     SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( myButtonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

  /* Retrieve SMESHGUI */
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
}


//====================================================================================== 
// function : ~SMESHGUI_aParameterDlg()
// purpose  : Destructor
//====================================================================================== 
SMESHGUI_aParameterDlg::~SMESHGUI_aParameterDlg() 
{
}

//====================================================================================== 
// function : SMESHGUI_aParameterDlg::setValue
// purpose  : sets double value
//====================================================================================== 
void SMESHGUI_aParameterDlg::setValue( const double val )
{
  if ( myDblSpin )
    myDblSpin->setValue( val );
}
//====================================================================================== 
// function : SMESHGUI_aParameterDlg::setValue
// purpose  : sets int value
//====================================================================================== 
void SMESHGUI_aParameterDlg::setValue( const int val )
{
  if ( myIntSpin )
    myIntSpin->setValue( val );
}
//====================================================================================== 
// function : SMESHGUI_aParameterDlg::getDblValue
// purpose  : returns double value entered by user
//====================================================================================== 
double SMESHGUI_aParameterDlg::getDblValue()
{
  if ( myDblSpin )
    return myDblSpin->value();
  return 0.0;
}

//====================================================================================== 
// function : SMESHGUI_aParameterDlg::getIntValu
// purpose  : returns int value entered by user
//====================================================================================== 
int SMESHGUI_aParameterDlg::getIntValue()
{
  if ( myIntSpin )
    return myIntSpin->value();
  return 0;
}









