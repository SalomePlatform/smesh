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
//  File   : SMESHGUI_MoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MoveNodesDlg.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESH_Actor.h"
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

// Open CASCADE Include
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

using namespace std;

//=================================================================================
// class    : SMESHGUI_MoveNodesDlg()
// purpose  : 
//=================================================================================
SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
					      bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_MOVE_NODE")));
    QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

    if ( !name )
	setName( "SMESHGUI_MoveNodesDlg" );
    resize( 303, 185 ); 
    setCaption( tr( "SMESH_MOVE_NODES_TITLE"  ) );
    setSizeGripEnabled( TRUE );
    SMESHGUI_MoveNodesDlgLayout = new QGridLayout( this ); 
    SMESHGUI_MoveNodesDlgLayout->setSpacing( 6 );
    SMESHGUI_MoveNodesDlgLayout->setMargin( 11 );

    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_NODES"  ) );
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
    SMESHGUI_MoveNodesDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
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
    SMESHGUI_MoveNodesDlgLayout->addWidget( GroupButtons, 3, 0 );

    /***************************************************************/
    GroupC1 = new QGroupBox( this, "GroupC1" );
    GroupC1->setTitle( tr( "SMESH_MOVE"  ) );
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

    SMESHGUI_MoveNodesDlgLayout->addWidget( GroupC1, 1, 0 );

    /***************************************************************/
    GroupCoordinates = new QGroupBox( this, "GroupCoordinates" );
    GroupCoordinates->setTitle( tr( "SMESH_COORDINATES"  ) );
    GroupCoordinates->setColumnLayout(0, Qt::Vertical );
    GroupCoordinates->layout()->setSpacing( 0 );
    GroupCoordinates->layout()->setMargin( 0 );
    GroupCoordinatesLayout = new QGridLayout( GroupCoordinates->layout() );
    GroupCoordinatesLayout->setAlignment( Qt::AlignTop );
    GroupCoordinatesLayout->setSpacing( 6 );
    GroupCoordinatesLayout->setMargin( 11 );
    TextLabel_X = new QLabel( GroupCoordinates, "TextLabel_X" );
    TextLabel_X->setText( tr( "SMESH_X"  ) );
    GroupCoordinatesLayout->addWidget( TextLabel_X, 0, 0 );
    TextLabel_Y = new QLabel( GroupCoordinates, "TextLabel_Y" );
    TextLabel_Y->setText( tr( "SMESH_Y"  ) );
    GroupCoordinatesLayout->addWidget( TextLabel_Y, 0, 2 );
  
    SpinBox_X = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_X" ) ;
    GroupCoordinatesLayout->addWidget( SpinBox_X, 0, 1 );
    
    SpinBox_Y = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_Y" ) ;
    GroupCoordinatesLayout->addWidget( SpinBox_Y, 0, 3 );
    
    SpinBox_Z = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_Z" ) ;
    GroupCoordinatesLayout->addWidget( SpinBox_Z, 0, 5 );


    TextLabel_Z = new QLabel( GroupCoordinates, "TextLabel_Z" );
    TextLabel_Z->setText( tr( "SMESH_Z"  ) );
    GroupCoordinatesLayout->addWidget( TextLabel_Z, 0, 4 );

    SMESHGUI_MoveNodesDlgLayout->addWidget( GroupCoordinates, 2, 0 );

    Init(Sel) ; /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_MoveNodesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::Init( SALOME_Selection* Sel )
{

  /* Get setting of step value from file configuration */
  double step ;
  // QString St = QAD_CONFIG->getSetting( "xxxxxxxxxxxxx" ) ;  TODO
  // step = St.toDouble() ;                                    TODO
  step = 25.0 ;
  
  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;  
  this->myOkNodes = false ;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* min, max, step and decimals for spin boxes */
  SpinBox_X->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;
  SpinBox_Y->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;
  SpinBox_Z->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;  
  SpinBox_X->SetValue( 0.0 ) ;
  SpinBox_Y->SetValue( 0.0 ) ;
  SpinBox_Z->SetValue( 0.0 ) ;

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

  connect( SpinBox_X, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Y, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Z, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;

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
void SMESHGUI_MoveNodesDlg::ConstructorsClicked(int constructorId)
{
  return ;
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      { 
	if(myOkNodes) {	  
	  mySMESHGUI->EraseSimulationActors();
//  	  mySMESHGUI->MoveNode( myMesh, myIdnode, LineEdit_X->text().toFloat(), LineEdit_Y->text().toFloat(), LineEdit_Z->text().toFloat() ) ;
//  	  mySelection->ClearIObjects();
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
void SMESHGUI_MoveNodesDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;

  return ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::ClickOnCancel()
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
void SMESHGUI_MoveNodesDlg::SelectionIntoArgument()
{

  disconnect( SpinBox_X, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  disconnect( SpinBox_Y, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  disconnect( SpinBox_Z, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;

  mySMESHGUI->EraseSimulationActors();

  myEditCurrentArgument->setText("") ;
  myOkNodes = false;
  QString aString = "";

  int nbSel = mySelection->IObjectCount();
  if(nbSel != 1)
    return ;

  int nbNodes = mySMESHGUI->GetNameOfSelectedNodes(mySelection, aString) ;
  if(nbNodes != 1) {
    SpinBox_X->SetValue(0.0) ;
    SpinBox_Y->SetValue(0.0) ;
    SpinBox_Z->SetValue(0.0) ;
    return ;
  }
  
  if ( mySelection->SelectionMode() != NodeSelection ){
    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
			    tr ("SMESH_WRN_SELECTIONMODE_NODES"), tr ("SMESH_BUT_OK") );
    return;
  }

  myEditCurrentArgument->setText(aString) ;
  Standard_Boolean res;
  myMesh = mySMESHGUI->ConvertIOinMesh( mySelection->firstIObject(), res );
  if (!res)
    return ;

  SMESH_Actor* ac = mySMESHGUI->FindActorByEntry( mySelection->firstIObject()->getEntry(), res, false );
  if ( !res )
    return ;

  mySelection->GetIndex( mySelection->firstIObject(), myMapIndex);

  TColStd_MapIteratorOfMapOfInteger ite( myMapIndex );
  if ( ite.More() ) {
    myIdnode = ite.Key();
    mySimulationActor = mySMESHGUI->SimulationMoveNode( ac, myIdnode );
  } else
    return ;

  MESSAGE ( " myIdnode " << myIdnode );

  float *pt  = ac->GetMapper()->GetInput()->GetPoint(myIdnode);
  MESSAGE ( " pt " << pt[0] << ";" << pt[1] << ";" << pt[2] )

  SpinBox_X->SetValue( (double)pt[0] ) ;
  SpinBox_Y->SetValue( (double)pt[1] ) ;
  SpinBox_Z->SetValue( (double)pt[2] ) ;

  connect( SpinBox_X, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Y, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Z, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;

  myOkNodes = true ;
  return ;
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::SetEditCurrentArgument()
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
void SMESHGUI_MoveNodesDlg::DeactivateActiveDialog()
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
void SMESHGUI_MoveNodesDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::enterEvent(QEvent* e)
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
void SMESHGUI_MoveNodesDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
  return ;
}


//=================================================================================
// function : ValueChangedInSpinBox()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::ValueChangedInSpinBox( double newValue )
{
  double vx = SpinBox_X->GetValue() ;
  double vy = SpinBox_Y->GetValue() ;
  double vz = SpinBox_Z->GetValue() ;
  
  mySMESHGUI->DisplaySimulationMoveNode( mySimulationActor, myIdnode, vx, vy , vz );
}
