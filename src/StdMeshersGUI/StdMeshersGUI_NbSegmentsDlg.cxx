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
//  File   : StdMeshersGUI_NbSegmentsDlg.cxx
//           Moved here from SMESHGUI_NbSegmentsDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshersGUI_NbSegmentsDlg.h"
#include "SMESHGUI.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "utilities.h"

#include "SALOMEGUI_QtCatchCorbaException.hxx"
#include "QAD_MessageBox.h"
#include "QAD_WaitCursor.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qspinbox.h>

//=================================================================================
// class    : StdMeshersGUI_NbSegmentsDlg()
// purpose  : Constructs a StdMeshersGUI_NbSegmentsDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
StdMeshersGUI_NbSegmentsDlg::StdMeshersGUI_NbSegmentsDlg (const QString& hypType,
							  QWidget* parent,
							  const char* name,
							  bool modal,
							  WFlags fl)
     : QDialog (parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose),
       myHypType(hypType)
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_NB_SEGMENTS")));
    
    if ( !name )
      setName( "StdMeshersGUI_NbSegmentsDlg" );
    setCaption( tr( "SMESH_NB_SEGMENTS_TITLE"  ) );
    setSizeGripEnabled( TRUE );
    QGridLayout* StdMeshersGUI_NbSegmentsDlgLayout = new QGridLayout( this ); 
    StdMeshersGUI_NbSegmentsDlgLayout->setSpacing( 6 );
    StdMeshersGUI_NbSegmentsDlgLayout->setMargin( 11 );

    /***************************************************************/
    iconLabel = new QLabel( this );
    iconLabel->setPixmap( image0 );
    iconLabel->setScaledContents( false );
    iconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    typeLabel = new QLabel( this );
    typeLabel->setText( tr( "SMESH_NB_SEGMENTS_HYPOTHESIS"  ) );
    StdMeshersGUI_NbSegmentsDlgLayout->addWidget( iconLabel, 0, 0 );
    StdMeshersGUI_NbSegmentsDlgLayout->addWidget( typeLabel, 0, 1 );
    
    /***************************************************************/
    GroupC1 = new QGroupBox( this, "GroupC1" );
    GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
    GroupC1->setColumnLayout(0, Qt::Vertical );
    GroupC1->layout()->setSpacing( 0 );
    GroupC1->layout()->setMargin( 0 );
    QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
    GroupC1Layout->setAlignment( Qt::AlignTop );
    GroupC1Layout->setSpacing( 6 );
    GroupC1Layout->setMargin( 11 );

    LineEdit_NameHypothesis = new QLineEdit( GroupC1, "LineEdit_NameHypothesis" );
    GroupC1Layout->addWidget( LineEdit_NameHypothesis, 0, 1 );

    TextLabel_NameHypothesis = new QLabel( GroupC1, "TextLabel_NameHypothesis" );
    TextLabel_NameHypothesis->setText( tr( "SMESH_NAME"  ) );
    GroupC1Layout->addWidget( TextLabel_NameHypothesis, 0, 0 );

    TextLabel_NbSeg = new QLabel(GroupC1 , "TextLabel_NbSeg" );
    TextLabel_NbSeg->setText( tr( "SMESH_SEGMENTS"  ) );
    GroupC1Layout->addWidget( TextLabel_NbSeg, 1, 0 );

    SpinBox_NbSeg = new QSpinBox( GroupC1, "SpinBox_NbSeg" );
    GroupC1Layout->addWidget( SpinBox_NbSeg, 1, 1 );

    StdMeshersGUI_NbSegmentsDlgLayout->addMultiCellWidget(GroupC1 , 1, 1, 0, 1 );

    /***************************************************************/
    GroupButtons = new QGroupBox( this, "GroupButtons" );
    GroupButtons->setColumnLayout(0, Qt::Vertical );
    GroupButtons->layout()->setSpacing( 0 );
    GroupButtons->layout()->setMargin( 0 );
    QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
    GroupButtonsLayout->setAlignment( Qt::AlignTop );
    GroupButtonsLayout->setSpacing( 6 );
    GroupButtonsLayout->setMargin( 11 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
    buttonApply = new QPushButton( GroupButtons, "buttonApply" );
    buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
    buttonApply->setAutoDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupButtonsLayout->addItem( spacer, 0, 2 );
    buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
    buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
    buttonCancel->setAutoDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
    StdMeshersGUI_NbSegmentsDlgLayout->addMultiCellWidget( GroupButtons, 2, 2, 0, 1 );

    /***************************************************************/
    Init() ;
}


//=================================================================================
// function : ~StdMeshersGUI_NbSegmentsDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
StdMeshersGUI_NbSegmentsDlg::~StdMeshersGUI_NbSegmentsDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::Init()
{
  SpinBox_NbSeg->setMinValue( 1 );
  SpinBox_NbSeg->setMaxValue( 9999 );
  SpinBox_NbSeg->setValue(3) ; /* myNbSeg */

  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;

  char* sHypType = (char*)myHypType.latin1();
  HypothesisData* aHypData = mySMESHGUI->GetHypothesisData(sHypType);
  LineEdit_NameHypothesis->setText( aHypData ? aHypData->Label : "" );

  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  connect( buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()) );
  connect( buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()) ) ;
  connect( buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()) );

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;  
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ; 
  this->show() ; /* displays Dialog */
}


//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::ClickOnOk()
{
  if ( this->ClickOnApply() )
    this->ClickOnCancel() ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool StdMeshersGUI_NbSegmentsDlg::ClickOnApply()
{
  QString myHypName = LineEdit_NameHypothesis->text().stripWhiteSpace();
  if ( myHypName.isEmpty() ) {
    QAD_MessageBox::warn1( this, tr( "SMESH_WRN_WARNING" ), tr( "SMESH_WRN_EMPTY_NAME" ), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  long myNbSeg = SpinBox_NbSeg->value();

  QAD_WaitCursor wc;
  try {
    SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow
      (mySMESHGUI->CreateHypothesis(myHypType,
				    myHypName,
				    false)); // isAlgorithm
      StdMeshers::StdMeshers_NumberOfSegments_var NbS =
	StdMeshers::StdMeshers_NumberOfSegments::_narrow(Hyp);
      if (!NbS->_is_nil())
	NbS->SetNumberOfSegments(myNbSeg);
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    wc.stop();
    QtCatchCorbaException(S_ex);
    return false;
  }
  return true;
}


//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::ClickOnCancel()
{
  close();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::DeactivateActiveDialog()
{
  iconLabel->setEnabled(false) ;
  typeLabel->setEnabled(false) ;
  GroupC1->setEnabled(false) ;
  GroupButtons->setEnabled(false) ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  iconLabel->setEnabled(true) ;
  typeLabel->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::enterEvent(QEvent* e)
{
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void StdMeshersGUI_NbSegmentsDlg::closeEvent( QCloseEvent* e )
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent( e );
}
