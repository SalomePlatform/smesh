// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File   : StdMeshersGUI_SubShapeSelectorWdg.cxx
// Author : Open CASCADE S.A.S. (dmv)
// SMESH includes
//
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

// SMESH Includes
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMESH_Gen_i.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMESH_PreviewActorsCollection.h"
#include "SMESH_Type.h"

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_Selector.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

// GEOM Includes
#include <GEOMBase.h>
#include <GEOM_TypeFilter.h>
#include <GEOM_CompoundFilter.h>

// Qt includes
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>


#define SPACING 6
#define MARGIN 0

//================================================================================
/*!
 *  Constructor
 */
//================================================================================

StdMeshersGUI_SubShapeSelectorWdg
::StdMeshersGUI_SubShapeSelectorWdg( QWidget *        parent,
                                     TopAbs_ShapeEnum subShType,
                                     const bool       toShowList ):
  QWidget( parent ),
  myMaxSize( -1 ),
  myPreviewActor( 0 )
{
  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );

  QGridLayout* edgesLayout = new QGridLayout( this );
  edgesLayout->setMargin( MARGIN );
  edgesLayout->setSpacing( SPACING );

  if ( toShowList )
  {
    myListWidget   = new QListWidget( this );
    myAddButton    = new QPushButton( tr( "SMESH_BUT_ADD" ),    this );
    myRemoveButton = new QPushButton( tr( "SMESH_BUT_REMOVE" ), this );
    myListWidget->setSelectionMode( QListWidget::ExtendedSelection );
    myListWidget->setMinimumWidth(300);
  }
  else
  {
    myListWidget   = 0;
    myAddButton    = 0;
    myRemoveButton = 0;
  }
  myInfoLabel    = new QLabel( this );
  myPrevButton   = new QPushButton( "<<", this );
  myNextButton   = new QPushButton( ">>", this );

  if ( myListWidget )
  {
    edgesLayout->addWidget(myListWidget,   0, 0, 3, 3);
    edgesLayout->addWidget(myAddButton,    0, 3);
    edgesLayout->addWidget(myRemoveButton, 1, 3);
    edgesLayout->addWidget(myInfoLabel,    3, 0, 1, 3);
    edgesLayout->addWidget(myPrevButton,   4, 0);
    edgesLayout->addWidget(myNextButton,   4, 2);

    edgesLayout->setRowStretch(2, 5);
    edgesLayout->setColumnStretch(1, 5);
  }
  else // show only Prev and Next buttons
  {
    edgesLayout->addWidget(myInfoLabel,    0, 0, 1, 2);
    edgesLayout->addWidget(myPrevButton,   1, 0);
    edgesLayout->addWidget(myNextButton,   1, 1);
  }
  //myInfoLabel->setMinimumWidth(300);
  //myInfoLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  myInfoLabel->setAlignment(Qt::AlignCenter);

  mySubShType = subShType;

  init();
}

//================================================================================
/*!
 *  Destructor
 */
//================================================================================

StdMeshersGUI_SubShapeSelectorWdg::~StdMeshersGUI_SubShapeSelectorWdg()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
    if ( myPreviewActor ) {
      myPreviewActor->RemoveFromRender( myRenderer );
      aViewWindow->Repaint();

      delete myPreviewActor;
      myPreviewActor = 0;
    }
  }
  myEntry = "";
  myParamValue = "";
  myMainShape.Nullify();

  if ( mySelectionMgr && myFilter )
    mySelectionMgr->removeFilter( myFilter );
  delete myFilter; myFilter=0;

  mySelectionMgr->clearSelected();

  SUIT_SelectionFilter* filter;
  foreach( filter, myGeomFilters )
    delete filter;
}

//================================================================================
/*!
 *  Create a layout, initialize fields
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::init()
{
  myParamValue = "";
  myListOfIDs.clear();
  mySelectedIDs.clear();

  mySMESHGUI     = SMESHGUI::GetSMESHGUI();
  mySelectionMgr = SMESH::GetSelectionMgr( mySMESHGUI );
  mySelector     = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );

  myFilter=0;

  if ( myListWidget )
  {
    myAddButton->setEnabled( false );
    myRemoveButton->setEnabled( false );

    connect( myListWidget,   SIGNAL(itemSelectionChanged()),    this, SLOT(onListSelectionChanged()));
    connect( myAddButton,    SIGNAL(clicked()), SLOT(onAdd()));
    connect( myRemoveButton, SIGNAL(clicked()), SLOT(onRemove()));
  }
  connect( myPrevButton,   SIGNAL(clicked()), SLOT(onPrevious()));
  connect( myNextButton,   SIGNAL(clicked()), SLOT(onNext()));

  connect( mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(selectionIntoArgument()));

  updateState();
}

//================================================================================
/*!
 * \brief Install filters to select sub-shapes of mySubShType or their groups
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::setFilter()
{
  // if ( !myFilter )
  // {
  //   SalomeApp_Study* study = mySMESHGUI->activeStudy();
  //   GEOM_TypeFilter* typeFilter = new GEOM_TypeFilter(study, mySubShType, /*isShapeType=*/true );
  //   GEOM_CompoundFilter* gpoupFilter = new GEOM_CompoundFilter(study);
  //   gpoupFilter->addSubType( mySubShType );
  //   myGeomFilters.append( typeFilter );
  //   myGeomFilters.append( gpoupFilter );
  //   myFilter = new SMESH_LogicalFilter( myGeomFilters, SMESH_LogicalFilter::LO_OR );
  // }
  // mySelectionMgr->installFilter( myFilter );
}

//================================================================================
/*!
 *  Create a layout, initialize fields
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::ShowPreview( bool visible)
{
  if ( !myPreviewActor )
    return;

  if ( myIsShown != visible ) {
    myPreviewActor->SetShown( visible );
    
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->Repaint();

    myIsShown = visible;
  }
}

//================================================================================
/*!
 * \brief Connect/disconnect to change of selection
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::ActivateSelection( bool toActivate )
{
  if ( !mySelectionMgr ) return;

  if ( toActivate )
  {
    connect( mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(selectionIntoArgument()));
  }
  else
  {
    disconnect(mySelectionMgr, 0, this, 0 );
  }
}

//================================================================================
/*!
 * \brief Clears selected IDs. This is a workaround of a bug that
 *        SUIT_SelectionMgr::clearSelected() does not emit currentSelectionChanged
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::ClearSelected()
{
  mySelectedIDs.clear();
  selectionIntoArgument();
}

//=================================================================================
// function : selectionIntoArgument()
// purpose  : Called when selection has changed or in other cases
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::selectionIntoArgument()
{
  if ( !myPreviewActor )
    return;

  mySelectedIDs.clear();

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  int nbSel = aList.Extent();

  if (nbSel > 0) {
    SALOME_ListIteratorOfListIO anIt (aList);

    for ( ; anIt.More(); anIt.Next()) // Loop on selected objects
    {
      Handle(SALOME_InteractiveObject) IO = anIt.Value();

      GEOM::GEOM_Object_var aGeomObj = GetGeomObjectByEntry( IO->getEntry() );
      if ( !CORBA::is_nil( aGeomObj ) ) { // Selected Object From Study
        // commented for IPAL52836
        //
        // GEOM::GEOM_Object_var aGeomFatherObj = aGeomObj->GetMainShape();
        // QString aFatherEntry = "";
        // QString aMainFatherEntry = "";
        // TopoDS_Shape shape;
        // if ( !CORBA::is_nil( aGeomFatherObj ) ) {
        //   // Get Main Shape
        //   GEOM::GEOM_Object_var aGeomMain = GetGeomObjectByEntry( myEntry.c_str() );
        //   if ( !CORBA::is_nil( aGeomMain ) && aGeomMain->GetType() == 37 ) {  // Main Shape is a Group
        //     GEOM::GEOM_Object_var aMainFatherObj = aGeomMain->GetMainShape();
        //     if ( !CORBA::is_nil( aMainFatherObj ) )
        //       aMainFatherEntry = aMainFatherObj->GetStudyEntry();
        //   }
        //   aFatherEntry = aGeomFatherObj->GetStudyEntry();
        // }

        // if (( ! aFatherEntry.isEmpty() ) &&
        //     ( aFatherEntry == myEntry.c_str() || aFatherEntry == aMainFatherEntry ) )
        {
          TopoDS_Shape shape;
          if ( aGeomObj->GetType() == 37 /*GEOM_GROUP*/ ) { // Selected Group that belongs the main object
            GEOMBase::GetShape(aGeomObj, shape);
            if ( !shape.IsNull() ) {
              TopExp_Explorer exp( shape, mySubShType );
              for ( ; exp.More(); exp.Next() ) {
                int index = myPreviewActor->GetIndexByShape( exp.Current() );
                if ( index ) {
                  mySelectedIDs.append( index );
                  myPreviewActor->HighlightID( index );
                }
              }
            }
          } else if ( aGeomObj->GetType() == 28 /*GEOM_SUBSHAPE*/ ||
                      myEntry == IO->getEntry() )
          {
            GEOMBase::GetShape(aGeomObj, shape);
            if ( !shape.IsNull() && shape.ShapeType() == mySubShType ) {
              int index = myPreviewActor->GetIndexByShape( shape );
              if ( index ) {
                mySelectedIDs.append( index );
                myPreviewActor->HighlightID( index );
              }
            }
          }
        }
      } else { // Selected Actor from Actor Collection
        QString anEntry = IO->getEntry();
        QString str = "_";
        int index = anEntry.lastIndexOf( str );
        anEntry.remove(0, index+1);
        int ind = anEntry.toInt();
        if ( ind )
          mySelectedIDs.append( ind );
      }
    }
  }
  // update add button
  if ( myListWidget )
  {
    myAddButton->setEnabled(( myListWidget->count() < myMaxSize || myMaxSize == -1 ) &&
                            ( mySelectedIDs.size() > 0                             ) &&
                            ( mySelectedIDs.size() <= myMaxSize || myMaxSize == -1 ) );

    //Connect Selected Ids in viewer and dialog's Ids list
    bool signalsBlocked = myListWidget->blockSignals( true );
    myListWidget->clearSelection();
    if ( mySelectedIDs.size() > 0 ) {
      for (int i = 0; i < mySelectedIDs.size(); i++) {
        QString anID = QString(" %1").arg( mySelectedIDs.at(i) );
        QList<QListWidgetItem*> anItems = myListWidget->findItems ( anID, Qt::MatchExactly );
        QListWidgetItem* item;
        foreach(item, anItems)
          item->setSelected(true);
      }
    }
    myListWidget->blockSignals( signalsBlocked );
  }

  emit shapeSelected();
}

//=================================================================================
// function : onAdd()
// purpose  : Called when Add Button Clicked
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onAdd()
{
  if ( mySelectedIDs.size() < 1 || !myListWidget )
    return;

  myListWidget->blockSignals( true );
  for (int i = 0; i < mySelectedIDs.size() && (myMaxSize == -1 || myListOfIDs.size() < myMaxSize); i++) {
    if ( myListOfIDs.indexOf( mySelectedIDs.at(i) ) == -1 ) {
      QString anID = QString(" %1").arg( mySelectedIDs.at(i) );

      QListWidgetItem* anItem = new QListWidgetItem( anID, myListWidget );
      anItem->setSelected(true);
      
      myListOfIDs.append( mySelectedIDs.at(i) );
    }
  }
  onListSelectionChanged();
  myListWidget->blockSignals( false );

  mySelectedIDs.clear();
  myAddButton->setEnabled( false );
}
         
//=================================================================================
// function : onRemove()
// purpose  : Called when Remove Button Clicked
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onRemove()
{
  if ( myListWidget->count() < 1 || !myListWidget )
    return;

  myListWidget->blockSignals( true );
  QList<QListWidgetItem*> selItems = myListWidget->selectedItems();
  QListWidgetItem* item;
  foreach(item, selItems) {
    QString idStr = item->text();
    int id = idStr.toInt();

    int index = myListOfIDs.indexOf( id );
    myListOfIDs.removeAt( index );
    delete item;
  }

  onListSelectionChanged();
  myListWidget->blockSignals( false );
  
  myAddButton->setEnabled( !mySelectedIDs.isEmpty() );
}

void StdMeshersGUI_SubShapeSelectorWdg::onPrevious()
{
  if ( myPreviewActor ) {
    myPreviewActor->previous();
    if ( myListWidget )
      myListWidget->clearSelection();
    updateButtons();
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->Repaint();
  }
}

void StdMeshersGUI_SubShapeSelectorWdg::onNext()
{
  if ( myPreviewActor ) {
    myPreviewActor->next();
    if ( myListWidget )
      myListWidget->clearSelection();
    updateButtons();
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->Repaint();
  }
}

//=================================================================================
// function : onListSelectionChanged()
// purpose  : Called when selection in element list is changed
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onListSelectionChanged()
{
  if ( !myPreviewActor )
    return;

  myPreviewActor->HighlightAll( false );
  QList<QListWidgetItem*> selItems = myListWidget->selectedItems();
  QListWidgetItem* anItem;
  foreach(anItem, selItems)
    myPreviewActor->HighlightID( anItem->text().toInt() );

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->Repaint();

  // update remove button
  myRemoveButton->setEnabled( selItems.size() > 0 );

  emit selectionChanged();
}

//=================================================================================
// function : setGeomShape
// purpose  : Called to set geometry whose sub-shapes are selected
//================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::SetGeomShapeEntry( const QString& theEntry,
                                                           const QString& theMainShapeEntry )
{
  if ( !theEntry.isEmpty() || theMainShapeEntry.isEmpty() )
  {
    myParamValue = theEntry;
    myEntry      = theEntry.toStdString();
    myMainEntry  = theMainShapeEntry.toStdString();

    if ( myMainEntry.empty() ) myMainEntry = myEntry;
    if ( myEntry.empty() )     myEntry     = myMainEntry;
    if ( myMainEntry.length() > myEntry.length() &&
         theMainShapeEntry.startsWith( theEntry ))
      std::swap( myMainEntry, myEntry );

    myGeomShape = GetTopoDSByEntry( myEntry.c_str() );
    if ( myEntry == myMainEntry )
      myMainShape = myGeomShape;
    else
      myMainShape = GetTopoDSByEntry( myMainEntry.c_str() );
    updateState();
  }
}

//=================================================================================
// function : updateState
// purpose  : update Widget state
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::updateState()
{
  bool state = ( !myGeomShape.IsNull() );

  myInfoLabel ->setVisible( false );
  myPrevButton->setVisible( false );
  myNextButton->setVisible( false );

  if ( myListWidget )
  {
    myListWidget->setEnabled( state );
    myAddButton->setEnabled( mySelectedIDs.size() > 0 );
  }
  if ( state ) {
    SUIT_OverrideCursor wc;
    if ( !myPreviewActor )
      myPreviewActor = new SMESH_PreviewActorsCollection();
    myPreviewActor->SetSelector( mySelector );
    myPreviewActor->Init( myGeomShape, myMainShape, mySubShType, myEntry.c_str() );
    myPreviewActor->SetShown( false );
    myIsShown = false;
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
      myRenderer = aViewWindow->getRenderer();
      myPreviewActor->AddToRender( myRenderer );
      aViewWindow->Repaint();
    }
    updateButtons();
  }
}

//=================================================================================
// function : GetGeomObjectByEntry
// purpose  : Called to get GeomObject
//=================================================================================
GEOM::GEOM_Object_var StdMeshersGUI_SubShapeSelectorWdg::GetGeomObjectByEntry( const QString& theEntry )
{
  GEOM::GEOM_Object_var aGeomObj;
  SALOMEDS::Study_var aStudy = SMESHGUI::GetSMESHGen()->GetCurrentStudy();
  if ( !aStudy->_is_nil() )
  {
    SALOMEDS::SObject_var aSObj = aStudy->FindObjectID( theEntry.toLatin1().data() );
    if (!aSObj->_is_nil() )
    {
      CORBA::Object_var obj = aSObj->GetObject();
      aGeomObj = GEOM::GEOM_Object::_narrow(obj);
      aSObj->UnRegister();
    }
  }
  return aGeomObj._retn();
}

//=================================================================================
// function : setObjectByEntry
// purpose  : Called to get GeomObject
//=================================================================================
TopoDS_Shape StdMeshersGUI_SubShapeSelectorWdg::GetTopoDSByEntry( const QString& theEntry )
{
  TopoDS_Shape shape;
  GEOM::GEOM_Object_var aGeomObj = GetGeomObjectByEntry( theEntry );
  GEOMBase::GetShape(aGeomObj, shape);
  return shape;
}

//=================================================================================
// function : GetListOfIds
// purpose  : Called to get the list of SubShapes IDs
//=================================================================================
SMESH::long_array_var StdMeshersGUI_SubShapeSelectorWdg::GetListOfIDs()
{
  SMESH::long_array_var anArray = new SMESH::long_array;

  int size = myListOfIDs.size();
  anArray->length( size );
  for (int i = 0; i < size; i++)
    anArray[i] = myListOfIDs.at(i);

  return anArray;
}

//=================================================================================
// function : SetListOfIds
// purpose  : Called to set the list of SubShapes IDs. Returns false if any ID is invalid
//=================================================================================
bool StdMeshersGUI_SubShapeSelectorWdg::SetListOfIDs( SMESH::long_array_var theIds)
{
  mySelectedIDs.clear();
  myListOfIDs.clear();
  int size = theIds->length();
  for ( int i = 0; i < size; i++ )
    mySelectedIDs.append( theIds[ i ] );

  if ( myListWidget )
  {
    myListWidget->blockSignals( true );
    myListWidget->clear();
    myListWidget->blockSignals( false );
  }

  bool isOk = true;
  if ( myPreviewActor )
  {
    for ( int i = 0; i < size && isOk; i++ )
      isOk = myPreviewActor->IsValidIndex( theIds[ i ] );
  }
  else if ( !myMainShape.IsNull() )
  {
    TopTools_IndexedMapOfShape aMainMap;
    TopExp::MapShapes(myMainShape, aMainMap);
    for ( int i = 0; i < size && isOk; i++ )
      isOk = ( theIds[ i ] > 0 && theIds[ i ] <= aMainMap.Extent() );
  }
  onAdd();
  return isOk;
}

//=================================================================================
// function : GetMainShapeEntry
// purpose  : Called to get the Main Object Entry
//=================================================================================
const char* StdMeshersGUI_SubShapeSelectorWdg::GetMainShapeEntry()
{
  if ( myMainEntry.empty() ) myMainEntry = "";
  return myMainEntry.c_str();
}

void StdMeshersGUI_SubShapeSelectorWdg::updateButtons()
{
  if ( myPreviewActor ) {
    int total = myPreviewActor->count();
    int chunk = myPreviewActor->currentChunk();
    int chunkSize = myPreviewActor->chunkSize();
    int imin = chunk*chunkSize+1;
    int imax = std::min((chunk+1)*chunkSize, total);
    bool vis = imax > 0 && total > chunkSize;
    myInfoLabel->setVisible( vis );
    myPrevButton->setVisible( vis );
    myNextButton->setVisible( vis );
    myInfoLabel->setText( tr( "X_FROM_Y_ITEMS_SHOWN" ).arg(imin).arg(imax).arg(total) );
    myPrevButton->setEnabled( myPreviewActor->hasPrevious() );
    myNextButton->setEnabled( myPreviewActor->hasNext() );
  }
}
