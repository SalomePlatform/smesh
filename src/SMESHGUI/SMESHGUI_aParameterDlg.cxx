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

#include "QAD_Tools.h"
#include "QAD_Desktop.h"
#include "QAD_Application.h"

#include "SMESHGUI_aParameterDlg.h"
#include "SMESHGUI_aParameter.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvalidator.h>

using namespace std;

//====================================================================================== 
// function : SMESHGUI_aParameterDlg()
//
//  The dialog will by default be modal, unless you set 'modal' to
//  false when constructing dialog
// 
//====================================================================================== 
SMESHGUI_aParameterDlg::SMESHGUI_aParameterDlg
                                        (std::list<SMESHGUI_aParameterPtr> params,
                                         QWidget*                          parent,
                                         QString                           title,
                                         bool                              modal ) 
: QDialog( parent, "MyParameterDialog", modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
  myParamList( params )
{
  /* creating widgets */
  init();
  /* title */
  setCaption( title );

  /* Move widget on the botton right corner of main widget */
  QAD_Tools::centerWidget( this, parent );
}

//====================================================================================== 
// function : SMESHGUI_aParameterDlg::init()
// purpose  : creates dialog's layout
//====================================================================================== 
void SMESHGUI_aParameterDlg::init()
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
  /* Spin boxes with labels */
  list<SMESHGUI_aParameterPtr>::iterator paramIt = myParamList.begin();
  for ( int row = 0; paramIt != myParamList.end(); paramIt++ , row++ )
  {
    SMESHGUI_aParameterPtr param = (*paramIt);
    QLabel * label = new QLabel( GroupC1, "TextLabel" );
    GroupC1Layout->addWidget( label, row, 0 );
    label->setText( param->Label() );
    QWidget* aSpinWidget;
    switch ( param->GetType() ) {
    case SMESHGUI_aParameter::DOUBLE: {
      SMESHGUI_SpinBox* spin = new SMESHGUI_SpinBox( GroupC1 );
      aSpinWidget = spin;
      spin->setPrecision( 12 );
      break;
    }
    case SMESHGUI_aParameter::INT: {
      QSpinBox* spin = new QSpinBox( GroupC1 );
      aSpinWidget = spin;
      break;
    }
    default:;
    }
    GroupC1Layout->addWidget( aSpinWidget, row, 1 );
    aSpinWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    aSpinWidget->setMinimumSize( 150, 0 );
    param->InitializeWidget( aSpinWidget );
    mySpinList.push_back( aSpinWidget );
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
  connect( myButtonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
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

//=======================================================================
//function : ClickOnOk
//purpose  : 
//=======================================================================

void SMESHGUI_aParameterDlg::ClickOnOk()
{
  if ( !mySMESHGUI->ActiveStudyLocked() ) {
    list<SMESHGUI_aParameterPtr>::iterator paramIt = myParamList.begin();
    list<QWidget*>::iterator              widgetIt = mySpinList.begin();
    for ( ;
         paramIt != myParamList.end() && widgetIt != mySpinList.end();
         paramIt++ , widgetIt++ )
      (*paramIt)->TakeValue( *widgetIt );

    accept();
  }
}


//=======================================================================
// function : Parameters()
// purpose  : return a list of parameters from a dialog box
//=======================================================================
bool SMESHGUI_aParameterDlg::Parameters( list<SMESHGUI_aParameterPtr> params, const char *aTitle)
{
  if ( !params.empty() ) {
    SMESHGUI_aParameterDlg *Dialog =
      new SMESHGUI_aParameterDlg(params,
                                 QAD_Application::getDesktop(),
                                 aTitle,
                                 TRUE);
    return (Dialog->exec() == QDialog::Accepted);
  }
  return false;
}
