using namespace std;
//  File      : SMESHGUI_AddHypothesisDlg.cxx
//  Created   : Wed May 22 19:05:31 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESHGUI
//  Copyright : Open CASCADE 2002
//  $Header$


#include "SMESHGUI_AddHypothesisDlg.h"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "SMESHGUI.h"
#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qframe.h>
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
#include <qevent.h>


//=================================================================================
// class    : SMESHGUI_AddHypothesisDlg()
// purpose  : Constructs a SMESHGUI_AddHypothesisDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_AddHypothesisDlg::SMESHGUI_AddHypothesisDlg( QWidget* parent, const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
     QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_ADD_HYPOTHESIS")));
     QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

    if ( !name )
	setName( "SMESHGUI_AddHypothesisDlg" );
    resize( 322, 220 ); 
    setCaption( tr( "SMESH_ADD_HYPOTHESIS_TITLE" ) );
    setSizeGripEnabled( TRUE );

    SMESHGUI_AddHypothesisDlgLayout = new QGridLayout( this ); 
    SMESHGUI_AddHypothesisDlgLayout->setSpacing( 6 );
    SMESHGUI_AddHypothesisDlgLayout->setMargin( 11 );

    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_ADD_HYPOTHESIS"  ) );
    GroupConstructors->setExclusive( TRUE );
    GroupConstructors->setColumnLayout(0, Qt::Vertical );
    GroupConstructors->layout()->setSpacing( 0 );
    GroupConstructors->layout()->setMargin( 0 );
    GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
    GroupConstructorsLayout->setAlignment( Qt::AlignTop );
    GroupConstructorsLayout->setSpacing( 6 );
    GroupConstructorsLayout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupConstructorsLayout->addItem( spacer, 0, 1 );
    Constructor1 = new QRadioButton( GroupConstructors, "Constructor1" );
    Constructor1->setText( tr( ""  ) );
    Constructor1->setPixmap( image0 );
    Constructor1->setChecked( TRUE );
    Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
    GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
    SMESHGUI_AddHypothesisDlgLayout->addWidget( GroupConstructors, 0, 0 );

    /***************************************************************/
    GroupConstructor1 = new QGroupBox( this, "GroupConstructor1" );
    GroupConstructor1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
    GroupConstructor1->setColumnLayout(0, Qt::Vertical );
    GroupConstructor1->layout()->setSpacing( 0 );
    GroupConstructor1->layout()->setMargin( 0 );
    GroupConstructor1Layout = new QGridLayout( GroupConstructor1->layout() );
    GroupConstructor1Layout->setAlignment( Qt::AlignTop );
    GroupConstructor1Layout->setSpacing( 6 );
    GroupConstructor1Layout->setMargin( 11 );
    LineEditC1A2Mesh = new QLineEdit( GroupConstructor1, "LineEditC1A2Mesh" );
    LineEditC1A2Mesh->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, LineEditC1A2Mesh->sizePolicy().hasHeightForWidth() ) );
    GroupConstructor1Layout->addWidget( LineEditC1A2Mesh, 1, 2 );
    LineEditC1A1Hyp = new QLineEdit( GroupConstructor1, "LineEditC1A1Hyp" );
    LineEditC1A1Hyp->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, LineEditC1A1Hyp->sizePolicy().hasHeightForWidth() ) );
    GroupConstructor1Layout->addWidget( LineEditC1A1Hyp, 0, 2 );
    SelectButtonC1A1Hyp = new QPushButton( GroupConstructor1, "SelectButtonC1A1Hyp" );
    SelectButtonC1A1Hyp->setText( tr( ""  ) );
    SelectButtonC1A1Hyp->setPixmap( image1 );
    GroupConstructor1Layout->addWidget( SelectButtonC1A1Hyp, 0, 1 );
    SelectButtonC1A2Mesh = new QPushButton( GroupConstructor1, "SelectButtonC1A2Mesh" );
    SelectButtonC1A2Mesh->setText( tr( ""  ) );
    SelectButtonC1A2Mesh->setPixmap( image1 );
    GroupConstructor1Layout->addWidget( SelectButtonC1A2Mesh, 1, 1 );
    TextLabelC1A2Mesh = new QLabel( GroupConstructor1, "TextLabelC1A2Mesh" );
    TextLabelC1A2Mesh->setText( tr( "SMESH_OBJECT_MESHorSUBMESH" ) );
    TextLabelC1A2Mesh->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A2Mesh->setFrameShape( QLabel::NoFrame );
    TextLabelC1A2Mesh->setFrameShadow( QLabel::Plain );
    GroupConstructor1Layout->addWidget( TextLabelC1A2Mesh, 1, 0 );
    TextLabelC1A1Hyp = new QLabel( GroupConstructor1, "TextLabelC1A1Hyp" );
    TextLabelC1A1Hyp->setText( tr( "SMESH_OBJECT_HYPOTHESIS" ) );
    TextLabelC1A1Hyp->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A1Hyp->setFrameShape( QLabel::NoFrame );
    TextLabelC1A1Hyp->setFrameShadow( QLabel::Plain );
    GroupConstructor1Layout->addWidget( TextLabelC1A1Hyp, 0, 0 );
    SMESHGUI_AddHypothesisDlgLayout->addWidget( GroupConstructor1, 1, 0 );

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
    QSpacerItem* spacer_1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupButtonsLayout->addItem( spacer_1, 0, 2 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
    SMESHGUI_AddHypothesisDlgLayout->addWidget( GroupButtons, 2, 0 ); 

    /* Initialisation */
    Init( Sel ) ; 
}


//=================================================================================
// function : ~SMESHGUI_AddHypothesisDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddHypothesisDlg::~SMESHGUI_AddHypothesisDlg()
{  
  /* no need to delete child widgets, Qt does it all for us */
  this->destroy(TRUE, TRUE) ;
}



//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::Init( SALOME_Selection* Sel )
{  
  mySelection = Sel ;
  myConstructorId = 0 ;
  
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;

  GroupConstructor1->show();
  myConstructorId = 0 ;
  myEditCurrentArgument = LineEditC1A1Hyp ;	
  Constructor1->setChecked( TRUE );
  myOkMesh = myOkSubMesh = myOkHypothesis = false ;

  myHypothesisFilter = new SMESH_TypeFilter( HYPOTHESIS );
  myMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );

  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );
  connect( SelectButtonC1A1Hyp, SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A2Mesh, SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ),     this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
 
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* Displays Dialog */ 

  return ;
}



//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_AddHypothesisDlg::ConstructorsClicked(int constructorId)
{
  switch (constructorId)
    {
    case 0:
      {
	GroupConstructor1->show();
	myConstructorId = constructorId ;
	myEditCurrentArgument = LineEditC1A1Hyp ;
	LineEditC1A2Mesh->setText(tr("")) ;
	Constructor1->setChecked( TRUE );
	myOkMesh = myOkSubMesh = myOkHypothesis  = false ;
	break;
      }
    }
 return ;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;

  return ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      {
	if(myOkMesh && myOkHypothesis ) {
	  SALOME_ListIteratorOfListIO It( HypoList );
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    Standard_Boolean testResult;
	    myHypothesis = mySMESHGUI->ConvertIOinSMESHHypothesis(IObject, testResult) ;
	    if( testResult )
	      mySMESHGUI->AddHypothesisOnMesh(myMesh, myHypothesis) ;
	  }
	} else if( myOkSubMesh && myOkHypothesis ) {
	  SALOME_ListIteratorOfListIO It( HypoList );
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    Standard_Boolean testResult;
	    myHypothesis = mySMESHGUI->ConvertIOinSMESHHypothesis(IObject, testResult) ;
	    if( testResult )
	      mySMESHGUI->AddHypothesisOnSubMesh(mySubMesh, myHypothesis) ;
	  }
	}
	break ;
      }
    }
}


//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::ClickOnCancel()
{
  mySelection->ClearFilters() ;
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
  return ;
}



//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_AddHypothesisDlg::SelectionIntoArgument()
{
  myEditCurrentArgument->setText("") ;
  QString aString = "";

  int nbSel = mySMESHGUI->GetNameOfSelectedIObjects(mySelection, aString) ;

  if ( myEditCurrentArgument == LineEditC1A2Mesh ) {
    if ( nbSel == 1 ) {
      Handle(SALOME_InteractiveObject) IObject = mySelection->firstIObject();
      QString aName = IObject->getName();
      Standard_Boolean testResult;
      myOkMesh = true ;
      myMesh = mySMESHGUI->ConvertIOinMesh(IObject, testResult) ;
      if( !testResult ) {
	myOkMesh = false;
	myOkSubMesh = true;
	mySubMesh = mySMESHGUI->ConvertIOinSubMesh(IObject, testResult) ;
	if( !testResult ) {
	  myOkSubMesh = false;
	  return ;
	}
      }
      LineEditC1A2Mesh->setText(aString) ;
    }
    else {
      myOkMesh = myOkSubMesh = false ;
      return ;
    }
  }
  else if ( myEditCurrentArgument == LineEditC1A1Hyp ) {
    if ( nbSel >= 1 ) {
      HypoList.Clear(); // = mySelection->StoredIObjects() ;
      SALOME_ListIteratorOfListIO Itinit( mySelection->StoredIObjects() );
      for (; Itinit.More(); Itinit.Next()) {
	HypoList.Append(Itinit.Value());
      }
      myOkHypothesis = true ;
      if (nbSel > 1)
	aString = tr("%1 Hypothesis").arg(nbSel) ;
      LineEditC1A1Hyp->setText(aString) ;
    }
    else {
      myOkHypothesis = false ;
      return ;
    }
  }

  return ; 
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {	
	if( send == SelectButtonC1A1Hyp ) {
	  myEditCurrentArgument = LineEditC1A1Hyp ;
	  LineEditC1A1Hyp->setFocus() ;
	  mySelection->ClearFilters() ;	  
	  mySelection->AddFilter(myHypothesisFilter) ;
	}
	else if(send == SelectButtonC1A2Mesh) {
	  myEditCurrentArgument = LineEditC1A2Mesh;
	  LineEditC1A2Mesh->setFocus() ;
	  mySelection->ClearFilters() ;
	  mySelection->AddFilter(myMeshOrSubMeshFilter) ;
	}
	SelectionIntoArgument() ;
	break;
      }
    }
  return ;
}



//=================================================================================
// function : LineEditReturnPressed()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::LineEditReturnPressed()
{

  return ;
}



//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {

    GroupConstructors->setEnabled(false) ;
    GroupConstructor1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;  
    disconnect( mySelection, 0, this, 0 );
  }
  return ;
}



//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::closeEvent( QCloseEvent* e )
{
  this->ClickOnCancel() ; /* same than click on cancel button */
}


//=================================================================================
// function : enterEvent()
// purpose  : when mouse enter onto the QWidget
//=================================================================================
void SMESHGUI_AddHypothesisDlg::enterEvent( QEvent *  )
{
  if ( GroupConstructors->isEnabled() )
    return ;
  ActivateThisDialog() ;
}



//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddHypothesisDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;
  GroupConstructors->setEnabled(true) ;
  GroupConstructor1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  connect ( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  return ;
}
