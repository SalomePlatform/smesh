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
//  File   : SMESHGUI_MultiEditDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_MultiEditDlg.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_Filter.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include "QAD_Desktop.h"
#include "QAD_RightFrame.h"

#include "VTKViewer_ViewFrame.h"

#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"

#include "SALOME_Selection.h"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "VTKViewer_InteractorStyleSALOME.h"

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>

#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <Precision.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>

#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qapplication.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 5
#define MARGIN  10

/*
  Class       : SMESHGUI_MultiEditDlg
  Description : Description : Inversion of the diagonal of a pseudo-quadrangle formed by 
                2 neighboring triangles with 1 common edge
*/

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::SMESHGUI_MultiEditDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_MultiEditDlg::SMESHGUI_MultiEditDlg( QWidget*              theParent, 
                                              SALOME_Selection*     theSelection,
                                              const int             theMode,
                                              const char*           theName )
: QDialog( theParent, theName, false, 
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  myFilterDlg = 0;
  mySubmeshFilter = new SMESH_TypeFilter( SUBMESH );
  myGroupFilter = new SMESH_TypeFilter( GROUP );

  myFilterType = theMode;
  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  QFrame* aMainFrame = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );
  aDlgLay->setStretchFactor( aBtnFrame, 0 );
  Init( theSelection ) ;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_MultiEditDlg::createMainFrame( QWidget* theParent )
{
  QGroupBox* aMainGrp = new QGroupBox( 1, Qt::Horizontal, theParent );
  aMainGrp->setFrameStyle( QFrame::NoFrame );
  aMainGrp->setInsideMargin( 0 );

  QPixmap aPix( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr( "ICON_SELECT" ) ) );
  
  // "Selected cells" group
  mySelGrp = new QGroupBox( 1, Qt::Horizontal,  aMainGrp );
  QFrame* aFrame = new QFrame( mySelGrp );
  
  myListBox = new QListBox( aFrame );
  myListBox->setSelectionMode( QListBox::Extended );
  myListBox->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding) );
//  myListBox->setColumnMode( QListBox::FitToHeight );
  
  myFilterBtn = new QPushButton( tr( "FILTER" )   , aFrame );
  myAddBtn    = new QPushButton( tr( "ADD" )      , aFrame );
  myRemoveBtn = new QPushButton( tr( "REMOVE" )   , aFrame );
  mySortBtn   = new QPushButton( tr( "SORT_LIST" ), aFrame );

  QGridLayout* aLay = new QGridLayout( aFrame, 5, 2, 0, 5 );
  aLay->addMultiCellWidget( myListBox, 0, 4, 0, 0 );
  aLay->addWidget( myFilterBtn, 0, 1 );
  aLay->addWidget( myAddBtn, 1, 1 );
  aLay->addWidget( myRemoveBtn, 2, 1 );
  aLay->addWidget( mySortBtn, 3, 1 );
  
  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
  aLay->addItem( aSpacer, 4, 1 );
  
  myToAllChk = new QCheckBox( tr( "TO_ALL" ), mySelGrp );

  // "Select from" group
  QGroupBox* aGrp = new QGroupBox( 3, Qt::Horizontal, tr( "SELECT_FROM" ), aMainGrp );
  
  mySubmeshChk = new QCheckBox( tr( "SMESH_SUBMESH" ), aGrp );
  mySubmeshBtn = new QPushButton( aGrp );
  mySubmesh = new QLineEdit( aGrp );
  mySubmesh->setReadOnly( true );
  mySubmeshBtn->setPixmap( aPix );
  
  myGroupChk = new QCheckBox( tr( "GROUP" ), aGrp );
  myGroupBtn = new QPushButton( aGrp );
  myGroup = new QLineEdit( aGrp );
  myGroup->setReadOnly( true );
  myGroupBtn->setPixmap( aPix );

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_MultiEditDlg::createButtonFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  aFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

  myOkBtn     = new QPushButton( tr( "SMESH_BUT_OK"    ), aFrame );
  myApplyBtn  = new QPushButton( tr( "SMESH_BUT_APPLY" ), aFrame );
  myCloseBtn  = new QPushButton( tr( "SMESH_BUT_CLOSE" ), aFrame );

  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );

  QHBoxLayout* aLay = new QHBoxLayout( aFrame, MARGIN, SPACING );

  aLay->addWidget( myOkBtn );
  aLay->addWidget( myApplyBtn );
  aLay->addItem( aSpacer);
  aLay->addWidget( myCloseBtn );

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_MultiEditDlg::isValid( const bool /*theMess*/ ) const
{
  return (!myMesh->_is_nil() &&
          (myListBox->count() > 0 || (myToAllChk->isChecked() && myActor)));
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::~SMESHGUI_MultiEditDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_MultiEditDlg::~SMESHGUI_MultiEditDlg()
{
  if ( myFilterDlg != 0 )
  {
    myFilterDlg->reparent( 0, QPoint() );
    delete myFilterDlg;
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::Init( SALOME_Selection* theSelection )
{
  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  mySelection = theSelection;
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;
  myListBox->clear();
  myIds.Clear();
  myBusy = false;
  myActor = 0;
  emit ListContensChanged();

  // main buttons
  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myApplyBtn, SIGNAL( clicked() ), SLOT( onApply() ) );

  // selection and SMESHGUI
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  connect( aSMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );

  // dialog controls
  connect( myFilterBtn, SIGNAL( clicked() ), SLOT( onFilterBtn()   ) );
  connect( myAddBtn   , SIGNAL( clicked() ), SLOT( onAddBtn()      ) );
  connect( myRemoveBtn, SIGNAL( clicked() ), SLOT( onRemoveBtn()   ) );
  connect( mySortBtn  , SIGNAL( clicked() ), SLOT( onSortListBtn() ) );
  
  connect( mySubmeshChk, SIGNAL( stateChanged( int ) ), SLOT( onSubmeshChk() ) );
  connect( myGroupChk  , SIGNAL( stateChanged( int ) ), SLOT( onGroupChk()   ) );
  connect( myToAllChk  , SIGNAL( stateChanged( int ) ), SLOT( onToAllChk()   ) );

  connect( myListBox, SIGNAL( selectionChanged() ), SLOT( onListSelectionChanged() ) );

  onSelectionDone();

  // set selection mode
  setSelectionMode();
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onOk
// Purpose : SLOT called when "Ok" button pressed. 
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::onOk()
{
  if ( onApply() )
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::getIds
// Purpose : Retrive identifiers from list box
//=======================================================================
SMESH::long_array_var SMESHGUI_MultiEditDlg::getIds()
{
  SMESH::long_array_var anIds = new SMESH::long_array;
  
  if ( myToAllChk->isChecked() )
  {
    myIds.Clear();
    if ( myActor != 0 )
    {
      TVisualObjPtr aVisualObj = myActor->GetObject();
      vtkUnstructuredGrid* aGrid = aVisualObj->GetUnstructuredGrid();
      if ( aGrid != 0 )
      {
        for ( int i = 0, n = aGrid->GetNumberOfCells(); i < n; i++ )
        {
          vtkCell* aCell = aGrid->GetCell( i );
          if ( aCell != 0 )
          {
            int nbNodes = aCell->GetNumberOfPoints();
            if ( nbNodes == 3 && myFilterType == SMESHGUI_TriaFilter  ||
                 nbNodes == 4 && myFilterType == SMESHGUI_QuadFilter ||
                ( nbNodes == 4 || nbNodes ==  3 ) && myFilterType == SMESHGUI_UnknownFilter )
            {
              int anObjId = aVisualObj->GetElemObjId( i );
              myIds.Add( anObjId );
            }
          }
        }
      }
    }
  }

  anIds->length( myIds.Extent() );
  TColStd_MapIteratorOfMapOfInteger anIter( myIds );
  for ( int i = 0; anIter.More(); anIter.Next()  )
  {
    anIds[ i++ ] = anIter.Key();
  }
  return anIds._retn();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::onClose()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState();
  
  SMESH::RemoveFilter(SMESHGUI_EdgeFilter);
  SMESH::RemoveFilter(SMESHGUI_FaceFilter);
  SMESH::RemoveFilter(SMESHGUI_QuadFilter);
  SMESH::RemoveFilter(SMESHGUI_TriaFilter);
  SMESH::SetPickable();

  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  
  reject();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_MultiEditDlg::onSelectionDone()
{
  if ( myBusy || !isEnabled() ) return;
  myBusy = true;

  int nbSel = mySelection->IObjectCount();
  myListBox->clearSelection();

  if ( mySubmeshChk->isChecked() || myGroupChk->isChecked() )
  {
    QLineEdit* aNameEdit = mySubmeshChk->isChecked() ? mySubmesh : myGroup;
    int nbSel = mySelection->IObjectCount();
    if ( nbSel == 1 )
    {
      Handle(SALOME_InteractiveObject) anIO = mySelection->firstIObject();
      anIO.IsNull() ? aNameEdit->clear() : aNameEdit->setText( anIO->getName() );

      if ( mySubmeshChk->isChecked() )
      {
        SMESH::SMESH_subMesh_var aSubMesh =
          SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( anIO );
        if ( !aSubMesh->_is_nil() )
          myMesh = aSubMesh->GetFather();
      }
      else
      {
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( anIO );
        if ( !aGroup->_is_nil() )
          myMesh = aGroup->GetMesh();
      }
    }
    else if ( nbSel > 1 )
    {
      QString aStr = mySubmeshChk->isChecked() ? 
        tr( "SMESH_SUBMESH_SELECTED" ) : tr( "SMESH_GROUP_SELECTED" );
      aNameEdit->setText( aStr.arg( nbSel ) );
    }
    else
      aNameEdit->clear();
  }
  else if ( nbSel == 1 )
  {
    QString aListStr = "";
    int aNbItems = SMESH::GetNameOfSelectedElements(mySelection, aListStr);
    if ( aNbItems > 0 )
    {
      QStringList anElements = QStringList::split(" ", aListStr);
      QListBoxItem* anItem = 0;
      for ( QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it)
      {
        anItem = myListBox->findItem( *it, Qt::ExactMatch );
        if (anItem) myListBox->setSelected( anItem, true );
      }
    }

    myMesh = SMESH::GetMeshByIO( mySelection->firstIObject() );
  }

  if ( nbSel == 1 ) {
    myActor = SMESH::FindActorByEntry(mySelection->firstIObject()->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject( myMesh );
    VTKViewer_InteractorStyleSALOME* aStyle = SMESH::GetInteractorStyle();
    Handle(VTKViewer_Filter) aFilter1 = aStyle->GetFilter( myFilterType );
    Handle(VTKViewer_Filter) aFilter2 = aStyle->GetFilter( SMESHGUI_FaceFilter );
    if ( !aFilter1.IsNull() )
      aFilter1->SetActor( myActor );
    if ( !aFilter2.IsNull() )
      aFilter2->SetActor( myActor );
    if ( myActor )
      SMESH::SetPickable(myActor);
  }
  myBusy = false;

  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_MultiEditDlg::onDeactivate()
{
  setEnabled( false );
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_MultiEditDlg::enterEvent( QEvent* )
{
  if ( !isEnabled() ) {
    SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
    setEnabled( true );
    setSelectionMode();
  }
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MultiEditDlg::closeEvent( QCloseEvent* e )
{
  onClose() ;
}
//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_MultiEditDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onFilterBtn
// Purpose : SLOT. Called when "Filter" button pressed.
//           Start "Selection filters" dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::onFilterBtn()
{
  if ( myFilterDlg == 0 )
  {
    myFilterDlg = new SMESHGUI_FilterDlg( (QWidget*)parent(), SMESH::FACE );
    connect( myFilterDlg, SIGNAL( Accepted() ), SLOT( onFilterAccepted() ) );
  }
  else
    myFilterDlg->Init( SMESH::FACE );

  myFilterDlg->SetSelection( mySelection );
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( myListBox );

  myFilterDlg->show();
}

//=================================================================================
// function : onFilterAccepted()
// purpose  : SLOT. Called when Filter dlg closed with OK button.
//            Uncheck "Select submesh" and "Select group" checkboxes
//=================================================================================
void SMESHGUI_MultiEditDlg::onFilterAccepted()
{
  myIds.Clear();
  for ( int i = 0, n = myListBox->count(); i < n; i++ )
    myIds.Add( myListBox->text( i ).toInt() );

  emit ListContensChanged();

  if ( mySubmeshChk->isChecked() || myGroupChk->isChecked() )
  {
    mySubmeshChk->blockSignals( true );
    myGroupChk->blockSignals( true );
    mySubmeshChk->setChecked( false );
    myGroupChk->setChecked( false );
    mySubmeshChk->blockSignals( false );
    myGroupChk->blockSignals( false );
  }
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onAddBtn
// Purpose : Verify whether Id of element satisfies to filters from viewer
//=======================================================================
bool SMESHGUI_MultiEditDlg::isIdValid( const int theId ) const
{
  VTKViewer_InteractorStyleSALOME* aStyle = SMESH::GetInteractorStyle();
  Handle(SMESHGUI_Filter) aFilter1 =
    Handle(SMESHGUI_Filter)::DownCast( aStyle->GetFilter( myFilterType ) );
  Handle(SMESHGUI_Filter) aFilter2 =
    Handle(SMESHGUI_Filter)::DownCast( aStyle->GetFilter( SMESHGUI_FaceFilter ) );
  return ( aFilter1.IsNull() || aFilter1->IsObjValid( theId ) ) &&
         ( aFilter2.IsNull() || aFilter2->IsObjValid( theId ) );
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onAddBtn
// Purpose : SLOT. Called when "Add" button pressed.
//           Add selected in viewer entities in list box
//=======================================================================
void SMESHGUI_MultiEditDlg::onAddBtn()
{
  int nbSelected = mySelection->IObjectCount();
  if ( nbSelected == 0 ) 
    return;

  TColStd_IndexedMapOfInteger toBeAdded;
  
  if ( !mySubmeshChk->isChecked() && !myGroupChk->isChecked() ) 
  {
    if ( nbSelected == 1 ) 
      SMESH::GetSelected( mySelection, toBeAdded );
  }
  else if ( mySubmeshChk->isChecked() ) 
  {
    SALOME_ListIteratorOfListIO anIter( mySelection->StoredIObjects() );
    for ( ; anIter.More(); anIter.Next() )
    {
      SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( anIter.Value() );
      if ( !aSubMesh->_is_nil() )
      {
        if ( aSubMesh->GetFather()->GetId() == myMesh->GetId() )
        {
          SMESH::long_array_var anIds = aSubMesh->GetElementsId();
          for ( int i = 0, n = anIds->length(); i < n; i++ )
          {
            if ( isIdValid( anIds[ i ] ) )
              toBeAdded.Add( anIds[ i ] );
          }
        }
      }
    }
  }
  else if ( myGroupChk->isChecked() ) 
  {
    SALOME_ListIteratorOfListIO anIter( mySelection->StoredIObjects() );
    for ( ; anIter.More(); anIter.Next() )
    {
      SMESH::SMESH_GroupBase_var aGroup =
        SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( anIter.Value() );
      if ( !aGroup->_is_nil() && aGroup->GetType() == SMESH::FACE )
      {
        if ( aGroup->GetMesh()->GetId() == myMesh->GetId() )
        {
          SMESH::long_array_var anIds = aGroup->GetListOfID();
          for ( int i = 0, n = anIds->length(); i < n; i++ )
          {
            if ( isIdValid( anIds[ i ] ) )
              toBeAdded.Add( anIds[ i ] );
          }
        }
      }
    }
  }

  myBusy = true;
  bool isGroupOrSubmesh = ( mySubmeshChk->isChecked() || myGroupChk->isChecked() );
  mySubmeshChk->setChecked( false );
  myGroupChk->setChecked( false );
  for( int i = 1; i <= toBeAdded.Extent(); i++ )
    if ( myIds.Add( toBeAdded(i) ) ) {
      QListBoxItem * item = new QListBoxText( QString( "%1" ).arg( toBeAdded(i) ));
      myListBox->insertItem( item );
      myListBox->setSelected( item, true );
    }
  myBusy = false;

  emit ListContensChanged(); 

  if ( isGroupOrSubmesh )
    onListSelectionChanged();

  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::updateButtons
// Purpose : Enable/disable buttons of dialog in accordance with current state
//=======================================================================
void SMESHGUI_MultiEditDlg::updateButtons()
{
  bool isOk = isValid(false);
  myOkBtn->setEnabled( isOk );
  myApplyBtn->setEnabled( isOk );

  bool isListBoxNonEmpty = myListBox->count() > 0;
  bool isToAll = myToAllChk->isChecked();
  myFilterBtn->setEnabled( !isToAll );
  myRemoveBtn->setEnabled( isListBoxNonEmpty && !isToAll );
  mySortBtn->setEnabled( isListBoxNonEmpty &&!isToAll );
  
  if ( isToAll ||
       myMesh->_is_nil() ||
       mySelection->IObjectCount() != 1 ||
       (SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( mySelection->firstIObject() )->_is_nil() &&
	SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( mySelection->firstIObject() )->_is_nil() &&
	SMESH::IObjectToInterface<SMESH::SMESH_Mesh>( mySelection->firstIObject() )->_is_nil()) )
    myAddBtn->setEnabled( false );
  else
    myAddBtn->setEnabled( true );
  
  mySubmeshChk->setEnabled( !isToAll );
  mySubmeshBtn->setEnabled( mySubmeshChk->isChecked() );
  mySubmesh->setEnabled( mySubmeshChk->isChecked() );
  
  myGroupChk->setEnabled( !isToAll );
  myGroupBtn->setEnabled( myGroupChk->isChecked() );
  myGroup->setEnabled( myGroupChk->isChecked() );
  
  if ( !mySubmeshChk->isChecked() )
    mySubmesh->clear();
  if ( !myGroupChk->isChecked() )
    myGroup->clear();
    
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onRemoveBtn
// Purpose : SLOT. Called when "Remove" button pressed.
//           Remove selected in list box entities
//=======================================================================
void SMESHGUI_MultiEditDlg::onRemoveBtn()
{
  myBusy = true;
  
  for ( int i = 0, n = myListBox->count(); i < n; i++ )
  {
    for ( int i = myListBox->count(); i > 0; i--) {
      if ( myListBox->isSelected( i - 1 ) ) 
      {
        int anId = myListBox->text( i - 1 ).toInt();
        myIds.Remove( anId );
        myIds.Remove( anId );
	      myListBox->removeItem( i-1 );
      }
    }        
  }
  myBusy = false;

  emit ListContensChanged();  
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onSortListBtn
// Purpose : SLOT. Called when "Sort list" button pressed.
//           Sort entities of list box
//=======================================================================
void SMESHGUI_MultiEditDlg::onSortListBtn()
{
  myBusy = true;

  int i, k = myListBox->count();
  if ( k > 0 ) 
  {
    QStringList aSelected;
    std::vector<int> anArray( k );
    QListBoxItem* anItem;
    for ( anItem = myListBox->firstItem(), i = 0; anItem != 0; anItem = anItem->next(), i++) 
    {
      anArray[ i ] = anItem->text().toInt();
      if ( anItem->isSelected() ) 
        aSelected.append( anItem->text() );
    }
    
    std::sort( anArray.begin(), anArray.end() );
    myListBox->clear();
    for ( i = 0; i < k; i++ ) 
      myListBox->insertItem( QString::number( anArray[ i ] ) );

    for ( QStringList::iterator it = aSelected.begin(); it != aSelected.end(); ++it ) 
    {
      anItem = myListBox->findItem( *it, Qt::ExactMatch );
      if ( anItem ) 
        myListBox->setSelected( anItem, true );
    }
  }
  myBusy = false;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onListSelectionChanged
// Purpose : SLOT. Called when selection in list box changed.
//           Highlight in selected entities
//=======================================================================
void SMESHGUI_MultiEditDlg::onListSelectionChanged()
{
  if ( myActor == 0 || myBusy )
    return;
  
  if ( mySubmeshChk->isChecked() || myGroupChk->isChecked() ) 
    return;

  SMESH_Actor * anActor = SMESH::FindActorByObject( myMesh );
  if ( !anActor )
    anActor = myActor;
  TVisualObjPtr anObj = anActor->GetObject();

  TColStd_MapOfInteger anIndexes;
  for ( QListBoxItem* anItem = myListBox->firstItem(); anItem != 0; anItem = anItem->next() ) 
  {
    if ( anItem->isSelected() ) 
    {
      int anId = anItem->text().toInt();
      if ( anObj->GetElemVTKId( anId ) >= 0 ) // avoid exception in hilight
        anIndexes.Add(anId);
    }
  }
  
  mySelection->ClearIObjects();
  mySelection->AddOrRemoveIndex( anActor->getIO(), anIndexes, false, false );
  mySelection->AddIObject( anActor->getIO() );
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onSubmeshChk
// Purpose : SLOT. Called when state of "SubMesh" check box changed.
//           Activate/deactivate selection of submeshes
//=======================================================================
void SMESHGUI_MultiEditDlg::onSubmeshChk()
{
  bool isChecked = mySubmeshChk->isChecked();
  mySubmeshBtn->setEnabled( isChecked );
  mySubmesh->setEnabled( isChecked );
  if ( !isChecked )
    mySubmesh->clear();
  if ( isChecked && myGroupChk->isChecked() )
      myGroupChk->setChecked( false );
      
  setSelectionMode();      
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onGroupChk
// Purpose : SLOT. Called when state of "Group" check box changed.
//           Activate/deactivate selection of groupes
//=======================================================================
void SMESHGUI_MultiEditDlg::onGroupChk()
{
  bool isChecked = myGroupChk->isChecked();
  myGroupBtn->setEnabled( isChecked );
  myGroup->setEnabled( isChecked );
  if ( !isChecked )
    myGroup->clear();
  if ( isChecked && mySubmeshChk->isChecked() )
      mySubmeshChk->setChecked( false );

  setSelectionMode();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onToAllChk
// Purpose : SLOT. Called when state of "Apply to all" check box changed.
//           Activate/deactivate selection 
//=======================================================================
void SMESHGUI_MultiEditDlg::onToAllChk()
{
  bool isChecked = myToAllChk->isChecked();

  if ( isChecked )
    myListBox->clear();

  myIds.Clear();

  emit ListContensChanged();
    
  updateButtons();
  setSelectionMode();

  if ( myActor )
    mySelection->AddIObject( myActor->getIO(), true );
}


//=======================================================================
// name    : SMESHGUI_MultiEditDlg::setSelectionMode
// Purpose : Set selection mode
//=======================================================================
void SMESHGUI_MultiEditDlg::setSelectionMode()
{
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  
  if ( mySubmeshChk->isChecked() )
  {
    QAD_Application::getDesktop()->SetSelectionMode( ActorSelection, true );
    mySelection->AddFilter( mySubmeshFilter );
  }
  else if ( myGroupChk->isChecked() )
  {
    QAD_Application::getDesktop()->SetSelectionMode( ActorSelection, true );
    mySelection->AddFilter( myGroupFilter );
  }
  else
  {
    QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
    if ( myFilterType == SMESHGUI_TriaFilter )
      SMESH::SetFilter( new SMESHGUI_TriangleFilter() );
    else if ( myFilterType == SMESHGUI_QuadFilter )
      SMESH::SetFilter( new SMESHGUI_QuadrangleFilter() );
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onApply
// Purpose : SLOT. Called when "Apply" button clicked. 
//=======================================================================
bool SMESHGUI_MultiEditDlg::onApply()
{
  if ( SMESHGUI::GetSMESHGUI()->ActiveStudyLocked() )
    return false;
  if ( !isValid( true ) )
    return false;

  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
  if ( aMeshEditor->_is_nil()  )
    return false;

  SMESH::long_array_var anIds = getIds();

  bool aResult = process( aMeshEditor, anIds.inout() );
  if ( aResult )
  {
    if ( myActor )
    {
      mySelection->ClearIObjects();
      SMESH::UpdateView();
      mySelection->AddIObject( myActor->getIO(), false );
    }

    myListBox->clear();
    myIds.Clear();
    emit ListContensChanged();

    updateButtons();
  }

  return aResult;
}

/*
  Class       : SMESHGUI_ChangeOrientationDlg
  Description : Modification of orientation of faces
*/

SMESHGUI_ChangeOrientationDlg::SMESHGUI_ChangeOrientationDlg( QWidget*          theParent, 
                                                              SALOME_Selection* theSelection,
                                                              const char*       theName )
: SMESHGUI_MultiEditDlg( theParent, theSelection, SMESHGUI_UnknownFilter, theName )
{
  setCaption( tr( "CAPTION" ) );
}

SMESHGUI_ChangeOrientationDlg::~SMESHGUI_ChangeOrientationDlg()
{
}

bool SMESHGUI_ChangeOrientationDlg::process( SMESH::SMESH_MeshEditor_ptr theEditor,
                                             const SMESH::long_array&    theIds )
{
  return theEditor->Reorient( theIds );
}

/*
  Class       : SMESHGUI_UnionOfTrianglesDlg
  Description : Construction of quadrangles by automatic association of triangles
*/

SMESHGUI_UnionOfTrianglesDlg::SMESHGUI_UnionOfTrianglesDlg( QWidget*          theParent,
                                                            SALOME_Selection* theSelection,
                                                            const char*       theName )
: SMESHGUI_MultiEditDlg( theParent, theSelection, SMESHGUI_TriaFilter, theName )
{
  setCaption( tr( "CAPTION" ) );
}

SMESHGUI_UnionOfTrianglesDlg::~SMESHGUI_UnionOfTrianglesDlg()
{
}

bool SMESHGUI_UnionOfTrianglesDlg::process( SMESH::SMESH_MeshEditor_ptr theEditor,
                                            const SMESH::long_array&    theIds )
{
  return theEditor->TriToQuad(theIds, SMESH::NumericalFunctor::_nil(), 1. );
}

/*
  Class       : SMESHGUI_CuttingOfQuadsDlg
  Description : Construction of quadrangles by automatic association of triangles
*/

SMESHGUI_CuttingOfQuadsDlg::SMESHGUI_CuttingOfQuadsDlg( QWidget*          theParent,
                                                        SALOME_Selection* theSelection,
                                                        const char*       theName )
: SMESHGUI_MultiEditDlg( theParent, theSelection, SMESHGUI_QuadFilter, theName )
{

  setCaption( tr( "CAPTION" ) );
  myPreviewActor = 0;

  myUseDiagChk = new QCheckBox( tr( "USE_DIAGONAL_2_4" ), mySelGrp );
  myPreviewChk = new QCheckBox( tr( "PREVIEW" ), mySelGrp );

  connect( myPreviewChk, SIGNAL( stateChanged( int ) ), this, SLOT( onPreviewChk() ) );
  connect( myUseDiagChk, SIGNAL( stateChanged( int ) ), this, SLOT( onPreviewChk() ) );
  connect( this, SIGNAL( ListContensChanged() ), this, SLOT( onPreviewChk() ) );
}

SMESHGUI_CuttingOfQuadsDlg::~SMESHGUI_CuttingOfQuadsDlg()
{
}

void SMESHGUI_CuttingOfQuadsDlg::onClose()
{
  erasePreview();
  SMESHGUI_MultiEditDlg::onClose();
}

bool SMESHGUI_CuttingOfQuadsDlg::process( SMESH::SMESH_MeshEditor_ptr theEditor,
                                          const SMESH::long_array&    theIds )
{
  return theEditor->SplitQuad( theIds, !myUseDiagChk->isChecked() );
}

void SMESHGUI_CuttingOfQuadsDlg::onPreviewChk()
{
  myPreviewChk->isChecked() ? displayPreview() : erasePreview();
}

void SMESHGUI_CuttingOfQuadsDlg::erasePreview()
{
  if ( myPreviewActor == 0 )
    return;
    
  if ( VTKViewer_ViewFrame* vf = SMESH::GetCurrentVtkView() )
  {
    vf->RemoveActor(myPreviewActor);
    vf->Repaint();
  }
  myPreviewActor->Delete();
  myPreviewActor = 0;
}

void SMESHGUI_CuttingOfQuadsDlg::displayPreview()
{
  if ( myActor == 0 )
    return;

  if ( myPreviewActor != 0 )
    erasePreview();

  // get Ids of elements
  SMESH::long_array_var anElemIds = getIds();
  if ( getIds()->length() == 0 )
    return;

  SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh();
  if ( aMesh == 0 )
    return;

  bool isDiag24 = myUseDiagChk->isChecked();

  //Create grid
  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();
    
  vtkIdType aNbCells = anElemIds->length() * 2;
  vtkIdType aCellsSize = 4 * aNbCells;
  vtkCellArray* aConnectivity = vtkCellArray::New();
  aConnectivity->Allocate( aCellsSize, 0 );

  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( anElemIds->length() * 4 );
  
  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents( 1 );
  aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );

  vtkIdList *anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds( 3 );

  TColStd_DataMapOfIntegerInteger anIdToVtk;

  int aNodes[ 4 ];
  int nbPoints = -1;
  for ( int i = 0, n = anElemIds->length(); i < n; i++ )
  {
    const SMDS_MeshElement* anElem = aMesh->FindElement( anElemIds[ i ] );
    if ( anElem == 0 || anElem->NbNodes() != 4 )
      continue;
      
    SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
    int k = 0;
    while( anIter->more() )
      if ( const SMDS_MeshNode* aNode = (SMDS_MeshNode*)anIter->next() )
      {
        if ( !anIdToVtk.IsBound( aNode->GetID() ) )
        {
          aPoints->SetPoint( ++nbPoints, aNode->X(), aNode->Y(), aNode->Z() );
          anIdToVtk.Bind( aNode->GetID(), nbPoints );
        }
        
        aNodes[ k++ ] = aNode->GetID();
      }

    if ( k != 4 )
      continue;

    if ( !isDiag24 )
    {
      anIdList->SetId( 0, anIdToVtk( aNodes[ 0 ] ) );
      anIdList->SetId( 1, anIdToVtk( aNodes[ 1 ] ) );
      anIdList->SetId( 2, anIdToVtk( aNodes[ 2 ] ) );
      aConnectivity->InsertNextCell( anIdList );
      aCellTypesArray->InsertNextValue( VTK_TRIANGLE );

      anIdList->SetId( 0, anIdToVtk( aNodes[ 2 ] ) );
      anIdList->SetId( 1, anIdToVtk( aNodes[ 3 ] ) );
      anIdList->SetId( 2, anIdToVtk( aNodes[ 0 ] ) );
      aConnectivity->InsertNextCell( anIdList );
      aCellTypesArray->InsertNextValue( VTK_TRIANGLE );
    }
    else
    {
      anIdList->SetId( 0, anIdToVtk( aNodes[ 1 ] ) );
      anIdList->SetId( 1, anIdToVtk( aNodes[ 2 ] ) );
      anIdList->SetId( 2, anIdToVtk( aNodes[ 3 ] ) );
      aConnectivity->InsertNextCell( anIdList );
      aCellTypesArray->InsertNextValue( VTK_TRIANGLE );

      anIdList->SetId( 0, anIdToVtk( aNodes[ 3 ] ) );
      anIdList->SetId( 1, anIdToVtk( aNodes[ 0 ] ) );
      anIdList->SetId( 2, anIdToVtk( aNodes[ 1 ] ) );
      aConnectivity->InsertNextCell( anIdList );
      aCellTypesArray->InsertNextValue( VTK_TRIANGLE );
    }
  }

  vtkIntArray* aCellLocationsArray = vtkIntArray::New();
  aCellLocationsArray->SetNumberOfComponents( 1 );
  aCellLocationsArray->SetNumberOfTuples( aNbCells );

  aConnectivity->InitTraversal();
  for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
    aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );

  aGrid->SetPoints( aPoints );
  aGrid->SetCells( aCellTypesArray, aCellLocationsArray,aConnectivity );

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInput( aGrid );
  
  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->PickableOff();
  myPreviewActor->SetMapper( aMapper );

  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor( 250, 0, 250 );
  aProp->SetLineWidth( myActor->GetLineWidth() + 1 );
  myPreviewActor->SetProperty( aProp );

  SMESH::GetCurrentVtkView()->AddActor( myPreviewActor );
  SMESH::GetCurrentVtkView()->Repaint();

  aProp->Delete();
  aPoints->Delete();
  aConnectivity->Delete();
  aGrid->Delete();
  aMapper->Delete();
  anIdList->Delete();
  aCellTypesArray->Delete();
  aCellLocationsArray->Delete();
}























