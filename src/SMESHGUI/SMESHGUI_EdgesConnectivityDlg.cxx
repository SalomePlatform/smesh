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
//  File   : SMESHGUI_EdgesConnectivityDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_EdgesConnectivityDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"

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

// VTK Include
#include <vtkFeatureEdges.h>

//=================================================================================
// class    : SMESHGUI_EdgesConnectivityDlg()
// purpose  : 
//=================================================================================
SMESHGUI_EdgesConnectivityDlg::SMESHGUI_EdgesConnectivityDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
						  bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESHGUI",tr("ICON_DLG_CONNECTIVITY")));
    QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESHGUI",tr("ICON_SELECT")));

    if ( !name )
	setName( "SMESHGUI_EdgesConnectivityDlg" );
    resize( 303, 185 ); 
    setCaption( tr( "SMESH_EDGES_CONNECTIVITY_TITLE"  ) );
    setSizeGripEnabled( TRUE );
    SMESHGUI_EdgesConnectivityDlgLayout = new QGridLayout( this ); 
    SMESHGUI_EdgesConnectivityDlgLayout->setSpacing( 6 );
    SMESHGUI_EdgesConnectivityDlgLayout->setMargin( 11 );

    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_EDGES_CONNECTIVITY"  ) );
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
    SMESHGUI_EdgesConnectivityDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
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
    //buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    //buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    //buttonOk->setAutoDefault( TRUE );
    //buttonOk->setDefault( TRUE );
    //GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
    SMESHGUI_EdgesConnectivityDlgLayout->addWidget( GroupButtons, 2, 0 );

    /***************************************************************/
    GroupC1 = new QGroupBox( this, "GroupC1" );
    GroupC1->setTitle( tr( "SMESH_EDGES_CONNECTIVITY"  ) );
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
    TextLabelC1A1->setText( tr( "SMESH_MESH"  ) );
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

    BoundaryEdges = new QCheckBox( GroupC1, "BoundaryEdges" );
    BoundaryEdges->setText( tr( "SMESH_BOUNDARYEDGES"  ) );
    GroupC1Layout->addWidget( BoundaryEdges, 1, 0 );

    ManifoldEdges = new QCheckBox( GroupC1, "ManifoldEdges" );
    ManifoldEdges->setText( tr( "SMESH_MANIFOLDEDGES"  ) );
    GroupC1Layout->addWidget( ManifoldEdges, 2, 0 );

    NonManifoldEdges = new QCheckBox( GroupC1, "NonManifoldEdges" );
    NonManifoldEdges->setText( tr( "SMESH_NONMANIFOLDEDGES"  ) );
    GroupC1Layout->addWidget( NonManifoldEdges, 1, 2 );

    FeatureEdges = new QCheckBox( GroupC1, "FeatureEdges" );
    FeatureEdges->setText( tr( "SMESH_FEATUREEDGES"  ) );
    GroupC1Layout->addWidget( FeatureEdges, 2, 2 );

    FeatureAngle= new QLabel( GroupC1, "FeatureAngle" );
    FeatureAngle->setText( tr( "SMESH_FEATUREANGLE"  ) );
    FeatureAngle->setMinimumSize( QSize( 50, 0 ) );
    FeatureAngle->setFrameShape( QLabel::NoFrame );
    FeatureAngle->setFrameShadow( QLabel::Plain );
    GroupC1Layout->addWidget( FeatureAngle, 3, 0 );

    SpinBox_FeatureAngle = new SMESHGUI_SpinBox( GroupC1, "SpinBox_FeatureAngle" ) ;
    GroupC1Layout->addWidget( SpinBox_FeatureAngle , 3, 2 );

    SMESHGUI_EdgesConnectivityDlgLayout->addWidget( GroupC1, 1, 0 );

    Init(Sel) ; /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_EdgesConnectivityDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_EdgesConnectivityDlg::~SMESHGUI_EdgesConnectivityDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::Init( SALOME_Selection* Sel )
{

  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;  
  this->myOkActor = false ;

  /* Get setting of step value from file configuration */
  double step ;
  // QString St = QAD_CONFIG->getSetting( "xxxxxxxxxxxxx" ) ;  TODO
  // step = St.toDouble() ;                                    TODO
  step = 5.0 ;
  
 /* min, max, step and decimals for spin boxes */
  SpinBox_FeatureAngle->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;
  SpinBox_FeatureAngle->SetValue( 30.0 ) ;


  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  //connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );

  connect( BoundaryEdges, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );
  connect( ManifoldEdges, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );
  connect( NonManifoldEdges, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );
  connect( FeatureEdges, SIGNAL(stateChanged(int)), SLOT(CheckBox(int)) );

  connect( SelectButtonC1A1, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

  return ;
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::ConstructorsClicked(int constructorId)
{
  return ;
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      { 
	if(myOkActor) {	  
	  Standard_Boolean res;
	  Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject();
	  if ( !IO->hasEntry() )
	    break;

	  SMESH_Actor* ac = mySMESHGUI->FindActorByEntry( IO->getEntry(), res, true );
	  if ( !res )
	    break;

	  mySMESHGUI->ChangeRepresentation( ac, 0 );

	  vtkGeometryFilter *gf = vtkGeometryFilter::New();
	  gf->SetInput(ac->DataSource);
	  vtkFeatureEdges *edges = vtkFeatureEdges::New();
	  edges->SetInput( gf->GetOutput() );
	  edges->ColoringOn();
	  if ( BoundaryEdges->isOn() ) edges->BoundaryEdgesOn();
	  else edges->BoundaryEdgesOff();
	  if ( ManifoldEdges->isOn() ) edges->ManifoldEdgesOn();
	  else  edges->ManifoldEdgesOff();
	  if ( NonManifoldEdges->isOn() ) edges->NonManifoldEdgesOn();
	  else  edges->NonManifoldEdgesOff();
	  if ( FeatureEdges->isOn() ) edges->FeatureEdgesOn();
	  else  edges->FeatureEdgesOff();

	  double angle = SpinBox_FeatureAngle->GetValue() ;
	  edges->SetFeatureAngle( angle );

	  vtkDataSetMapper *EdgeMapper = vtkDataSetMapper::New();
	  EdgeMapper->SetInput( edges->GetOutput() );
	  EdgeMapper->SetScalarModeToUseCellData();
	  ac->SetMapper(EdgeMapper);

	  mySMESHGUI->DisplayEdgesConnectivityLegendBox(ac);
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
void SMESHGUI_EdgesConnectivityDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;

  return ;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::ClickOnCancel()
{
  mySMESHGUI->ScalarVisibilityOff();
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  mySMESHGUI->EraseSimulationActors();
  mySMESHGUI->ScalarVisibilityOff();
  reject() ;
  return ;
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::SelectionIntoArgument()
{
  myEditCurrentArgument->setText("") ;
  myOkActor = false;
  QString aString = "";

  int nbSel = mySMESHGUI->GetNameOfSelectedIObjects(mySelection, aString) ;
  if(nbSel != 1) {
    mySMESHGUI->ScalarVisibilityOff();
    return;
  }

  myEditCurrentArgument->setText(aString) ;
  myOkActor = true ;
  return ;
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::SetEditCurrentArgument()
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
void SMESHGUI_EdgesConnectivityDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
  }
  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_EdgesConnectivityDlg::ActivateThisDialog()
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
void SMESHGUI_EdgesConnectivityDlg::enterEvent(QEvent* e)
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
void SMESHGUI_EdgesConnectivityDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
  return ;
}

void SMESHGUI_EdgesConnectivityDlg::CheckBox( int state )
{
    QCheckBox* send = (QCheckBox*)sender();
    if( send == BoundaryEdges ) {
    } else if( send == ManifoldEdges ) {
    } else if( send == NonManifoldEdges ) {
    } else {
    }
}

