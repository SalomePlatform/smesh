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
//  File   : SMESHGUI_EditHypothesesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_EditHypothesesDlg.h"
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

//VRV: porting on Qt 3.0.5
#if QT_VERSION >= 0x030005
#include <qlistbox.h>
#endif
//VRV: porting on Qt 3.0.5

//=================================================================================
// class    : SMESHGUI_EditHypothesesDlg()
// purpose  : Constructs a SMESHGUI_EditHypothesesDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_EditHypothesesDlg::SMESHGUI_EditHypothesesDlg( QWidget* parent, const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_EDIT_MESH")));
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));
    if ( !name )
	setName( "SMESHGUI_EditHypothesesDlg" );
    resize( 417, 573 ); 
    setCaption( tr( "SMESH_EDIT_HYPOTHESES"  ) );
    setSizeGripEnabled( TRUE );
    SMESHGUI_EditHypothesesDlgLayout = new QGridLayout( this ); 
    SMESHGUI_EditHypothesesDlgLayout->setSpacing( 6 );
    SMESHGUI_EditHypothesesDlgLayout->setMargin( 11 );

    /***************************************************************/
    GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
    GroupConstructors->setTitle( tr( "SMESH_HYPOTHESES"  ) );
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
    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupConstructors, 0, 0 );
    
    /***************************************************************/
    GroupButtons = new QGroupBox( this, "GroupButtons" );
    GroupButtons->setGeometry( QRect( 10, 10, 281, 96 ) ); 
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
    buttonCancel->setDefault( TRUE );
    buttonCancel->setEnabled( TRUE ) ;
    
    GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
    buttonApply = new QPushButton( GroupButtons, "buttonApply" );
    buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
    buttonApply->setAutoDefault( TRUE );
    buttonApply->setDefault( FALSE );
    buttonApply->setEnabled( FALSE ) ;
    
    GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
    QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupButtonsLayout->addItem( spacer_9, 0, 2 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( FALSE );
    buttonOk->setEnabled( FALSE ) ;
    
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupButtons, 5, 0 );

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
    TextLabelC1A1->setText( tr( "SMESH_OBJECT_MESHorSUBMESH"  ) );
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

    GroupHypotheses = new QGroupBox( this, "GroupHypotheses" );
    GroupHypotheses->setTitle( tr( "SMESH_HYPOTHESES"  ) );
    GroupHypotheses->setColumnLayout(0, Qt::Vertical );
    GroupHypotheses->layout()->setSpacing( 0 );
    GroupHypotheses->layout()->setMargin( 0 );
    grid_3 = new QGridLayout( GroupHypotheses->layout() );
    grid_3->setGeometry( QRect( 12, 18, 139, 110 ) ); 
    grid_3->setAlignment( Qt::AlignTop );
    grid_3->setSpacing( 6 );
    grid_3->setMargin( 11 );

    hbox_2 = new QHBoxLayout; 
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    vbox = new QVBoxLayout; 
    vbox->setSpacing( 6 );
    vbox->setMargin( 0 );

    TextHypDefinition = new QLabel( GroupHypotheses, "TextHypDefinition" );
    TextHypDefinition->setText( tr( "SMESH_AVAILABLE"  ) );
    vbox->addWidget( TextHypDefinition );

    ListHypDefinition = new QListBox( GroupHypotheses, "ListHypDefinition" );
    ListHypDefinition->setMinimumSize( 100, 50);
//      ListHypDefinition->setRowMode(4);
//      ListHypDefinition->setRowMode( QListBox::FixedNumber );
//    ListHypDefinition->setLineWidth( 4 );
//      ListHypDefinition->setColumnMode( QListBox::Variable );
//      ListHypDefinition->setVariableHeight( FALSE );
//      ListHypDefinition->insertItem( tr( "New Item" ) );
    vbox->addWidget( ListHypDefinition );
    hbox_2->addLayout( vbox );

    vbox_2 = new QVBoxLayout; 
    vbox_2->setSpacing( 6 );
    vbox_2->setMargin( 0 );

    TextHypAssignation = new QLabel( GroupHypotheses, "TextHypAssignation" );
    TextHypAssignation->setText( tr( "SMESH_EDIT_USED"  ) );
    vbox_2->addWidget( TextHypAssignation );

    ListHypAssignation = new QListBox( GroupHypotheses, "ListHypAssignation" );
    ListHypAssignation->setMinimumSize( 100, 50);
//      ListHypAssignation->setRowMode(4); 
//      ListHypAssignation->setRowMode( QListBox::FixedNumber );
//    ListHypAssignation->setLineWidth( 4 );
//      ListHypAssignation->setColumnMode( QListBox::Variable );
//      ListHypAssignation->setVariableHeight( FALSE );
//      ListHypAssignation->insertItem( tr( "New Item" ) );
    vbox_2->addWidget( ListHypAssignation );
    hbox_2->addLayout( vbox_2 );

    grid_3->addLayout( hbox_2, 0, 0 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupHypotheses, 2, 0 );

    GroupAlgorithms = new QGroupBox( this, "GroupAlgorithms" );
    GroupAlgorithms->setTitle( tr( "SMESH_ADD_ALGORITHM"  ) );
    GroupAlgorithms->setColumnLayout(0, Qt::Vertical );
    GroupAlgorithms->layout()->setSpacing( 0 );
    GroupAlgorithms->layout()->setMargin( 0 );
    grid_4 = new QGridLayout( GroupAlgorithms->layout() );
    grid_4->setGeometry( QRect( 12, 18, 139, 110 ) ); 
    grid_4->setAlignment( Qt::AlignTop );
    grid_4->setSpacing( 6 );
    grid_4->setMargin( 11 );

    hbox_3 = new QHBoxLayout; 
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );

    vbox_3 = new QVBoxLayout; 
    vbox_3->setSpacing( 6 );
    vbox_3->setMargin( 0 );

    TextAlgoDefinition = new QLabel( GroupAlgorithms, "TextAlgoDefinition" );
    TextAlgoDefinition->setText( tr( "SMESH_AVAILABLE"  ) );
    vbox_3->addWidget( TextAlgoDefinition );

    ListAlgoDefinition = new QListBox( GroupAlgorithms, "ListAlgoDefinition" );
    ListAlgoDefinition->setMinimumSize( 100, 50);
//      ListAlgoDefinition->setRowMode(4);
//      ListAlgoDefinition->setRowMode( QListBox::FixedNumber );
//    ListAlgoDefinition->setLineWidth( 4 );
//      ListAlgoDefinition->setColumnMode( QListBox::Variable );
//      ListAlgoDefinition->setVariableHeight( FALSE );
//      ListAlgoDefinition->insertItem( tr( "New Item" ) );
    vbox_3->addWidget( ListAlgoDefinition );
    hbox_3->addLayout( vbox_3 );

    vbox_4 = new QVBoxLayout; 
    vbox_4->setSpacing( 6 );
    vbox_4->setMargin( 0 );

    TextAlgoAssignation = new QLabel( GroupAlgorithms, "TextAlgoAssignation" );
    TextAlgoAssignation->setText( tr( "SMESH_EDIT_USED"  ) );
    vbox_4->addWidget( TextAlgoAssignation );

    ListAlgoAssignation = new QListBox( GroupAlgorithms, "ListAlgoAssignation" );
    ListAlgoAssignation ->setMinimumSize( 100, 50);
//      ListAlgoAssignation->setRowMode(4);
//      ListAlgoAssignation->setRowMode( QListBox::FixedNumber );
//    ListAlgoAssignation->setLineWidth( 4 );
//      ListAlgoAssignation->setColumnMode( QListBox::Variable );
//      ListAlgoAssignation->setVariableHeight( FALSE );
//      ListAlgoAssignation->insertItem( tr( "New Item" ) );
    vbox_4->addWidget( ListAlgoAssignation );
    hbox_3->addLayout( vbox_4 );

    grid_4->addLayout( hbox_3, 0, 0 );
    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupAlgorithms, 3, 0 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupC1, 1, 0 );
    /***************************************************************/

    Init(Sel) ;

}


//=================================================================================
// function : ~SMESHGUI_EditHypothesesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_EditHypothesesDlg::~SMESHGUI_EditHypothesesDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::Init( SALOME_Selection* Sel )
{
  GroupC1->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
  myEditCurrentArgument = LineEditC1A1 ;	
  mySelection = Sel;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  InitHypDefinition();
  InitAlgoDefinition();

  InitHypAssignation();
  InitAlgoAssignation();

  myGeomFilter = new SALOME_TypeFilter( "GEOM" );
  myMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );

  myGeomShape = GEOM::GEOM_Shape::_nil();
  myMesh = SMESH::SMESH_Mesh::_nil();
  mySubMesh = SMESH::SMESH_subMesh::_nil();

  /* signals and slots connections */
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );

  connect( SelectButtonC1A1, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A2, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( mySelection, SIGNAL( currentSelectionChanged() ),     this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
 
  connect( ListHypAssignation, SIGNAL( clicked(QListBoxItem*) ), this, SLOT( removeItem(QListBoxItem*) ) );
  connect( ListAlgoAssignation, SIGNAL( clicked(QListBoxItem*) ), this, SLOT( removeItem(QListBoxItem*) ) );
 
  connect( ListHypDefinition, SIGNAL( clicked(QListBoxItem*) ), this, SLOT( addItem(QListBoxItem*) ) );
  connect( ListAlgoDefinition, SIGNAL( clicked(QListBoxItem*) ), this, SLOT( addItem(QListBoxItem*) ) );

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
void SMESHGUI_EditHypothesesDlg::ConstructorsClicked(int constructorId)
{
  return ;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::ClickOnCancel()
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
void SMESHGUI_EditHypothesesDlg::SelectionIntoArgument()
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
	    myMesh = SMESH::SMESH_Mesh::_nil();
	    mySubMesh = SMESH::SMESH_subMesh::_nil();
	    InitHypAssignation();
	    InitAlgoAssignation();
	    InitGeom();
	    return ;
	  } else {
	    Standard_Boolean testResult ;
	    Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
	    myMesh = mySMESHGUI->ConvertIOinMesh(IO, testResult) ;
	    if( !testResult ) {
	      myMesh = SMESH::SMESH_Mesh::_nil();

	      mySubMesh = mySMESHGUI->ConvertIOinSubMesh(IO, testResult) ;
	      if( !testResult ) {
		mySubMesh = SMESH::SMESH_subMesh::_nil();
		InitHypAssignation();
		InitAlgoAssignation();
		InitGeom();
		return ;
	      }
	    }
	    InitHypAssignation();
	    InitAlgoAssignation();
	    InitGeom();
	  }
	} else if ( myEditCurrentArgument == LineEditC1A2 ) {
	  if ( nbSel != 1 ) {
	    myOkHypothesis = false;
	    myOkAlgorithm = false;
	    myGeomShape = GEOM::GEOM_Shape::_nil();
	    InitHypAssignation();
	    InitAlgoAssignation();
	    InitGeom();
	    return ;
	  } else {
	    Standard_Boolean testResult ;
	    Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
	    myGeomShape = mySMESHGUI->ConvertIOinGEOMShape(IO, testResult) ;
	    if( !testResult ) {
	      myGeomShape = GEOM::GEOM_Shape::_nil();
	      InitHypAssignation();
	      InitAlgoAssignation();
	      InitGeom();
	      return ;
	    }
	    InitHypAssignation();
	    InitAlgoAssignation();
	    InitGeom();	    
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
void SMESHGUI_EditHypothesesDlg::SetEditCurrentArgument()
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
	  mySelection->AddFilter(myMeshOrSubMeshFilter) ;
	} else if (send == SelectButtonC1A2) {
	  LineEditC1A2->setFocus() ;
	  myEditCurrentArgument = LineEditC1A2;
	  mySelection->ClearFilters() ;
	  mySelection->AddFilter(myGeomFilter) ;
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
void SMESHGUI_EditHypothesesDlg::DeactivateActiveDialog()
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
void SMESHGUI_EditHypothesesDlg::ActivateThisDialog()
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
void SMESHGUI_EditHypothesesDlg::enterEvent(QEvent* e)
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
void SMESHGUI_EditHypothesesDlg::closeEvent( QCloseEvent* e )
{
  this->ClickOnCancel() ;
  return ;
}

//=================================================================================
// function : TextChangedInLineEdit()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::TextChangedInLineEdit(const QString& newText)
{  
  QLineEdit* send = (QLineEdit*)sender();
  QString newT = strdup(newText) ;
  
  return ;
}

void SMESHGUI_EditHypothesesDlg::removeItem(QListBoxItem* i)
{
  if (!i) return;

  SMESHGUI_StudyAPI myStudyAPI = mySMESHGUI->GetStudyAPI();
  int index = ListHypAssignation->index( i );
  if ( index != -1 ) {
    if (mapNameIOR.find( string((const char*)(i->text())) ) != mapNameIOR.end()) {
      SMESH::SMESH_Hypothesis_var Hyp = 
	SMESH::SMESH_Hypothesis::_narrow( myStudyAPI.StringToIOR( mapNameIOR[ string((const char*)(i->text())) ].c_str() ) );
  
      if ( !myMesh->_is_nil() ) {
	SALOMEDS::SObject_var aMesh = myStudyAPI.FindMesh(myMesh);
	mySMESHGUI->RemoveHypothesisOrAlgorithmOnMesh(aMesh, Hyp);
	//	mySMESHGUI->GetStudyAPI().ModifiedMesh( aMesh, false );
	mySMESHGUI->GetActiveStudy()->updateObjBrowser();
      }
      if ( !mySubMesh->_is_nil() ) {
	SALOMEDS::SObject_var aSubMesh = myStudyAPI.FindSubMesh(mySubMesh);
	mySMESHGUI->RemoveHypothesisOrAlgorithmOnMesh(aSubMesh, Hyp);
	//	mySMESHGUI->GetStudyAPI().ModifiedMesh( aSubMesh, false );
	mySMESHGUI->GetActiveStudy()->updateObjBrowser();
      }
      
      ListHypAssignation->removeItem( index );
    }
    return;
  } 
  index = ListAlgoAssignation->index( i );
  if ( index != -1 ) {
    if (mapNameIOR.find( string((const char*)(i->text())) ) != mapNameIOR.end()) {
      SMESH::SMESH_Hypothesis_var Hyp = 
	SMESH::SMESH_Hypothesis::_narrow( myStudyAPI.StringToIOR(mapNameIOR[ string((const char*)(i->text())) ].c_str()) );
      
      if ( !myMesh->_is_nil() ) {
	SALOMEDS::SObject_var aMesh = myStudyAPI.FindMesh(myMesh);
	mySMESHGUI->RemoveHypothesisOrAlgorithmOnMesh(aMesh, Hyp);
	//	mySMESHGUI->GetStudyAPI().ModifiedMesh( aMesh, false );
	mySMESHGUI->GetActiveStudy()->updateObjBrowser();
      }
      if ( !mySubMesh->_is_nil() ) {
	SALOMEDS::SObject_var aSubMesh = myStudyAPI.FindSubMesh(mySubMesh);
	mySMESHGUI->RemoveHypothesisOrAlgorithmOnMesh(aSubMesh, Hyp);
	//	mySMESHGUI->GetStudyAPI().ModifiedMesh( aSubMesh, false );
	mySMESHGUI->GetActiveStudy()->updateObjBrowser();
      }

      ListAlgoAssignation->removeItem( index );
    } 
  }
}


//=================================================================================
// function : addItem()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::addItem(QListBoxItem* i)
{
  if (!i) return;

  SMESHGUI_StudyAPI myStudyAPI = mySMESHGUI->GetStudyAPI();
  if ( ListHypDefinition->findItem( i->text() ) ) {
    if ( !ListHypAssignation->findItem( i->text() ) ) {
      ListHypAssignation->insertItem( i->text() );
      
      if (mapNameIOR.find( string((const char*)(i->text())) ) != mapNameIOR.end()) {
	SMESH::SMESH_Hypothesis_var Hyp = 
	  SMESH::SMESH_Hypothesis::_narrow( myStudyAPI.StringToIOR(mapNameIOR[ string((const char*)(i->text())) ].c_str()) );
      
	if ( !myMesh->_is_nil() )
	  mySMESHGUI->AddHypothesisOnMesh(myMesh, Hyp);
	if ( !mySubMesh->_is_nil() )
	  mySMESHGUI->AddHypothesisOnSubMesh(mySubMesh, Hyp);
      }
    }
    return;
  }
  if ( ListAlgoDefinition->findItem( i->text() ) ) {
    if ( !ListAlgoAssignation->findItem( i->text() ) ) {
      ListAlgoAssignation->insertItem( i->text() );
      
      if (mapNameIOR.find( string((const char*)(i->text())) ) != mapNameIOR.end()) {
	SMESH::SMESH_Hypothesis_var Hyp = 
	  SMESH::SMESH_Hypothesis::_narrow( myStudyAPI.StringToIOR(mapNameIOR[ string((const char*)(i->text())) ].c_str()) );
      
	if ( !myMesh->_is_nil() )
	  mySMESHGUI->AddAlgorithmOnMesh(myMesh, Hyp);
	if ( !mySubMesh->_is_nil() )
	  mySMESHGUI->AddAlgorithmOnSubMesh(mySubMesh, Hyp);
      }
    }
  }
  mySMESHGUI->GetActiveStudy()->updateObjBrowser();
}


//=================================================================================
// function : InitHypDefinition()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitHypDefinition()
{
  SALOMEDS::SComponent_var father = mySMESHGUI->GetStudy()->FindComponent("MESH");
  SALOMEDS::SObject_var           HypothesisRoot;
  SALOMEDS::GenericAttribute_var  anAttr;
  SALOMEDS::AttributeName_var     aName;
  SALOMEDS::AttributeIOR_var      anIOR;

  int Tag_HypothesisRoot = 1;
  if (father->FindSubObject (1, HypothesisRoot)) {
    SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(HypothesisRoot);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName") ) {
	aName = SALOMEDS::AttributeName::_narrow(anAttr);
	ListHypDefinition->insertItem(aName->Value());
	
	if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
	  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  mapNameIOR[ aName->Value() ] = anIOR->Value();
	}
      }
    }
  }
}

//=================================================================================
// function : InitHypAssignation()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitHypAssignation()
{
  MESSAGE ( " InitHypAssignation " << myMesh->_is_nil() )
  MESSAGE ( " InitHypAssignation " << mySubMesh->_is_nil() )
  ListHypAssignation->clear();
  SMESHGUI_StudyAPI myStudyAPI = mySMESHGUI->GetStudyAPI();
  int Tag_RefOnAppliedHypothesis = 2;
  SALOMEDS::SObject_var             AHR, aRef;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;

  if ( !myMesh->_is_nil() ) {
    SALOMEDS::SObject_var aMesh = myStudyAPI.FindMesh( myMesh );
    if ( aMesh->FindSubObject (2, AHR)) {
      SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	      aName = SALOMEDS::AttributeName::_narrow(anAttr);
	    ListHypAssignation->insertItem(aName->Value());
	  }
	}
      }
    }
  }
  if ( !mySubMesh->_is_nil() ) {
    SALOMEDS::SObject_var aSubMesh = myStudyAPI.FindSubMesh( mySubMesh );
    if ( aSubMesh->FindSubObject (2, AHR)) {
      SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	    aName = SALOMEDS::AttributeName::_narrow(anAttr);
	    ListHypAssignation->insertItem(aName->Value());
	  }
	}
      }
    }    
  }
}

//=================================================================================
// function : InitAlgoDefinition()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitAlgoDefinition()
{
  SALOMEDS::SComponent_var father = mySMESHGUI->GetStudy()->FindComponent("MESH");
  SALOMEDS::SObject_var          AlgorithmsRoot;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributeIOR_var     anIOR;

  int Tag_AlgorithmsRoot = 2;
  if (father->FindSubObject (2, AlgorithmsRoot)) {
    SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(AlgorithmsRoot);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName") ) {
	aName = SALOMEDS::AttributeName::_narrow(anAttr);
	ListAlgoDefinition->insertItem(aName->Value());

	if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
	  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  mapNameIOR[ aName->Value() ] = anIOR->Value();
	}
      }
    }
  }
}


//=================================================================================
// function : InitAlgoAssignation()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitAlgoAssignation()
{
  MESSAGE ( " InitAlgoAssignation " << myMesh->_is_nil() )
  MESSAGE ( " InitAlgoAssignation " << mySubMesh->_is_nil() )
  ListAlgoAssignation->clear();
  SMESHGUI_StudyAPI myStudyAPI = mySMESHGUI->GetStudyAPI();
  int Tag_RefOnAppliedAlgorithms = 3;
  SALOMEDS::SObject_var             AHR, aRef;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  
  if ( !myMesh->_is_nil() ) {
    SALOMEDS::SObject_var aMesh = myStudyAPI.FindMesh( myMesh );
    if ( aMesh->FindSubObject (3, AHR) ) {
      SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	    aName = SALOMEDS::AttributeName::_narrow(anAttr);
	    ListAlgoAssignation->insertItem(aName->Value());
	  }
	}
      }
    }
  }
  if ( !mySubMesh->_is_nil() ) {
    SALOMEDS::SObject_var aSubMesh = myStudyAPI.FindSubMesh( mySubMesh );
    if ( aSubMesh->FindSubObject (3, AHR) ) {
      SALOMEDS::ChildIterator_var it = mySMESHGUI->GetStudy()->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	    aName = SALOMEDS::AttributeName::_narrow(anAttr);
	    ListAlgoAssignation->insertItem(aName->Value());
	  }
	}
      }
    }
  }
}

//=================================================================================
// function : InitGeom()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitGeom()
{
  LineEditC1A2->setText("") ;
  SMESHGUI_StudyAPI myStudyAPI = mySMESHGUI->GetStudyAPI();

  if ( !myMesh->_is_nil() ) {
    SALOMEDS::SObject_var aMesh = myStudyAPI.FindMesh( myMesh );
    if ( !aMesh->_is_nil() )
      myGeomShape = myStudyAPI.GetShapeOnMeshOrSubMesh(aMesh);
  }
  if ( !mySubMesh->_is_nil() ) {
    SALOMEDS::SObject_var aSubMesh = myStudyAPI.FindSubMesh( mySubMesh );
    if ( !aSubMesh->_is_nil() )
      myGeomShape = myStudyAPI.GetShapeOnMeshOrSubMesh(aSubMesh);
  }
  
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  if ( !myGeomShape->_is_nil() && (!myMesh->_is_nil() || !mySubMesh->_is_nil()) ) {
    SALOMEDS::SObject_var aSO = mySMESHGUI->GetStudy()->FindObjectIOR( myGeomShape->Name() );
    if ( !aSO->_is_nil() ) {
      if (aSO->FindAttribute(anAttr, "AttributeName") ) {
	aName = SALOMEDS::AttributeName::_narrow(anAttr);
	LineEditC1A2->setText( QString(aName->Value()) ) ;
      }
    }
  }
}
