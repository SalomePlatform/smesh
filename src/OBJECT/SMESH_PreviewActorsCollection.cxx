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

//  SMESH OBJECT : interactive object for SMESH visualization
//  File   : SMESH_PreviewActorsCollection.cxx
//  Author : 
//  Module : SMESH
//
#include "SMESH_PreviewActorsCollection.h"

#include "utilities.h"

#include "SALOME_InteractiveObject.hxx"

// OCC includes
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

// VTK includes
#include <vtkUnstructuredGrid.h>
#include <vtkPlane.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>

#include "VTKViewer_Actor.h"

#include "SVTK_DeviceActor.h"
#include "SALOME_Actor.h"

// QT
#include <QString>
#include <QColor>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;


//vtkStandardNewMacro(SMESH_PreviewActorsCollection);


SMESH_PreviewActorsCollection
::SMESH_PreviewActorsCollection()
{
  if(MYDEBUG) MESSAGE("SMESH_PreviewActorsCollection - "<<this);
}


SMESH_PreviewActorsCollection
::~SMESH_PreviewActorsCollection()
{
  if(MYDEBUG) MESSAGE("~SMESH_PreviewActorsCollection - "<<this);
  if (myRenderer)
    RemoveFromRender(myRenderer);

  QMap<int, GEOM_Actor*>::iterator iter = myMapOfActors.begin();
  for ( ; iter != myMapOfActors.end(); ++iter )
    if ( GEOM_Actor* anActor = iter.value() )
      anActor->Delete();
  myMapOfActors.clear();
}

bool SMESH_PreviewActorsCollection::Init( const TopoDS_Shape& theShape, TopAbs_ShapeEnum theType, const QString& theEntry )
{
  myMainShape = theShape;
  myMapOfActors.clear();
  myMapOfShapes.Clear();

  if ( theShape.IsNull() )
    return false;

  Handle( SALOME_InteractiveObject ) anIO = new SALOME_InteractiveObject();
  anIO->setEntry( theEntry.toLatin1().constData() );
  
  // get indexes of seleted elements
  TopExp::MapShapes(theShape, myMapOfShapes);

  TopExp_Explorer exp( theShape, theType );
  for ( ; exp.More(); exp.Next() ) {
    int index = myMapOfShapes.FindIndex( exp.Current() );
    if ( index && !myMapOfActors.contains( index ) ) { 
      // create actor if the index is present
      if ( GEOM_Actor* anActor = createActor( exp.Current().Oriented(TopAbs_FORWARD))) {
        // Create new entry for actor
        QString aString = theEntry;
        aString += QString("_%1").arg( index ); // add index to actor entry

        // Create interactive object
        Handle( SALOME_InteractiveObject ) anIO = new SALOME_InteractiveObject();
        anIO->setEntry( aString.toLatin1().constData() );

        // Init Actor
        anActor->SetVectorMode( theType==TopAbs_EDGE );
        anActor->setIO( anIO );
        anActor->SetSelector( mySelector );
        anActor->SetPickable( true );
        anActor->SetResolveCoincidentTopology( true );

        // Add Actor to the Actors Map
        myMapOfActors.insert(index, anActor);
      }
    }
  }
  mySelector->ClearIObjects();

  return true;
}

GEOM_Actor* SMESH_PreviewActorsCollection::createActor(const TopoDS_Shape& shape)
{
  GEOM_Actor* actor = GEOM_Actor::New();
  actor->SetShape(shape,0,0);

  //Color Properties
  /*    
        vtkProperty* aProp = vtkProperty::New();
        vtkProperty* aHLProp = vtkProperty::New();
        vtkProperty* aPHLProp = vtkProperty::New();
        
        aProp->SetColor( 255, 0, 0);
        actor->SetProperty(aProp);

        aHLProp->SetColor( 255, 255, 255);
        actor->SetHighlightProperty(aHLProp);

        aPHLProp->SetColor( 155, 155, 155);
        aPHLProp->SetLineWidth ( 3 );
        aPHLProp->SetOpacity ( 0.75 );
        actor->SetPreHighlightProperty(aPHLProp);

        aProp->Delete();
        aHLProp->Delete();
        aPHLProp->Delete();
  */

  return actor;
}

GEOM_Actor* SMESH_PreviewActorsCollection::GetActorByIndex(int index)
{
  return myMapOfActors.value(index);
}

int SMESH_PreviewActorsCollection::GetIndexByShape( const TopoDS_Shape& theShape )
{
  return myMapOfShapes.FindIndex( theShape );
}

void SMESH_PreviewActorsCollection::AddToRender(vtkRenderer* theRenderer)
{
  myRenderer = theRenderer;

  QMap<int, GEOM_Actor*>::iterator iter = myMapOfActors.begin();
  for ( ; iter != myMapOfActors.end(); ++iter )
    iter.value()->AddToRender( theRenderer );
}

void SMESH_PreviewActorsCollection::RemoveFromRender(vtkRenderer* theRenderer){
  QMap<int, GEOM_Actor*>::iterator iter = myMapOfActors.begin();
  for ( ; iter != myMapOfActors.end(); ++iter )
    iter.value()->RemoveFromRender( theRenderer );
}

void SMESH_PreviewActorsCollection::SetSelector(SVTK_Selector* theSelector)
{
  mySelector = theSelector;
}

void SMESH_PreviewActorsCollection::HighlightAll( bool theHighlight ){
  QMap<int, GEOM_Actor*>::iterator iter = myMapOfActors.begin();
  for ( ; iter != myMapOfActors.end(); ++iter )
    iter.value()->Highlight( theHighlight );
}

void SMESH_PreviewActorsCollection::HighlightID( int index ){
  GEOM_Actor* anActor = GetActorByIndex( index );
  if ( anActor && !anActor->isHighlighted() )
    anActor->Highlight( true );
}

void SMESH_PreviewActorsCollection::SetShown( bool shown ){
  QMap<int, GEOM_Actor*>::iterator iter = myMapOfActors.begin();
  for ( ; iter != myMapOfActors.end(); ++iter )
    iter.value()->SetVisibility( shown );
}
