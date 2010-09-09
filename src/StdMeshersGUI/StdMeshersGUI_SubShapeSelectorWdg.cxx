//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File   : StdMeshersGUI_SubShapeSelectorWdg.cxx
// Author : Open CASCADE S.A.S. (dmv)
// SMESH includes
//
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

// SMESH Includes
#include <SMESH_Type.h>
#include "SMESHGUI_MeshUtils.h"
#include <SMESH_Actor.h>
#include <SMESH_PreviewActorsCollection.h>
#include <SMESH_ActorUtils.h>
#include "SMESHGUI_GroupUtils.h"
#include "SMESH_Gen_i.hxx"
#include "SMESHGUI_GEOMGenUtils.h"

// SVTK Includes
#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_Selector.h>

// SALOME GUI includes
#include <SALOME_ListIO.hxx>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIteratorOfListIO.hxx>

// SUIT Includes
#include <SUIT_ResourceMgr.h>

// GEOM Includes
#include <GEOMBase.h>

// Qt includes
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>

#define SPACING 6
#define MARGIN 0

//================================================================================
/*!
 *  Constructor
 */
//================================================================================

StdMeshersGUI_SubShapeSelectorWdg
::StdMeshersGUI_SubShapeSelectorWdg( QWidget * parent ): 
  QWidget( parent ),
  myPreviewActor( 0 ),
  myMaxSize( -1 )
{
  QPixmap image0( SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap( "SMESH", tr( "ICON_SELECT" ) ) );

  QGridLayout* edgesLayout = new QGridLayout( this );
  edgesLayout->setMargin( MARGIN );
  edgesLayout->setSpacing( SPACING );
  
  myListWidget    = new QListWidget( this );
  myAddButton    = new QPushButton( tr( "SMESH_BUT_ADD" ),    this );
  myRemoveButton = new QPushButton( tr( "SMESH_BUT_REMOVE" ), this );      
  myListWidget->setSelectionMode( QListWidget::ExtendedSelection );

  edgesLayout->addWidget(myListWidget,   0, 0, 3, 3);
  edgesLayout->addWidget(myAddButton,    0, 4);
  edgesLayout->addWidget(myRemoveButton, 1, 4);

  edgesLayout->setRowStretch(2, 5);
  edgesLayout->setColumnStretch(2, 5);

  setLayout( edgesLayout );
  setMinimumWidth( 300 );

  mySubShType = TopAbs_EDGE;

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
}

//================================================================================
/*!
 *  Create a layout, initialize fields
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::init()
{
  myParamValue = "";
  myIsNotCorrected = true; // to dont call the GetCorrectedValue method twice
  myListOfIDs.clear();
  mySelectedIDs.clear();

  myAddButton->setEnabled( false );
  myRemoveButton->setEnabled( false );

  mySMESHGUI     = SMESHGUI::GetSMESHGUI();
  mySelectionMgr = SMESH::GetSelectionMgr( mySMESHGUI );
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );

  connect( myAddButton,    SIGNAL(clicked()), SLOT(onAdd()));
  connect( myRemoveButton, SIGNAL(clicked()), SLOT(onRemove()));
  
  connect( mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  connect( myListWidget,   SIGNAL(itemSelectionChanged()),    this, SLOT(onListSelectionChanged()));

  updateState();
}

//================================================================================
/*!
 *  Create a layout, initialize fields
 */
//================================================================================

void StdMeshersGUI_SubShapeSelectorWdg::showPreview( bool visible)
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

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::SelectionIntoArgument()
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
    
    for ( ; anIt.More(); anIt.Next()) { // Loop on selected objects
      Handle(SALOME_InteractiveObject) IO = anIt.Value();
      
      GEOM::GEOM_Object_var aGeomObj = GetGeomObjectByEntry( IO->getEntry() );  
      if ( !CORBA::is_nil( aGeomObj ) ) { // Selected Object From Study
        GEOM::GEOM_Object_ptr aGeomFatherObj = aGeomObj->GetMainShape();
        QString aFatherEntry = "";
        QString aMainFatherEntry = "";
        TopoDS_Shape shape;
        if ( !CORBA::is_nil( aGeomFatherObj ) ) {
          // Get Main Shape
          GEOM::GEOM_Object_var aGeomMain = GetGeomObjectByEntry( myEntry );
          if ( !CORBA::is_nil( aGeomMain ) && aGeomMain->GetType() == 37 ) {  // Main Shape is a Group
            GEOM::GEOM_Object_ptr aMainFatherObj = aGeomMain->GetMainShape();
            if ( !CORBA::is_nil( aMainFatherObj ) )
              aMainFatherEntry = aMainFatherObj->GetStudyEntry();
          }
          aFatherEntry = aGeomFatherObj->GetStudyEntry();
        }
        
        if ( aFatherEntry != "" && ( aFatherEntry == myEntry || aFatherEntry == aMainFatherEntry ) ) {
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
          } else if ( aGeomObj->GetType() == 28 /*GEOM_SUBSHAPE*/  ) {
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
  myAddButton->setEnabled( ( myListWidget->count() < myMaxSize || myMaxSize == -1 ) && mySelectedIDs.size() > 0 && ( mySelectedIDs.size() <= myMaxSize || myMaxSize == -1 ) );

  //Connect Selected Ids in viewer and dialog's Ids list
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
}

//=================================================================================
// function : onAdd()
// purpose  : Called when Add Button Clicked
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onAdd()
{
  if ( mySelectedIDs.size() < 1 )
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
  myAddButton->setEnabled( myMaxSize == -1 || myListOfIDs.size() < myMaxSize );
}
         
//=================================================================================
// function : onRemove()
// purpose  : Called when Remove Button Clicked
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onRemove()
{
  if ( myListWidget->count() < 1 )
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
  
  myAddButton->setEnabled( true );
}

//=================================================================================
// function : onListSelectionChanged()
// purpose  : Called when selection in element list is changed
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::onListSelectionChanged()
{
  if ( !myPreviewActor )
    return;

  mySelectionMgr->clearSelected();
  TColStd_MapOfInteger aIndexes;
  QList<QListWidgetItem*> selItems = myListWidget->selectedItems();
  QListWidgetItem* anItem;
  foreach(anItem, selItems)
    myPreviewActor->HighlightID( anItem->text().toInt() );

  // update remove button
  myRemoveButton->setEnabled( selItems.size() > 0 );
}

//=================================================================================
// function : setGeomShape
// purpose  : Called to set geometry
//================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::SetGeomShapeEntry( const QString& theEntry )
{
  if ( theEntry != "") {
    myParamValue = theEntry;
    myEntry = theEntry;
    myGeomShape = GetTopoDSByEntry( theEntry );
    updateState();
    myIsNotCorrected = true;
  }
}

//=================================================================================
// function : updateState
// purpose  : update Widget state
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::updateState()
{
  bool state = false;
  if ( !myGeomShape.IsNull() )
    state = true;
  
  myListWidget->setEnabled( state );
  myAddButton->setEnabled( mySelectedIDs.size() > 0 );
  
  if (state) {
    myPreviewActor = new SMESH_PreviewActorsCollection();
    myPreviewActor->SetSelector( mySelector );
    myPreviewActor->Init( myGeomShape, mySubShType, myEntry );
    myPreviewActor->SetShown( false );
    myIsShown = false;
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
      myRenderer = aViewWindow->getRenderer();
      myPreviewActor->AddToRender( myRenderer );
      aViewWindow->Repaint();
    }
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
  if (aStudy != 0) {
    SALOMEDS::SObject_var aSObj = aStudy->FindObjectID( theEntry.toLatin1().data() );
    SALOMEDS::GenericAttribute_var anAttr;

    if (!aSObj->_is_nil() && aSObj->FindAttribute(anAttr, "AttributeIOR")) {
      SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
      CORBA::String_var aVal = anIOR->Value();
      CORBA::Object_var obj = aStudy->ConvertIORToObject(aVal);
      aGeomObj = GEOM::GEOM_Object::_narrow(obj);
    }
  }
  return aGeomObj;
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

  if ( myMainEntry != "" && myIsNotCorrected )
    myListOfIDs = GetCorrectedListOfIDs( true );

  int size = myListOfIDs.size();
  anArray->length( size );
  if ( size ) {
    for (int i = 0; i < size; i++) {
        anArray[i] = myListOfIDs.at(i);
    }
  }
  return anArray;
}

//=================================================================================
// function : SetListOfIds
// purpose  : Called to set the list of SubShapes IDs
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::SetListOfIDs( SMESH::long_array_var theIds)
{
  mySelectedIDs.clear();
  myListOfIDs.clear();
  int size = theIds->length();
  for ( int i = 0; i < size; i++ )
    mySelectedIDs.append( theIds[ i ] );

  mySelectedIDs = GetCorrectedListOfIDs( false );
  onAdd();
}

//=================================================================================
// function : SetMainShapeEntry
// purpose  : Called to set the Main Object Entry
//=================================================================================
void StdMeshersGUI_SubShapeSelectorWdg::SetMainShapeEntry( const QString& theEntry )
{
  myMainEntry = theEntry;
  myMainShape = GetTopoDSByEntry( theEntry );
  myIsNotCorrected = true;
}

//=================================================================================
// function : GetMainShapeEntry
// purpose  : Called to get the Main Object Entry
//=================================================================================
const char* StdMeshersGUI_SubShapeSelectorWdg::GetMainShapeEntry()
{
  if ( myMainEntry == "")
    return myEntry.toLatin1().data();

  return myMainEntry.toLatin1().data();
}

//=================================================================================
// function : GetCorrectedListOfIds
// purpose  : Called to convert the list of IDs from subshape IDs to main shape IDs
//=================================================================================
QList<int> StdMeshersGUI_SubShapeSelectorWdg::GetCorrectedListOfIDs( bool fromSubshapeToMainshape )
{
  if ( ( myMainShape.IsNull() || myGeomShape.IsNull() ) &&  fromSubshapeToMainshape )
    return myListOfIDs;
  else   if ( ( myMainShape.IsNull() || myGeomShape.IsNull() ) &&  !fromSubshapeToMainshape )
    return mySelectedIDs;

  QList<int> aList;
  TopTools_IndexedMapOfShape   aGeomMap;
  TopTools_IndexedMapOfShape   aMainMap;
  TopExp::MapShapes(myGeomShape, aGeomMap);
  TopExp::MapShapes(myMainShape, aMainMap);

  if ( fromSubshapeToMainshape ) { // convert indexes from subshape to mainshape
    int size = myListOfIDs.size();
    for (int i = 0; i < size; i++) {
      TopoDS_Shape aSubShape = aGeomMap.FindKey( myListOfIDs.at(i) );
      int index = aMainMap.FindIndex( aSubShape );
      aList.append( index );
    }
    myIsNotCorrected = false;
  } else { // convert indexes from main shape to subshape
    int size = mySelectedIDs.size();
    for (int i = 0; i < size; i++) {
      TopoDS_Shape aSubShape = aMainMap.FindKey( mySelectedIDs.at(i) );
      int index = aGeomMap.FindIndex( aSubShape );
      aList.append( index );
    }
  }

  return aList;
}
