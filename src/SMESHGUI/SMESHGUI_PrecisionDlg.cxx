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
//  File   : SMESHGUI_PrecisionDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_PrecisionDlg.h"
#include "QAD_Config.h"
#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qlabel.h>

#define SPACING 5                  
#define MARGIN  10
#define DEFAULT_VAL 10
#define RANGE 128

/*
  Class       : SMESHGUI_PrecisionDlg
  Description : Dialog to specify precision of mesh quality controls
*/

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::SMESHGUI_PrecisionDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_PrecisionDlg::SMESHGUI_PrecisionDlg( QWidget* theParent )
: QDialog( theParent, "SMESHGUI_PrecisionDlg", true,
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  QFrame* aMainFrame = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );

  setMinimumWidth( (int)( QFontMetrics( font() ).width( tr( "CAPTION" ) ) * 1.5 ) );

  Init();

}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::~SMESHGUI_PrecisionDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_PrecisionDlg::~SMESHGUI_PrecisionDlg()
{
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_PrecisionDlg::createButtonFrame( QWidget* theParent )
{
  QGroupBox* aGrp = new QGroupBox( 1, Qt::Vertical, theParent );
  aGrp->setFrameStyle( QFrame::NoFrame );
  aGrp->setInsideMargin( 0 );

  myOKBtn = new QPushButton( tr( "SMESH_BUT_OK"    ), aGrp );

  QLabel* aLbl = new QLabel( aGrp );
  aLbl->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myCancelBtn = new QPushButton( tr( "SMESH_BUT_CANCEL" ), aGrp );

  connect( myOKBtn, SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCancelBtn, SIGNAL( clicked() ), SLOT( onClose() ) );

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg:: createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_PrecisionDlg::createMainFrame( QWidget* theParent )
{
  QGroupBox* aGrp = new QGroupBox( 2, Qt::Horizontal, theParent );
  new QLabel( tr( "PRECISION" ), aGrp );
  mySpinBox = new QSpinBox( 0, RANGE, 1, aGrp );
  myNotUseChk = new QCheckBox( tr( "NOT_USE" ), aGrp );

  connect( myNotUseChk, SIGNAL( toggled( bool ) ), SLOT( onNotUse() ) );

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::Init
// Purpose : Initialize dialog fields
//=======================================================================
void SMESHGUI_PrecisionDlg::Init()
{
  if ( QAD_CONFIG->hasSetting( "SMESH:ControlsPrecision" ) )
  {
    QString aStr = QAD_CONFIG->getSetting( "SMESH:ControlsPrecision" );
    bool isOk = false;
    int aVal = aStr.toInt( &isOk );
    mySpinBox->setValue( isOk ? aVal : DEFAULT_VAL );
    myNotUseChk->setChecked( !isOk );
  }
  else
  {
    mySpinBox->setValue( DEFAULT_VAL );
    myNotUseChk->setChecked( true );
  }

  onNotUse();

  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::onOk
// Purpose : SLOT. Called when OK button pressed
//=======================================================================
void SMESHGUI_PrecisionDlg::onOk()
{
  if ( myNotUseChk->isChecked() )
  {
    QAD_CONFIG->removeSettings( "SMESH:ControlsPrecision" );
    SMESH::SetControlsPrecision( -1 );
  }
  else
  {
    mySpinBox->clearFocus();
    int aVal = mySpinBox->value();
    QAD_CONFIG->addSetting( "SMESH:ControlsPrecision", QString( "%1" ).arg( aVal ) );
    SMESH::SetControlsPrecision( aVal );
  }
  
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState() ;
  accept();
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::onClose
// Purpose : SLOT. Called when "Cancel" button pressed
//=======================================================================
void SMESHGUI_PrecisionDlg::onClose()
{
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  reject();
}

//=================================================================================
// function : SMESHGUI_PrecisionDlg::closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_PrecisionDlg::closeEvent( QCloseEvent* )
{
  onClose();
}

//=======================================================================
// name    : SMESHGUI_PrecisionDlg::onClose
// Purpose : SLOT. Called when state of "Do not use" check box changed
//=======================================================================
void SMESHGUI_PrecisionDlg::onNotUse()
{
  mySpinBox->setEnabled( !myNotUseChk->isChecked() );
}





















