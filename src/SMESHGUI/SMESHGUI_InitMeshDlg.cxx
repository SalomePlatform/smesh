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
//  File   : SMESHGUI_InitMeshDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_InitMeshDlg.h"
#include "SMESHGUI.h"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "QAD_WaitCursor.h"
#include "QAD_Operation.h"

#include "utilities.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>

//=================================================================================
// class    : SMESHGUI_InitMeshDlg()
// purpose  : Constructs a SMESHGUI_InitMeshDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_InitMeshDlg::SMESHGUI_InitMeshDlg( QWidget* parent, const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose )
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));
    if ( !name )
	setName( "SMESHGUI_InitMeshDlg" );
    setCaption( tr( "SMESH_INIT_MESH"  ) );
    setSizeGripEnabled( TRUE );
    QGridLayout* SMESHGUI_InitMeshDlgLayout = new QGridLayout( this ); 
    SMESHGUI_InitMeshDlgLayout->setSpacing( 6 );
    SMESHGUI_InitMeshDlgLayout->setMargin( 11 );
    
    /***************************************************************/
    GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), this, "GroupC1" );
    GroupC1->setColumnLayout(0, Qt::Vertical );
    GroupC1->layout()->setSpacing( 0 );
    GroupC1->layout()->setMargin( 0 );
    QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
    GroupC1Layout->setAlignment( Qt::AlignTop );
    GroupC1Layout->setSpacing( 6 );
    GroupC1Layout->setMargin( 11 );

    TextLabel_NameMesh = new QLabel( tr( "SMESH_NAME" ), GroupC1, "TextLabel_NameMesh" );
    GroupC1Layout->addWidget( TextLabel_NameMesh, 0, 0 );
    LineEdit_NameMesh = new QLineEdit( GroupC1, "LineEdit_NameMesh" );
    GroupC1Layout->addWidget( LineEdit_NameMesh, 0, 2 );

    TextLabelC1A1 = new QLabel( tr( "SMESH_OBJECT_GEOM" ), GroupC1, "TextLabelC1A1" );
    GroupC1Layout->addWidget( TextLabelC1A1, 1, 0 );
    SelectButtonC1A1 = new QPushButton( GroupC1, "SelectButtonC1A1" );
    SelectButtonC1A1->setPixmap( image0 );
    SelectButtonC1A1->setToggleButton( FALSE );
    GroupC1Layout->addWidget( SelectButtonC1A1, 1, 1 );
    LineEditC1A1 = new QLineEdit( GroupC1, "LineEditC1A1" );
    GroupC1Layout->addWidget( LineEditC1A1, 1, 2 );

    TextLabelC1A1Hyp = new QLabel( tr( "SMESH_OBJECT_HYPOTHESIS" ), GroupC1, "TextLabelC1A1Hyp" );
    GroupC1Layout->addWidget( TextLabelC1A1Hyp, 2, 0 );
    SelectButtonC1A1Hyp = new QPushButton( GroupC1, "SelectButtonC1A1Hyp" );
    SelectButtonC1A1Hyp->setPixmap( image0 );
    GroupC1Layout->addWidget( SelectButtonC1A1Hyp, 2, 1 );
    LineEditC1A1Hyp = new QLineEdit( GroupC1, "LineEditC1A1Hyp" );
    GroupC1Layout->addWidget( LineEditC1A1Hyp, 2, 2 );

    TextLabelC1A1Algo = new QLabel( tr( "SMESH_OBJECT_ALGORITHM" ), GroupC1, "TextLabelC1A1Algo" );
    GroupC1Layout->addWidget( TextLabelC1A1Algo, 3, 0 );
    SelectButtonC1A1Algo = new QPushButton( GroupC1, "SelectButtonC1A1Algo" );
    SelectButtonC1A1Algo->setPixmap( image0 );
    GroupC1Layout->addWidget( SelectButtonC1A1Algo, 3, 1 );
    LineEditC1A1Algo = new QLineEdit( GroupC1, "LineEditC1A1Algo" );
    GroupC1Layout->addWidget( LineEditC1A1Algo, 3, 2 );

    SMESHGUI_InitMeshDlgLayout->addWidget( GroupC1, 1, 0 );

    /***************************************************************/
    GroupButtons = new QGroupBox( this, "GroupButtons" );
    GroupButtons->setColumnLayout(0, Qt::Vertical );
    GroupButtons->layout()->setSpacing( 0 );
    GroupButtons->layout()->setMargin( 0 );
    QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
    GroupButtonsLayout->setAlignment( Qt::AlignTop );
    GroupButtonsLayout->setSpacing( 6 );
    GroupButtonsLayout->setMargin( 11 );

    buttonOk = new QPushButton( tr( "SMESH_BUT_OK" ), GroupButtons, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );

    buttonApply = new QPushButton( tr( "SMESH_BUT_APPLY" ), GroupButtons, "buttonApply" );
    buttonApply->setAutoDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonApply, 0, 1 );

    GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 2 );

    buttonCancel = new QPushButton( tr( "SMESH_BUT_CLOSE" ), GroupButtons, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );

    SMESHGUI_InitMeshDlgLayout->addWidget( GroupButtons, 2, 0 );

    /***************************************************************/
    Init( Sel ) ;
}


//=================================================================================
// function : ~SMESHGUI_InitMeshDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_InitMeshDlg::~SMESHGUI_InitMeshDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::Init( SALOME_Selection* Sel )
{
  mySelection = Sel;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  myGeomFilter       = new SALOME_TypeFilter( "GEOM" );
  myAlgorithmFilter  = new SMESH_TypeFilter( ALGORITHM );
  myHypothesisFilter = new SMESH_TypeFilter( HYPOTHESIS );

  /* signals and slots connections */
  connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonApply,  SIGNAL( clicked() ), this, SLOT( ClickOnApply() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;

  connect( SelectButtonC1A1,     SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A1Hyp,  SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A1Algo, SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( mySelection, SIGNAL( currentSelectionChanged() ),      this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI,  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySMESHGUI,  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( ClickOnCancel() ) ) ;
  
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; 

  LineEdit_NameMesh->setText( tr( "SMESH_OBJECT_MESH" ) );
  LineEdit_NameMesh->setFocus() ;
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection->ClearFilters() ;   
  mySelection->AddFilter( myGeomFilter ) ;

  SelectionIntoArgument();

  UpdateControlState();
}


//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::ClickOnOk()
{
  if ( this->ClickOnApply() )
    this->ClickOnCancel() ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_InitMeshDlg::ClickOnApply()
{
  QString myNameMesh = LineEdit_NameMesh->text().stripWhiteSpace();
  if ( myNameMesh.isEmpty() ) {
    QAD_MessageBox::warn1( this, tr( "SMESH_WRN_WARNING" ), tr( "SMESH_WRN_EMPTY_NAME" ), tr( "SMESH_BUT_OK" ) );
    return false;
  }
  
  if ( myGeomShape->_is_nil() || !HypoList.count() || !AlgoList.count() )
    return false;

  QAD_WaitCursor wc;

  QAD_Operation* op = new QAD_Operation( mySMESHGUI->GetActiveStudy() );

  // start transaction
  op->start();
  
  // create mesh
  SMESH::SMESH_Mesh_var aMesh = mySMESHGUI->InitMesh( myGeomShape, myNameMesh ) ;
  
  if ( !aMesh->_is_nil() ) {
    // assign hypotheses
    for( int i = 0; i < HypoList.count(); i++ ) {
      SALOMEDS::SObject_var aHypSO = mySMESHGUI->GetStudy()->FindObjectID( HypoList[i] );
      if ( !aHypSO->_is_nil() ) {
	CORBA::Object_var anObject = aHypSO->GetObject();
	if ( !CORBA::is_nil( anObject ) ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
	  if ( !aHyp->_is_nil() ) {
	    if ( !mySMESHGUI->AddHypothesisOnMesh( aMesh, aHyp ) ) {
	      // abort transaction
	      op->abort();
	      return false;
	    }
	  }
	}
      }
    }
    // assign algorithms
    for( int i = 0; i < AlgoList.count(); i++ ) {
      SALOMEDS::SObject_var aHypSO = mySMESHGUI->GetStudy()->FindObjectID( AlgoList[i] );
      if ( !aHypSO->_is_nil() ) {
	CORBA::Object_var anObject = aHypSO->GetObject();
	if ( !CORBA::is_nil( anObject ) ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
	  if ( !aHyp->_is_nil() ) {
	    if ( !mySMESHGUI->AddAlgorithmOnMesh( aMesh, aHyp ) ) {
	      // abort transaction
	      op->abort();
	      return false;
	    }
	  }
	}
      }
    }
  }
  // commit transaction
  op->finish();
  return true;
}


//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::ClickOnCancel()
{
  close();
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_InitMeshDlg::SelectionIntoArgument()
{
  QString aString = ""; 

  int nbSel = mySMESHGUI->GetNameOfSelectedIObjects(mySelection, aString) ;

  if ( myEditCurrentArgument == LineEditC1A1 ) {
    // geom shape
    if ( nbSel != 1 ) {
      myGeomShape = GEOM::GEOM_Shape::_nil();
      aString = "";
    } 
    else {
      Standard_Boolean testResult ;
      Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
      myGeomShape = mySMESHGUI->ConvertIOinGEOMShape(IO, testResult) ;
      if( !testResult )  {
	myGeomShape = GEOM::GEOM_Shape::_nil();
	aString = "";
      }
    }
  }
  else if ( myEditCurrentArgument == LineEditC1A1Hyp ) {
    // hypotheses
    HypoList.clear();
    if ( nbSel >= 1 ) {
      SALOME_ListIteratorOfListIO Itinit( mySelection->StoredIObjects() );
      for ( ; Itinit.More(); Itinit.Next() ) {
	HypoList.append( Itinit.Value()->getEntry() );
      }
      if ( nbSel > 1 )
	aString = tr( "%1 Hypothesis" ).arg( nbSel ) ;
    }
    else {
      aString = "";
    }
  }
  else if ( myEditCurrentArgument == LineEditC1A1Algo ) {
    // algorithms
    AlgoList.clear(); 
    if ( nbSel >= 1 ) {
      SALOME_ListIteratorOfListIO Itinit( mySelection->StoredIObjects() );
      for ( ; Itinit.More(); Itinit.Next() ) {
	AlgoList.append( Itinit.Value()->getEntry() );
      }
      if ( nbSel > 1 )
	aString = tr( "%1 Algorithms" ).arg( nbSel ) ;
    }
    else {
      aString = "";
    }
  }

  myEditCurrentArgument->setText(aString) ;

  UpdateControlState();
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  if(send == SelectButtonC1A1) {
    LineEditC1A1->setFocus() ;
    myEditCurrentArgument = LineEditC1A1;
    mySelection->ClearFilters() ;	  
    mySelection->AddFilter(myGeomFilter) ;
  } else if( send == SelectButtonC1A1Hyp ) {
    LineEditC1A1Hyp->setFocus() ;
    myEditCurrentArgument = LineEditC1A1Hyp ;
    mySelection->ClearFilters() ;	  
    mySelection->AddFilter(myHypothesisFilter) ;
  } else if( send == SelectButtonC1A1Algo ) {
    LineEditC1A1Algo->setFocus() ;
    myEditCurrentArgument = LineEditC1A1Algo ;
    mySelection->ClearFilters() ;	  
    mySelection->AddFilter(myAlgorithmFilter) ;
  }
  SelectionIntoArgument() ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::DeactivateActiveDialog()
{
  if ( GroupC1->isEnabled() ) {
    disconnect( mySelection, 0, this, 0 );
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
  }
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  connect ( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::enterEvent(QEvent* e)
{
  if ( !GroupC1->isEnabled() )
    ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::closeEvent( QCloseEvent* e )
{
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  mySelection->ClearFilters() ;
  QDialog::closeEvent( e );
}

//=================================================================================
// function : UpdateControlState()
// purpose  :
//=================================================================================
void SMESHGUI_InitMeshDlg::UpdateControlState()
{
  bool isEnabled = ( !myGeomShape->_is_nil() && HypoList.count() && AlgoList.count() );

  buttonOk   ->setEnabled( isEnabled );
  buttonApply->setEnabled( isEnabled );
}





