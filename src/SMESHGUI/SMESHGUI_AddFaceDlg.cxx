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
//  File   : SMESHGUI_AddFaceDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_AddFaceDlg.h"

#include "SMESHGUI.h"
#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qcheckbox.h>


//=================================================================================
// class    : SMESHGUI_AddFaceDlg()
// purpose  : 
//=================================================================================
SMESHGUI_AddFaceDlg::SMESHGUI_AddFaceDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
					  int nbNodes, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  myNodes = nbNodes;
  QPixmap image0;
  if ( myNodes == 3 )
    image0 = QPixmap(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_TRIANGLE")));
  else if ( myNodes == 4 )
    image0 = QPixmap(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_QUADRANGLE")));

  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_AddFaceDlg" );
  resize( 303, 185 ); 
  if ( myNodes == 3 )
    setCaption( tr( "SMESH_ADD_TRIANGLE_TITLE"  ) );
  else if ( myNodes == 4 )
    setCaption( tr( "SMESH_ADD_QUADRANGLE_TITLE"  ) );

  setSizeGripEnabled( TRUE );
  SMESHGUI_AddFaceDlgLayout = new QGridLayout( this ); 
  SMESHGUI_AddFaceDlgLayout->setSpacing( 6 );
  SMESHGUI_AddFaceDlgLayout->setMargin( 11 );

    /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  if ( myNodes == 3 )
    GroupConstructors->setTitle( tr( "SMESH_TRIANGLE"  ) );
  else if ( myNodes == 4 )
    GroupConstructors->setTitle( tr( "SMESH_QUADRANGLE"  ) );
  
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  Constructor1 = new QRadioButton( GroupConstructors, "Constructor1" );
  Constructor1->setText( tr( ""  ) );
  Constructor1->setPixmap( image0 );
  Constructor1->setChecked( TRUE );
  Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
  Constructor1->setMinimumSize( QSize( 50, 0 ) );
  GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupConstructorsLayout->addItem( spacer, 0, 1 );
  SMESHGUI_AddFaceDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
    /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
  GroupButtons->setTitle( tr( ""  ) );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  buttonApply = new QPushButton( GroupButtons, "buttonApply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer_9, 0, 2 );
  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  SMESHGUI_AddFaceDlgLayout->addWidget( GroupButtons, 2, 0 );

    /***************************************************************/
  GroupC1 = new QGroupBox( this, "GroupC1" );
  if ( myNodes == 3 )
    GroupC1->setTitle( tr( "SMESH_ADD_TRIANGLE"  ) );
  else if ( myNodes == 4 )
    GroupC1->setTitle( tr( "SMESH_ADD_QUADRANGLE"  ) );

  GroupC1->setMinimumSize( QSize( 0, 0 ) );
  GroupC1->setFrameShape( QGroupBox::Box );
  GroupC1->setFrameShadow( QGroupBox::Sunken );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );
  TextLabelC1A1 = new QLabel( GroupC1, "TextLabelC1A1" );
  TextLabelC1A1->setText( tr( "SMESH_ID_NODES"  ) );
  TextLabelC1A1->setMinimumSize( QSize( 50, 0 ) );
  TextLabelC1A1->setFrameShape( QLabel::NoFrame );
  TextLabelC1A1->setFrameShadow( QLabel::Plain );
  GroupC1Layout->addWidget( TextLabelC1A1, 0, 0 );
  SelectButtonC1A1 = new QPushButton( GroupC1, "SelectButtonC1A1" );
  SelectButtonC1A1->setText( tr( ""  ) );
  SelectButtonC1A1->setPixmap( image1 );
  SelectButtonC1A1->setToggleButton( FALSE );
  GroupC1Layout->addWidget( SelectButtonC1A1, 0, 1 );
  LineEditC1A1 = new QLineEdit( GroupC1, "LineEditC1A1" );
  GroupC1Layout->addWidget( LineEditC1A1, 0, 2 );

  Reverse = new QCheckBox( GroupC1, "Reverse" );
  Reverse->setText( tr( "SMESH_REVERSE"  ) );
  GroupC1Layout->addWidget( Reverse, 1, 0 );

  SMESHGUI_AddFaceDlgLayout->addWidget( GroupC1, 1, 0 );

  Init(Sel) ; /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_AddFaceDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddFaceDlg::~SMESHGUI_AddFaceDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::Init( SALOME_Selection* Sel )
{

  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;  
  this->myOkNodes = false ;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );

  connect( SelectButtonC1A1, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;

  connect( Reverse, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );

  reverse = false;

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

  SelectionIntoArgument();

  return ;
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_AddFaceDlg::ConstructorsClicked(int constructorId)
{
  return ;
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      { 
	if(myOkNodes) {	  
	  mySMESHGUI->EraseSimulationActors();
	  mySMESHGUI->AddFace( myMesh, myMapIndex, reverse ) ;
	  mySMESHGUI->ViewNodes();
	  mySelection->ClearIObjects();
	}
	break ;
      }
    }
  return ;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;
  return ;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::ClickOnCancel()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  mySMESHGUI->EraseSimulationActors();
  reject() ;
  return ;
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_AddFaceDlg::SelectionIntoArgument()
{
  myEditCurrentArgument->setText("") ;
  myOkNodes = false;
  QString aString = "";

  int nbSel = mySelection->IObjectCount();
  if(nbSel != 1)
    return;

  int nbNodes = mySMESHGUI->GetNameOfSelectedNodes(mySelection, aString) ;
  if(nbNodes < 1)
    return ;
  
  if ( mySelection->SelectionMode() != NodeSelection ) {
    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
			    tr ("SMESH_WRN_SELECTIONMODE_NODES"), tr ("SMESH_BUT_OK") );
    return;
  }

  myEditCurrentArgument->setText(aString) ;
  Standard_Boolean res;
  myMesh = mySMESHGUI->ConvertIOinMesh( mySelection->firstIObject(), res );
  if (!res)
    return ;

  mySelection->GetIndex( mySelection->firstIObject(), myMapIndex );

  if ( myNodes == 3 && nbNodes == 3 ) {
    mySMESHGUI->DisplaySimulationTriangle(myMesh, myMapIndex, reverse);
    myOkNodes = true ;
  } else if ( myNodes == 4 && nbNodes == 4 ) {
    mySMESHGUI->DisplaySimulationQuadrangle(myMesh, myMapIndex, reverse);
    myOkNodes = true ;
  } else {
    //    mySMESHGUI->EraseSimulationActors();
    myOkNodes = false ;
  }
  return ;
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {	
	if(send == SelectButtonC1A1) {
	  LineEditC1A1->setFocus() ;
	  myEditCurrentArgument = LineEditC1A1;
	}
	SelectionIntoArgument() ;
	break;
      }
    }
  return ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {  
    GroupConstructors->setEnabled(false) ;
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySMESHGUI->EraseSimulationActors() ;
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
  return ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddFaceDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
  return ;
}


void SMESHGUI_AddFaceDlg::CheckBox( int state )
{
  if( Reverse->isOn() ) reverse = true;
  else reverse = false;

  if ( myOkNodes == false )
    return;
  
  if ( state >= 0 ) {
    mySMESHGUI->EraseSimulationActors();
    if ( myNodes == 3 )
      mySMESHGUI->DisplaySimulationTriangle( myMesh, myMapIndex, reverse);
    else if ( myNodes == 4 ) {
      mySMESHGUI->DisplaySimulationQuadrangle( myMesh, myMapIndex, reverse);
    }
  }
}
