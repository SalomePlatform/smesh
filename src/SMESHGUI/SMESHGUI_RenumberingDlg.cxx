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
//  File   : SMESHGUI_RenumberingDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RenumberingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "utilities.h"

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpixmap.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_RenumberingDlg()
// purpose  : 
//=================================================================================
SMESHGUI_RenumberingDlg::SMESHGUI_RenumberingDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
						  const int unit, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
	       Qt::WDestructiveClose)
{
  myUnit = unit;

  if ( !name )
    setName( "SMESHGUI_RenumberingDlg" );
  resize( 303, 185 );
  

  QPixmap image0;
  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if (unit == 0) {
    image0 = QPixmap(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_RENUMBERING_NODES")));
    setCaption( tr( "SMESH_RENUMBERING_NODES_TITLE"  ) );
  }
  else if (unit == 1) {
    image0 = QPixmap(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_RENUMBERING_ELEMENTS")));
    setCaption( tr( "SMESH_RENUMBERING_ELEMENTS_TITLE"  ) );
  }
  
  setSizeGripEnabled( TRUE );
  SMESHGUI_RenumberingDlgLayout = new QGridLayout( this ); 
  SMESHGUI_RenumberingDlgLayout->setSpacing( 6 );
  SMESHGUI_RenumberingDlgLayout->setMargin( 11 );
  
  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  if (unit == 0)
    GroupConstructors->setTitle( tr( "SMESH_NODES"  ) );
  else if (unit == 1)
    GroupConstructors->setTitle( tr( "SMESH_ELEMENTS"  ) );
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
  SMESHGUI_RenumberingDlgLayout->addWidget( GroupConstructors, 0, 0 );
  
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
  SMESHGUI_RenumberingDlgLayout->addWidget( GroupButtons, 2, 0 );
  
  /***************************************************************/
  GroupMesh = new QGroupBox( this, "GroupMesh" );
  GroupMesh->setTitle( tr( "SMESH_RENUMBERING"  ) );
  GroupMesh->setMinimumSize( QSize( 0, 0 ) );
  GroupMesh->setFrameShape( QGroupBox::Box );
  GroupMesh->setFrameShadow( QGroupBox::Sunken );
  GroupMesh->setColumnLayout(0, Qt::Vertical );
  GroupMesh->layout()->setSpacing( 0 );
  GroupMesh->layout()->setMargin( 0 );
  GroupMeshLayout = new QGridLayout( GroupMesh->layout() );
  GroupMeshLayout->setAlignment( Qt::AlignTop );
  GroupMeshLayout->setSpacing( 6 );
  GroupMeshLayout->setMargin( 11 );
  TextLabelMesh = new QLabel( GroupMesh, "TextLabelMesh" );
  TextLabelMesh->setText( tr( "SMESH_MESH" ) );
  TextLabelMesh->setMinimumSize( QSize( 50, 0 ) );
  TextLabelMesh->setFrameShape( QLabel::NoFrame );
  TextLabelMesh->setFrameShadow( QLabel::Plain );
  GroupMeshLayout->addWidget( TextLabelMesh, 0, 0 );
  SelectButton = new QPushButton( GroupMesh, "SelectButton" );
  SelectButton->setText( tr( ""  ) );
  SelectButton->setPixmap( image1 );
  SelectButton->setToggleButton( FALSE );
  GroupMeshLayout->addWidget( SelectButton, 0, 1 );
  LineEditMesh = new QLineEdit( GroupMesh, "LineEditMesh" );
  LineEditMesh->setReadOnly( true );
  GroupMeshLayout->addWidget( LineEditMesh, 0, 2 );
  SMESHGUI_RenumberingDlgLayout->addWidget( GroupMesh, 1, 0 );
  
  Init(Sel) ; /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RenumberingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RenumberingDlg::~SMESHGUI_RenumberingDlg()
{
  // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::Init( SALOME_Selection* Sel )
{

  GroupMesh->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  mySelection = Sel;  
  mySMESHGUI  = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this );

  myMesh = SMESH::SMESH_Mesh::_nil();
    
  myMeshFilter = new SMESH_TypeFilter( MESH );

  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );
  
  connect( SelectButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */
  
  myEditCurrentArgument = LineEditMesh ;
  LineEditMesh->setFocus();
  mySelection->ClearFilters() ;   
  mySelection->AddFilter( myMeshFilter ) ;
  
  SelectionIntoArgument();
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RenumberingDlg::ConstructorsClicked(int constructorId)
{
  return ;
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnApply()
{
  if (mySMESHGUI->ActiveStudyLocked())
    return;
  
  if ( !myMesh->_is_nil())
    {
      try
	{
	  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	  SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
	  bool isUnitsLabeled = false;
	  
	  if (myUnit == 0 && anActor) {
	    isUnitsLabeled = anActor->GetPointsLabeled();
	    if (isUnitsLabeled)  anActor->SetPointsLabeled(false);
	  }
	  else if (myUnit == 1 && anActor) {
	    isUnitsLabeled = anActor->GetCellsLabeled();
	    if (isUnitsLabeled)  anActor->SetCellsLabeled(false);
	  }
	  
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  if (myUnit == 0) {
	    aMeshEditor->RenumberNodes();
	    if (isUnitsLabeled && anActor) anActor->SetPointsLabeled(true);
	  }
	  else if (myUnit == 1) {
	    aMeshEditor->RenumberElements();
	    if (isUnitsLabeled && anActor) anActor->SetCellsLabeled(true);
	  }
	  QApplication::restoreOverrideCursor();
	}
      catch( ... )
	{
	}
      
      mySelection->ClearIObjects();
      SMESH::UpdateView();
    }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnOk()
{
  ClickOnApply() ;
  ClickOnCancel() ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnCancel()
{
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RenumberingDlg::SelectionIntoArgument()
{
  if ( !GroupButtons->isEnabled() ) // inactive
    return;
  
  QString aString = ""; 
  
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelection, aString) ;

  if ( myEditCurrentArgument == LineEditMesh ) {
    // mesh
    if ( nbSel != 1 ) {
      myMesh = SMESH::SMESH_Mesh::_nil();
      aString = "";
    }
    else {
      Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO) ;
      if( myMesh->_is_nil() )
	aString = "";
    }
  }
  
  myEditCurrentArgument->setText(aString) ;
  
  bool isEnabled = (!myMesh->_is_nil());
  buttonOk->setEnabled( isEnabled );
  buttonApply->setEnabled( isEnabled );
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {	
	if(send == SelectButton) {
	  LineEditMesh->setFocus() ;
	  myEditCurrentArgument = LineEditMesh;
	}
	SelectionIntoArgument() ;
	break;
      }
    }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupMesh->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupMesh->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;
  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
}


//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_RenumberingDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel();
}
