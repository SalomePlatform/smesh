using namespace std;
//  File      : SMESHGUI_AddSubMeshDlg.cxx
//  Created   : Mon May 27 10:20:11 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#include "SMESHGUI_AddSubMeshDlg.h"
#include "SMESHGUI.h"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
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


//=================================================================================
// class    : SMESHGUI_AddSubMeshDlg()
// purpose  : Constructs a SMESHGUI_AddSubMeshDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_AddSubMeshDlg::SMESHGUI_AddSubMeshDlg( QWidget* parent, const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_ADD_SUBMESH")));
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));
    if ( !name )
	setName( "SMESHGUI_AddSubMeshDlg" );
    resize( 303, 175 ); 
    setCaption( tr( "SMESH_ADD_SUBMESH"  ) );
    setSizeGripEnabled( TRUE );
    SMESHGUI_AddSubMeshDlgLayout = new QGridLayout( this ); 
    SMESHGUI_AddSubMeshDlgLayout->setSpacing( 6 );
    SMESHGUI_AddSubMeshDlgLayout->setMargin( 11 );
    
    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_SUBMESH"  ) );
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
    Constructor1->setPixmap( image1 );
    Constructor1->setChecked( TRUE );
    Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
    Constructor1->setMinimumSize( QSize( 50, 0 ) );
    GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupConstructorsLayout->addItem( spacer, 0, 1 );
    SMESHGUI_AddSubMeshDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
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
    SMESHGUI_AddSubMeshDlgLayout->addWidget( GroupButtons, 2, 0 );

    /***************************************************************/
    GroupC1 = new QGroupBox( this, "GroupC1" );
    GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
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
    TextLabelC1A1->setText( tr( "SMESH_OBJECT_MESH"  ) );
    TextLabelC1A1->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A1->setFrameShape( QLabel::NoFrame );
    TextLabelC1A1->setFrameShadow( QLabel::Plain );
    GroupC1Layout->addWidget( TextLabelC1A1, 0, 0 );
    SelectButtonC1A1 = new QPushButton( GroupC1, "SelectButtonC1A1" );
    SelectButtonC1A1->setText( tr( ""  ) );
    SelectButtonC1A1->setPixmap( image0 );
    SelectButtonC1A1->setToggleButton( FALSE );
    GroupC1Layout->addWidget( SelectButtonC1A1, 0, 1 );
    LineEditC1A1 = new QLineEdit( GroupC1, "LineEditC1A1" );
    GroupC1Layout->addWidget( LineEditC1A1, 0, 2 );

    TextLabelC1A2 = new QLabel( GroupC1, "TextLabelC1A2" );
    TextLabelC1A2->setText( tr( "SMESH_OBJECT_GEOM"  ) );
    TextLabelC1A2->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A2->setFrameShape( QLabel::NoFrame );
    TextLabelC1A2->setFrameShadow( QLabel::Plain );
    GroupC1Layout->addWidget( TextLabelC1A2, 1, 0 );
    SelectButtonC1A2 = new QPushButton( GroupC1, "SelectButtonC1A2" );
    SelectButtonC1A2->setText( tr( ""  ) );
    SelectButtonC1A2->setPixmap( image0 );
    SelectButtonC1A2->setToggleButton( FALSE );
    GroupC1Layout->addWidget( SelectButtonC1A2, 1, 1 );
    LineEditC1A2 = new QLineEdit( GroupC1, "LineEditC1A2" );
    GroupC1Layout->addWidget( LineEditC1A2, 1, 2 );

    TextLabel_NameMesh = new QLabel( GroupC1, "TextLabel_NameMesh" );
    TextLabel_NameMesh->setText( tr( "SMESH_NAME"  ) );
    GroupC1Layout->addWidget( TextLabel_NameMesh, 2, 0 );
    LineEdit_NameMesh = new QLineEdit( GroupC1, "LineEdit_NameMesh" );
    GroupC1Layout->addWidget( LineEdit_NameMesh, 2, 2 );

    TextLabelC1A1Hyp = new QLabel( GroupC1, "TextLabelC1A1Hyp" );
    TextLabelC1A1Hyp->setText( tr( "SMESH_OBJECT_HYPOTHESIS" ) );
    TextLabelC1A1Hyp->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A1Hyp->setFrameShape( QLabel::NoFrame );
    TextLabelC1A1Hyp->setFrameShadow( QLabel::Plain );
    GroupC1Layout->addWidget( TextLabelC1A1Hyp, 3, 0 );
    SelectButtonC1A1Hyp = new QPushButton( GroupC1, "SelectButtonC1A1Hyp" );
    SelectButtonC1A1Hyp->setText( tr( ""  ) );
    SelectButtonC1A1Hyp->setPixmap( image0 );
    GroupC1Layout->addWidget( SelectButtonC1A1Hyp, 3, 1 );
    LineEditC1A1Hyp = new QLineEdit( GroupC1, "LineEditC1A1Hyp" );
    LineEditC1A1Hyp->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, LineEditC1A1Hyp->sizePolicy().hasHeightForWidth() ) );
    GroupC1Layout->addWidget( LineEditC1A1Hyp, 3, 2 );

    TextLabelC1A1Algo = new QLabel( GroupC1, "TextLabelC1A1Algo" );
    TextLabelC1A1Algo->setText( tr( "SMESH_OBJECT_ALGORITHM" ) );
    TextLabelC1A1Algo->setMinimumSize( QSize( 50, 0 ) );
    TextLabelC1A1Algo->setFrameShape( QLabel::NoFrame );
    TextLabelC1A1Algo->setFrameShadow( QLabel::Plain );
    GroupC1Layout->addWidget( TextLabelC1A1Algo, 4, 0 );
    SelectButtonC1A1Algo = new QPushButton( GroupC1, "SelectButtonC1A1Algo" );
    SelectButtonC1A1Algo->setText( tr( ""  ) );
    SelectButtonC1A1Algo->setPixmap( image0 );
    GroupC1Layout->addWidget( SelectButtonC1A1Algo, 4, 1 );
    LineEditC1A1Algo = new QLineEdit( GroupC1, "LineEditC1A1Algo" );
    LineEditC1A1Algo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, LineEditC1A1Algo->sizePolicy().hasHeightForWidth() ) );
    GroupC1Layout->addWidget( LineEditC1A1Algo, 4, 2 );

    SMESHGUI_AddSubMeshDlgLayout->addWidget( GroupC1, 1, 0 );
    /***************************************************************/

    Init(Sel) ;
}


//=================================================================================
// function : ~SMESHGUI_AddSubMeshDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AddSubMeshDlg::~SMESHGUI_AddSubMeshDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::Init( SALOME_Selection* Sel )
{
  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  myGeomFilter = new SALOME_TypeFilter( "GEOM" );
  myMeshFilter = new SMESH_TypeFilter( MESH );

  myAlgorithmFilter = new SMESH_TypeFilter( ALGORITHM );
  myHypothesisFilter = new SMESH_TypeFilter( HYPOTHESIS );

  myNameSubMesh = "SubMesh";

  myGeomShape = GEOM::GEOM_Shape::_nil();
  myMesh = SMESH::SMESH_Mesh::_nil();

  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );

  connect( SelectButtonC1A1, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A2, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( LineEdit_NameMesh, SIGNAL (textChanged(const QString&)  ), this, SLOT( TextChangedInLineEdit(const QString&) ) ) ;

  connect( SelectButtonC1A1Hyp, SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A1Algo, SIGNAL (clicked() ),  this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( mySelection, SIGNAL( currentSelectionChanged() ),     this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; 

  SelectionIntoArgument();

  return ;
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_AddSubMeshDlg::ConstructorsClicked(int constructorId)
{
  return ;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::ClickOnApply()
{
  switch(myConstructorId)
    { 
    case 0 :
      { 
	if ( !myNameSubMesh.isEmpty() && !myNameSubMesh.isNull() && 
	     !myGeomShape->_is_nil() && !myMesh->_is_nil()) {
	  mySubMesh = mySMESHGUI->AddSubMesh( myMesh, myGeomShape, myNameSubMesh ) ;
	}
	
	if( myOkHypothesis && !mySubMesh->_is_nil() ) {
	  SALOME_ListIteratorOfListIO It( HypoList );
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    Standard_Boolean testResult;
	    myHypothesis = mySMESHGUI->ConvertIOinSMESHHypothesis(IObject, testResult) ;
	    if( testResult )
	      mySMESHGUI->AddHypothesisOnSubMesh(mySubMesh, myHypothesis) ;
	  }
	}

	if( myOkAlgorithm && !mySubMesh->_is_nil() ) {
	  SALOME_ListIteratorOfListIO It( AlgoList );
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    Standard_Boolean testResult;
	    myAlgorithm = mySMESHGUI->ConvertIOinSMESHHypothesis(IObject, testResult) ;
	    if( testResult )
	      mySMESHGUI->AddAlgorithmOnSubMesh(mySubMesh, myAlgorithm) ;
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
void SMESHGUI_AddSubMeshDlg::ClickOnCancel()
{
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  mySelection->ClearFilters() ;
  reject() ;
  return ;
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_AddSubMeshDlg::SelectionIntoArgument()
{
  myEditCurrentArgument->setText("") ;
  QString aString = ""; 

  int nbSel = mySMESHGUI->GetNameOfSelectedIObjects(mySelection, aString) ;

  switch (myConstructorId) 
    {
    case 0:
      {
	if ( myEditCurrentArgument == LineEditC1A1 ) {
	  if ( nbSel != 1 ) {
	    myOkHypothesis = false;
	    myOkAlgorithm = false;
	    return ;
	  } else {
	    Standard_Boolean testResult ;
	    Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
	    myMesh = mySMESHGUI->ConvertIOinMesh(IO, testResult) ;
	    if( !testResult ) {
	      myMesh = SMESH::SMESH_Mesh::_nil();
	      return ;
	    }
	  }
	} else if ( myEditCurrentArgument == LineEditC1A2 ) {
	  if ( nbSel != 1 ) {
	    myOkHypothesis = false;
	    myOkAlgorithm = false;
	    return ;
	  } else {
	    Standard_Boolean testResult ;
	    Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
	    myGeomShape = mySMESHGUI->ConvertIOinGEOMShape(IO, testResult) ;
	    if( !testResult ) {
	      myGeomShape = GEOM::GEOM_Shape::_nil();
	      return ;
	    }
	  }
	} else if ( myEditCurrentArgument == LineEditC1A1Hyp ) {
	  if ( nbSel >= 1 ) {
	    HypoList.Clear();
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
	} else if ( myEditCurrentArgument == LineEditC1A1Algo ) {
	  if ( nbSel >= 1 ) {
	    AlgoList.Clear(); 
	    SALOME_ListIteratorOfListIO Itinit( mySelection->StoredIObjects() );
	    for (; Itinit.More(); Itinit.Next()) {
	      AlgoList.Append(Itinit.Value());
	    }
	    myOkAlgorithm = true ;
	    if (nbSel > 1)
	      aString = tr("%1 Algorithms").arg(nbSel) ;
	    LineEditC1A1Algo->setText(aString) ;
	  }
	  else {
	    myOkAlgorithm = false ;
	    return ;
	  }
	}
	break;
      }
    }
  
  myEditCurrentArgument->setText(aString) ;
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {	
	if(send == SelectButtonC1A1) {
	  LineEditC1A1->setFocus() ;
	  myEditCurrentArgument = LineEditC1A1;
	  mySelection->ClearFilters() ;	  
	  mySelection->AddFilter(myMeshFilter) ;
	} else if (send == SelectButtonC1A2) {
	  LineEditC1A2->setFocus() ;
	  myEditCurrentArgument = LineEditC1A2;
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
	break;
      }
    }
  return ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    disconnect( mySelection, 0, this, 0 );
    GroupConstructors->setEnabled(false) ;
    GroupC1->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
  }
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupC1->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  connect ( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::enterEvent(QEvent* e)
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
void SMESHGUI_AddSubMeshDlg::closeEvent( QCloseEvent* e )
{
  this->ClickOnCancel() ;
  return ;
}

//=================================================================================
// function : TextChangedInLineEdit()
// purpose  :
//=================================================================================
void SMESHGUI_AddSubMeshDlg::TextChangedInLineEdit(const QString& newText)
{  
  QLineEdit* send = (QLineEdit*)sender();
  QString newT = strdup(newText) ;
  
  if (send == LineEdit_NameMesh) {
    myNameSubMesh = newText;
  }
  return ;
}
