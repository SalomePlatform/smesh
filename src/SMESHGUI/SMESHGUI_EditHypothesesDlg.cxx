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

#include "SMESHGUI_EditHypothesesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_HypothesesUtils.h"

#include "SALOME_ListIteratorOfListIO.hxx"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_WaitCursor.h"
#include "QAD_Operation.h"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

#include "utilities.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>

using namespace std;

//VRV: porting on Qt 3.0.5
#if QT_VERSION >= 0x030005
#include <qlistbox.h>
#endif
//VRV: porting on Qt 3.0.5

class ListBoxIOR : public QListBoxText
{
public:
  enum { RTTI_IOR = 1000 };

public:
  ListBoxIOR( QListBox* listbox, 
              const char* ior,
              const QString& text = QString::null)
  : QListBoxText( listbox, text ), myIOR( ior ) {}
  virtual ~ListBoxIOR() {};
  virtual int rtti() const { return RTTI_IOR; }
  const char* GetIOR() { return myIOR.c_str(); }

private:
  string myIOR;
};

#define ALLOW_CHANGE_SHAPE 0

int findItem( QListBox* listBox, const string& ior )
{
  for ( int i = 0; i < listBox->count(); i++ ) {
    if ( listBox->item( i )->rtti() == ListBoxIOR::RTTI_IOR ) {
      ListBoxIOR* anItem = ( ListBoxIOR* )( listBox->item( i ) );
      if ( anItem && ior == string( anItem->GetIOR() ) )
	return i;
    }
  }
  return -1;
}

//=================================================================================
// class    : SMESHGUI_EditHypothesesDlg()
// purpose  : Constructs a SMESHGUI_EditHypothesesDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_EditHypothesesDlg::SMESHGUI_EditHypothesesDlg( QWidget* parent, const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose ),
    myImportedMesh( false )
{
    QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));
    if ( !name )
        setName( "SMESHGUI_EditHypothesesDlg" );
    setCaption( tr( "SMESH_EDIT_HYPOTHESES"  ) );
    setSizeGripEnabled( TRUE );
    QGridLayout* SMESHGUI_EditHypothesesDlgLayout = new QGridLayout( this ); 
    SMESHGUI_EditHypothesesDlgLayout->setSpacing( 6 );
    SMESHGUI_EditHypothesesDlgLayout->setMargin( 11 );

    /***************************************************************/
    GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), this, "GroupC1" );
    GroupC1->setColumnLayout(0, Qt::Vertical );
    GroupC1->layout()->setSpacing( 0 );
    GroupC1->layout()->setMargin( 0 );
    QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
    GroupC1Layout->setAlignment( Qt::AlignTop );
    GroupC1Layout->setSpacing( 6 );
    GroupC1Layout->setMargin( 11 );

    TextLabelC1A1 = new QLabel( tr( "SMESH_OBJECT_MESHorSUBMESH" ), GroupC1, "TextLabelC1A1" );
    GroupC1Layout->addWidget( TextLabelC1A1, 0, 0 );
    SelectButtonC1A1 = new QPushButton( GroupC1, "SelectButtonC1A1" );
    SelectButtonC1A1->setPixmap( image0 );
    GroupC1Layout->addWidget( SelectButtonC1A1, 0, 1 );
    LineEditC1A1 = new QLineEdit( GroupC1, "LineEditC1A1" );
    LineEditC1A1->setReadOnly( true );
    GroupC1Layout->addWidget( LineEditC1A1, 0, 2 );

    TextLabelC1A2 = new QLabel( tr( "SMESH_OBJECT_GEOM" ), GroupC1, "TextLabelC1A2" );
    GroupC1Layout->addWidget( TextLabelC1A2, 1, 0 );
    SelectButtonC1A2 = new QPushButton( GroupC1, "SelectButtonC1A2" );
    SelectButtonC1A2->setPixmap( image0 );
    SelectButtonC1A2->setToggleButton( FALSE );
    GroupC1Layout->addWidget( SelectButtonC1A2, 1, 1 );
    LineEditC1A2 = new QLineEdit( GroupC1, "LineEditC1A2" );
    LineEditC1A2->setReadOnly( true );
    GroupC1Layout->addWidget( LineEditC1A2, 1, 2 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupC1, 0, 0 );

    /***************************************************************/
    GroupHypotheses = new QGroupBox( tr( "SMESH_HYPOTHESES" ), this, "GroupHypotheses" );
    GroupHypotheses->setColumnLayout(0, Qt::Vertical );
    GroupHypotheses->layout()->setSpacing( 0 );
    GroupHypotheses->layout()->setMargin( 0 );
    QGridLayout* grid_3 = new QGridLayout( GroupHypotheses->layout() );
    grid_3->setAlignment( Qt::AlignTop );
    grid_3->setSpacing( 6 );
    grid_3->setMargin( 11 );

    TextHypDefinition = new QLabel( tr( "SMESH_AVAILABLE" ), GroupHypotheses, "TextHypDefinition" );
    grid_3->addWidget( TextHypDefinition, 0, 0 );

    ListHypDefinition = new QListBox( GroupHypotheses, "ListHypDefinition" );
    ListHypDefinition->setMinimumSize( 100, 100 );
    grid_3->addWidget( ListHypDefinition, 1, 0 );

    TextHypAssignation = new QLabel( tr( "SMESH_EDIT_USED" ), GroupHypotheses, "TextHypAssignation" );
    grid_3->addWidget( TextHypAssignation, 0, 1 );

    ListHypAssignation = new QListBox( GroupHypotheses, "ListHypAssignation" );
    ListHypAssignation->setMinimumSize( 100, 100 );
    grid_3->addWidget( ListHypAssignation, 1, 1 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupHypotheses, 1, 0 );

    /***************************************************************/
    GroupAlgorithms = new QGroupBox( tr( "SMESH_ADD_ALGORITHM" ), this, "GroupAlgorithms" );
    GroupAlgorithms->setColumnLayout(0, Qt::Vertical );
    GroupAlgorithms->layout()->setSpacing( 0 );
    GroupAlgorithms->layout()->setMargin( 0 );
    QGridLayout* grid_4 = new QGridLayout( GroupAlgorithms->layout() );
    grid_4->setAlignment( Qt::AlignTop );
    grid_4->setSpacing( 6 );
    grid_4->setMargin( 11 );

    TextAlgoDefinition = new QLabel( tr( "SMESH_AVAILABLE" ), GroupAlgorithms, "TextAlgoDefinition" );
    grid_4->addWidget( TextAlgoDefinition, 0, 0 );

    ListAlgoDefinition = new QListBox( GroupAlgorithms, "ListAlgoDefinition" );
    ListAlgoDefinition->setMinimumSize( 100, 100 );
    grid_4->addWidget( ListAlgoDefinition, 1, 0 );

    TextAlgoAssignation = new QLabel( tr( "SMESH_EDIT_USED" ), GroupAlgorithms, "TextAlgoAssignation" );
    grid_4->addWidget( TextAlgoAssignation, 0, 1 );

    ListAlgoAssignation = new QListBox( GroupAlgorithms, "ListAlgoAssignation" );
    ListAlgoAssignation ->setMinimumSize( 100, 100 );
    grid_4->addWidget( ListAlgoAssignation, 1, 1 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupAlgorithms, 2, 0 );

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
    buttonOk->setDefault( FALSE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 0 );

    buttonApply = new QPushButton( tr( "SMESH_BUT_APPLY" ), GroupButtons, "buttonApply" );
    buttonApply->setAutoDefault( TRUE );
    buttonApply->setDefault( FALSE );
    GroupButtonsLayout->addWidget( buttonApply, 0, 1 );

    GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 2 );

    buttonCancel = new QPushButton( tr( "SMESH_BUT_CLOSE" ), GroupButtons, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    buttonCancel->setDefault( TRUE );
    buttonCancel->setEnabled( TRUE ) ;
    GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );

    SMESHGUI_EditHypothesesDlgLayout->addWidget( GroupButtons, 4, 0 );

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
  mySelection = Sel;
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  InitHypDefinition();
  InitAlgoDefinition();

  myGeomFilter = new SALOME_TypeFilter( "GEOM" );
  myMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );

  myGeomShape = GEOM::GEOM_Object::_nil();
  myMesh = SMESH::SMESH_Mesh::_nil();
  mySubMesh = SMESH::SMESH_subMesh::_nil();

  /* signals and slots connections */
  connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonApply,  SIGNAL( clicked() ), this, SLOT( ClickOnApply() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) );

  connect( SelectButtonC1A1, SIGNAL (clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectButtonC1A2, SIGNAL (clicked() ), this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( mySelection, SIGNAL( currentSelectionChanged() ),      this, SLOT( SelectionIntoArgument() ) );
  connect( mySMESHGUI,  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySMESHGUI,  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( ClickOnCancel() ) ) ;
 
  connect( ListHypAssignation,  SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( removeItem(QListBoxItem*) ) );
  connect( ListAlgoAssignation, SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( removeItem(QListBoxItem*) ) );
 
  connect( ListHypDefinition,  SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( addItem(QListBoxItem*) ) );
  connect( ListAlgoDefinition, SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( addItem(QListBoxItem*) ) );

  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; 

  LineEditC1A1->setFocus() ;
  myEditCurrentArgument = LineEditC1A1;
  mySelection->ClearFilters() ;   
  mySelection->AddFilter(myMeshOrSubMeshFilter) ;

  SelectionIntoArgument();

  UpdateControlState();
}


//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::ClickOnOk()
{
  if ( ClickOnApply() )
    ClickOnCancel() ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesDlg::ClickOnApply()
{
  if (mySMESHGUI->ActiveStudyLocked())
    return false;

  bool aRes = false;

  QAD_WaitCursor wc;

  QAD_Operation* op = new QAD_Operation( mySMESHGUI->GetActiveStudy() );

  // start transaction
  op->start();
  
  if ( !myMesh->_is_nil() )
    aRes = StoreMesh();
  else if ( !mySubMesh->_is_nil() )
    aRes = StoreSubMesh();

  if ( true/*aRes*/ ) // abort desynchronizes contents of a Study and a mesh on server
  {
    // commit transaction
    op->finish();
    InitHypAssignation();
    InitAlgoAssignation();
  }
  else
    // abort transaction
    op->abort();

  UpdateControlState();

  return aRes;
}


//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::ClickOnCancel()
{
  close();
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_EditHypothesesDlg::SelectionIntoArgument()
{
  QString aString = ""; 

  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelection, aString) ;
  
  if ( myEditCurrentArgument == LineEditC1A1 ) {
    if ( nbSel != 1 ) {
      myMesh      = SMESH::SMESH_Mesh::_nil();
      mySubMesh   = SMESH::SMESH_subMesh::_nil();
      aString     = "";
    } else {
      Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO) ;
      if(myMesh->_is_nil()){
	mySubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO) ;
	if(mySubMesh->_is_nil()){
	  aString = "";
	}
      }
    }
    myEditCurrentArgument->setText( aString );
    
    myGeomShape = GEOM::GEOM_Object::_nil(); // InitGeom() will try to retrieve a shape from myMesh or mySubMesh
    InitGeom();
    
    myImportedMesh = myGeomShape->_is_nil();
    
    InitHypAssignation();
    InitAlgoAssignation();
  }
  else if ( myEditCurrentArgument == LineEditC1A2 ) {
    if ( nbSel != 1 )
      myGeomShape = GEOM::GEOM_Object::_nil();
    else {
      Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject() ;
      myGeomShape = SMESH::IObjectToInterface<GEOM::GEOM_Object>(IO) ;
    }
    InitGeom();
  } 

  UpdateControlState();
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
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
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::DeactivateActiveDialog()
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
void SMESHGUI_EditHypothesesDlg::ActivateThisDialog()
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
void SMESHGUI_EditHypothesesDlg::enterEvent(QEvent* e)
{
  if ( !GroupC1->isEnabled() )
    ActivateThisDialog();
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::closeEvent( QCloseEvent* e )
{
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  mySelection->ClearFilters() ;
  QDialog::closeEvent( e );
}

//=======================================================================
//function : IsOld
//purpose  : 
//=======================================================================

bool SMESHGUI_EditHypothesesDlg::IsOld(QListBoxItem* hypItem)
{
  if ( hypItem->rtti() == ListBoxIOR::RTTI_IOR ) {
    ListBoxIOR* hyp = ( ListBoxIOR* ) hypItem;
    return (myMapOldHypos.find( hyp->GetIOR() ) != myMapOldHypos.end() ||
            myMapOldAlgos.find( hyp->GetIOR() ) != myMapOldAlgos.end() );
  }

  return false;
}

//=================================================================================
// function : removeItem()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::removeItem(QListBoxItem* item)
{
  const QObject* aSender = sender();

  if (!item) return;

  if ( aSender == ListHypAssignation ) {
    myNbModification += IsOld( item ) ? 1 : -1;
    ListHypAssignation->removeItem( ListHypAssignation->index( item ) );
  } 
  else if ( aSender == ListAlgoAssignation ) {
    myNbModification += IsOld( item ) ? 1 : -1;
    ListAlgoAssignation->removeItem( ListAlgoAssignation->index( item ) );
  }


  UpdateControlState();
}


//=================================================================================
// function : addItem()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::addItem(QListBoxItem* item)
{
  const QObject* aSender = sender();

  if (!item) return;

  ListBoxIOR* i = 0;
  if ( item->rtti() == ListBoxIOR::RTTI_IOR )
    i = (ListBoxIOR*)item;
  if (!i) return;
  
  bool isFound = false;

  if ( aSender == ListHypDefinition ) {
    for ( int j = 0, n = ListHypAssignation->count(); !isFound && j < n; j++ ) {
      if ( ListHypAssignation->item( j )->rtti() == ListBoxIOR::RTTI_IOR ) {
	ListBoxIOR* anItem = (ListBoxIOR*)ListHypAssignation->item( j );
	isFound = !strcmp( anItem->GetIOR(), i->GetIOR() );
      }
    }
    if ( !isFound )
      ListBoxIOR* anItem = new ListBoxIOR( ListHypAssignation, 
					   CORBA::string_dup( i->GetIOR() ), 
					   CORBA::string_dup( i->text().latin1() ) );
  }
  else if ( aSender == ListAlgoDefinition ) {
    for ( int j = 0, n = ListAlgoAssignation->count(); !isFound && j < n; j++ ) {
      if ( ListAlgoAssignation->item( j )->rtti() == ListBoxIOR::RTTI_IOR ) {
	ListBoxIOR* anItem = (ListBoxIOR*)ListAlgoAssignation->item( j );
	isFound = !strcmp( anItem->GetIOR(), i->GetIOR() );
      }
    }
    if ( !isFound )
      ListBoxIOR* anItem = new ListBoxIOR( ListAlgoAssignation, 
					   CORBA::string_dup( i->GetIOR() ), 
					   CORBA::string_dup( i->text().latin1() ) );
  }

  if ( !isFound )
    myNbModification += IsOld( item ) ? -1 : 1;

  UpdateControlState();
}


//=================================================================================
// function : InitHypDefinition()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::InitHypDefinition()
{
  ListHypDefinition->clear();

  SALOMEDS::SComponent_var father = SMESH::GetActiveStudyDocument()->FindComponent("SMESH");
  if ( father->_is_nil() )
    return;

  SALOMEDS::SObject_var           HypothesisRoot;
  SALOMEDS::GenericAttribute_var  anAttr;
  SALOMEDS::AttributeName_var     aName;
  SALOMEDS::AttributeIOR_var      anIOR;

  int Tag_HypothesisRoot = 1;
  if (father->FindSubObject (1, HypothesisRoot)) {
    SALOMEDS::ChildIterator_var it = SMESH::GetActiveStudyDocument()->NewChildIterator(HypothesisRoot);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName") ) {
        aName = SALOMEDS::AttributeName::_narrow(anAttr);
        if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
          anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
          ListBoxIOR* anItem = new ListBoxIOR( ListHypDefinition, 
                                               anIOR->Value(), 
                                               aName->Value() );
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
  myNbModification = 0;
//   MESSAGE ( " InitHypAssignation " << myMesh->_is_nil() )
//   MESSAGE ( " InitHypAssignation " << mySubMesh->_is_nil() )

  myMapOldHypos.clear();
  ListHypAssignation->clear();
  if ( myImportedMesh )
    return;

  SALOMEDS::SObject_var             aMorSM, AHR, aRef;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeIOR_var        anIOR;

  if ( !myMesh->_is_nil() )
    aMorSM = SMESH::FindSObject( myMesh );
  else if ( !mySubMesh->_is_nil() )
    aMorSM = SMESH::FindSObject( mySubMesh );

  if ( !aMorSM->_is_nil() && aMorSM->FindSubObject (2, AHR)) {
    SALOMEDS::ChildIterator_var it = SMESH::GetActiveStudyDocument()->NewChildIterator(AHR);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if ( Obj->ReferencedObject(aRef) ) {
        if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	  aName = SALOMEDS::AttributeName::_narrow(anAttr);
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    ListBoxIOR* anItem = new ListBoxIOR( ListHypAssignation, 
						 anIOR->Value(), 
						 aName->Value() );
	    myMapOldHypos[ anIOR->Value() ] = ListHypAssignation->index( anItem );
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
  ListAlgoDefinition->clear();

  SALOMEDS::SComponent_var father = SMESH::GetActiveStudyDocument()->FindComponent("SMESH");
  if ( father->_is_nil() )
    return;

  SALOMEDS::SObject_var          AlgorithmsRoot;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributeIOR_var     anIOR;

  if (father->FindSubObject (2, AlgorithmsRoot)) {
    SALOMEDS::ChildIterator_var it = SMESH::GetActiveStudyDocument()->NewChildIterator(AlgorithmsRoot);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if (Obj->FindAttribute(anAttr, "AttributeName") ) {
        aName = SALOMEDS::AttributeName::_narrow(anAttr);
        if (Obj->FindAttribute(anAttr, "AttributeIOR")) {
          anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
          ListBoxIOR* anItem = new ListBoxIOR( ListAlgoDefinition, 
                                               anIOR->Value(), 
                                               aName->Value() );
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

  myMapOldAlgos.clear();
  ListAlgoAssignation->clear();
  if ( myImportedMesh )
    return;

  SALOMEDS::SObject_var             aMorSM, AHR, aRef;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  SALOMEDS::AttributeIOR_var        anIOR;

  if ( !myMesh->_is_nil() )
    aMorSM = SMESH::FindSObject( myMesh );
  else if ( !mySubMesh->_is_nil() )
    aMorSM = SMESH::FindSObject( mySubMesh );

  if ( !aMorSM->_is_nil() && aMorSM->FindSubObject (3, AHR)) {
    SALOMEDS::ChildIterator_var it = SMESH::GetActiveStudyDocument()->NewChildIterator(AHR);
    for (; it->More();it->Next()) {
      SALOMEDS::SObject_var Obj = it->Value();
      if ( Obj->ReferencedObject(aRef) ) {
        if (aRef->FindAttribute(anAttr, "AttributeName") ) {
	  aName = SALOMEDS::AttributeName::_narrow(anAttr);
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    ListBoxIOR* anItem = new ListBoxIOR( ListAlgoAssignation, 
						 anIOR->Value(), 
						 aName->Value() );
	    myMapOldAlgos[ anIOR->Value() ] = ListAlgoAssignation->index( anItem );
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

  if ( myGeomShape->_is_nil() && !myMesh->_is_nil() ) {
    SALOMEDS::SObject_var aMesh = SMESH::FindSObject( myMesh );
    if ( !aMesh->_is_nil() )
      myGeomShape = SMESH::GetShapeOnMeshOrSubMesh(aMesh);
  }
  if ( myGeomShape->_is_nil() && !mySubMesh->_is_nil() ) {
    SALOMEDS::SObject_var aSubMesh = SMESH::FindSObject( mySubMesh );
    if ( !aSubMesh->_is_nil() )
      myGeomShape = SMESH::GetShapeOnMeshOrSubMesh(aSubMesh);
  }
  
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeName_var       aName;
  if ( !myGeomShape->_is_nil() && (!myMesh->_is_nil() || !mySubMesh->_is_nil()) ) {
    SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument();
    SALOMEDS::SObject_var aSO = aStudy->FindObjectIOR( aStudy->ConvertObjectToIOR(myGeomShape) );
    if ( !aSO->_is_nil() ) {
      if (aSO->FindAttribute(anAttr, "AttributeName") ) {
	aName = SALOMEDS::AttributeName::_narrow(anAttr);
	LineEditC1A2->setText( QString(aName->Value()) ) ;
      }
    }
  }
}

//=================================================================================
// function : UpdateControlState()
// purpose  :
//=================================================================================
void SMESHGUI_EditHypothesesDlg::UpdateControlState()
{
  bool isEnabled = ( !myMesh   ->_is_nil() && !myGeomShape->_is_nil() &&   ListHypAssignation->count() && ListAlgoAssignation->count() ) ||
                   ( !mySubMesh->_is_nil() && !myGeomShape->_is_nil() && ( ListHypAssignation->count() || ListAlgoAssignation->count() ) );

  buttonOk   ->setEnabled( myNbModification && isEnabled && !myImportedMesh );
  buttonApply->setEnabled( myNbModification && isEnabled && !myImportedMesh );

  SelectButtonC1A2   ->setEnabled( ALLOW_CHANGE_SHAPE && !myImportedMesh );
  LineEditC1A2       ->setEnabled( ALLOW_CHANGE_SHAPE && !myImportedMesh );
  ListHypDefinition  ->setEnabled( !myImportedMesh );
  ListHypAssignation ->setEnabled( !myImportedMesh );
  ListAlgoDefinition ->setEnabled( !myImportedMesh );
  ListAlgoAssignation->setEnabled( !myImportedMesh );
}

//=================================================================================
// function : StoreMesh()
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesDlg::StoreMesh()
{
  MapIOR anOldHypos, aNewHypos;
  if ( myGeomShape->_is_nil() )
    return false;
  // 1. Check whether the geometric shape has changed
  SALOMEDS::SObject_var aMeshSO = SMESH::FindSObject( myMesh );
  GEOM::GEOM_Object_var aIniGeomShape = SMESH::GetShapeOnMeshOrSubMesh( aMeshSO );
  bool bShapeChanged = aIniGeomShape->_is_nil() || !aIniGeomShape->_is_equivalent( myGeomShape );
  if ( bShapeChanged ) {
    // VSR : TODO : Set new shape - not supported yet by SMESH engine
    // 1. remove all old hypotheses and algorithms and also submeshes
    // 2. set new shape
  }

  int nbFail = 0;
  MapIOR::iterator it;
  // 2. remove not used hypotheses from the mesh
  for ( it = myMapOldHypos.begin(); it != myMapOldHypos.end(); ++it ) {
    string ior = it->first;
    int index = findItem( ListHypAssignation, ior );
    if ( index < 0 ) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
      if ( !aHyp->_is_nil() ){
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( aMeshSO, aHyp ))
	  nbFail++;
      }
    }
  }
  // 3. remove not used algorithms from the mesh
  for ( it = myMapOldAlgos.begin(); it != myMapOldAlgos.end(); ++it ) {
    string ior = it->first;
    int index = findItem( ListAlgoAssignation, ior );
    if ( index < 0 ) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
      if ( !aHyp->_is_nil() ){
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( aMeshSO, aHyp ))
	  nbFail++;
      }
    }
  }
  // 4. Add new algorithms
  for ( int i = 0; i < ListAlgoAssignation->count(); i++ ) {
    if ( ListAlgoAssignation->item( i )->rtti() == ListBoxIOR::RTTI_IOR ) {
      ListBoxIOR* anItem = ( ListBoxIOR* )( ListAlgoAssignation->item( i ) );
      if ( anItem ) {
	string ior = anItem->GetIOR();
	if ( myMapOldAlgos.find( ior ) == myMapOldAlgos.end() ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
	  if ( !aHyp->_is_nil() ){
	    if (!SMESH::AddHypothesisOnMesh( myMesh, aHyp ))
	      nbFail++;
	  }
	}
      }
    }
  }
  // 5. Add new hypotheses
  for ( int i = 0; i < ListHypAssignation->count(); i++ ) {
    if ( ListHypAssignation->item( i )->rtti() == ListBoxIOR::RTTI_IOR ) {
      ListBoxIOR* anItem = ( ListBoxIOR* )( ListHypAssignation->item( i ) );
      if ( anItem ) {
	string ior = anItem->GetIOR();
	if ( myMapOldHypos.find( ior ) == myMapOldHypos.end() ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
	  if ( !aHyp->_is_nil() ){
	    if (!SMESH::AddHypothesisOnMesh( myMesh, aHyp ))
	      nbFail++;
	  }
	}
      }
    }
  }
  return ( nbFail == 0 );
}

//=================================================================================
// function : StoreSubMesh()
// purpose  :
//=================================================================================
bool SMESHGUI_EditHypothesesDlg::StoreSubMesh()
{
  MapIOR anOldHypos, aNewHypos;
  if ( myGeomShape->_is_nil() )
    return false;
  // 1. Check whether the geometric shape has changed
  SALOMEDS::SObject_var aSubMeshSO = SMESH::FindSObject( mySubMesh );
  GEOM::GEOM_Object_var aIniGeomShape = SMESH::GetShapeOnMeshOrSubMesh( aSubMeshSO );
  bool bShapeChanged = aIniGeomShape->_is_nil() || !aIniGeomShape->_is_equivalent( myGeomShape );
  if ( bShapeChanged ) {
    // VSR : TODO : Set new shape - not supported yet by engine
    // 1. remove all old hypotheses and algorithms
    // 2. set new shape
  }
  int nbFail = 0;
  MapIOR::iterator it;
  // 2. remove not used hypotheses from the submesh
  for ( it = myMapOldHypos.begin(); it != myMapOldHypos.end(); ++it ) {
    string ior = it->first;
    int index = findItem( ListHypAssignation, ior );
    if ( index < 0 ) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
      if ( !aHyp->_is_nil() ){
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( aSubMeshSO, aHyp ))
	  nbFail++;
      }
    }
  }
  // 3. remove not used algorithms from the submesh
  for ( it = myMapOldAlgos.begin(); it != myMapOldAlgos.end(); ++it ) {
    string ior = it->first;
    int index = findItem( ListAlgoAssignation, ior );
    if ( index < 0 ) {
      SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
      if ( !aHyp->_is_nil() ){
	if (!SMESH::RemoveHypothesisOrAlgorithmOnMesh( aSubMeshSO, aHyp ))
	  nbFail++;
      }
    }
  }
  // 4. Add new algorithms
  for ( int i = 0; i < ListAlgoAssignation->count(); i++ ) {
    if ( ListAlgoAssignation->item( i )->rtti() == ListBoxIOR::RTTI_IOR ) {
      ListBoxIOR* anItem = ( ListBoxIOR* )( ListAlgoAssignation->item( i ) );
      if ( anItem ) {
	string ior = anItem->GetIOR();
	if ( myMapOldAlgos.find( ior ) == myMapOldAlgos.end() ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
	  if ( !aHyp->_is_nil() ){
	    if (!SMESH::AddHypothesisOnSubMesh( mySubMesh, aHyp ))
	      nbFail++;
	  }
	}
      }
    }
  }
  // 5. Add new hypotheses
  for ( int i = 0; i < ListHypAssignation->count(); i++ ) {
    if ( ListHypAssignation->item( i )->rtti() == ListBoxIOR::RTTI_IOR ) {
      ListBoxIOR* anItem = ( ListBoxIOR* )( ListHypAssignation->item( i ) );
      if ( anItem ) {
	string ior = anItem->GetIOR();
	if ( myMapOldHypos.find( ior ) == myMapOldHypos.end() ) {
	  SMESH::SMESH_Hypothesis_var aHyp = SMESH::IORToInterface<SMESH::SMESH_Hypothesis>(ior.c_str());
	  if ( !aHyp->_is_nil() ){
	    if (!SMESH::AddHypothesisOnSubMesh( mySubMesh, aHyp ))
	      nbFail++;
	  }
	}
      }
    }
  }
  return ( nbFail == 0 );
}
