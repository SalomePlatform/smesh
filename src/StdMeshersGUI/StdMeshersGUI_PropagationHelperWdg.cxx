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
// File      : StdMeshersGUI_PropagationHelperWdg.cxx
// Created   : Thu Mar 19 18:46:24 2015
// Author    : Edward AGAPOV (eap)

#include "StdMeshersGUI_PropagationHelperWdg.h"

#include "StdMeshersGUI_SubShapeSelectorWdg.h"
#include "SMESH_PreviewActorsCollection.h"
#include "SMESHGUI_VTKUtils.h"

#include <GEOM_Actor.h>

#include <LightApp_SelectionMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SVTK_ViewWindow.h>
#include <vtkRenderer.h>

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>

#include <BRepTools_WireExplorer.hxx>
#include <BRep_Builder.hxx>
#include <NCollection_DataMap.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>

#include <set>

#define SPACING 6
#define MARGIN 11

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

StdMeshersGUI_PropagationHelperWdg::
StdMeshersGUI_PropagationHelperWdg( StdMeshersGUI_SubShapeSelectorWdg* subSelectWdg,
                                    QWidget*                           parent,
                                    bool                               show ):
  QWidget( parent ), mySubSelectWdg( subSelectWdg ), myActor( 0 ), myModelActor( 0 )
{
  QGroupBox* helperBox = new QGroupBox( tr("HELPER"), this );
  myShowGeomChkBox     = new QCheckBox( tr("SHOW_GEOMETRY"), helperBox );
  myChainBox           = new QGroupBox( tr("PROPAGATION_CHAINS"), helperBox );
  myChainBox->setCheckable( true );
  myChainBox->setChecked( false );
  myListWidget         = new QListWidget( helperBox );
  myListWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  myAddButton          = new QPushButton( tr("ADD"), helperBox );
  myReverseButton      = new QPushButton( tr("REVERSE"), helperBox );

  QGridLayout* chainsLayout = new QGridLayout( myChainBox );
  chainsLayout->setMargin( MARGIN );
  chainsLayout->setSpacing( SPACING );
  chainsLayout->addWidget(myListWidget,    0, 0, 3, 3);
  chainsLayout->addWidget(myAddButton,     0, 3);
  chainsLayout->addWidget(myReverseButton, 1, 3);

  QVBoxLayout* helperLayout = new QVBoxLayout( helperBox );
  helperLayout->setMargin( MARGIN );
  helperLayout->setSpacing( SPACING );
  helperLayout->addWidget( myShowGeomChkBox );
  helperLayout->addWidget( myChainBox );

  QVBoxLayout* lay = new QVBoxLayout( this );
  lay->setMargin( 0 );
  lay->setSpacing( SPACING );
  lay->addWidget( helperBox );

  connect( myShowGeomChkBox,SIGNAL( toggled(bool)), SLOT( onShowGeometry(bool)));
  connect( myChainBox,        SIGNAL( toggled(bool)), SLOT( updateList(bool)));
  connect( myListWidget,    SIGNAL( itemSelectionChanged()), SLOT( onListSelectionChanged() ));
  connect( myAddButton,     SIGNAL( clicked(bool)), SLOT( onAdd() ));
  connect( myReverseButton, SIGNAL( clicked(bool)), SLOT( onReverse() ));

  if ( show )
    onListSelectionChanged();
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshersGUI_PropagationHelperWdg::~StdMeshersGUI_PropagationHelperWdg()
{
  if ( myActor )
  {
    myActor->RemoveFromRender( myRenderer );
    myActor->Delete();
    myActor = 0;
  }
  if ( myModelActor )
  {
    myModelActor->RemoveFromRender( myRenderer );
    myModelActor->Delete();
    myModelActor = 0;
  }
}

//================================================================================
/*!
 * \brief Switch off all buttons and previews
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::Clear()
{
  myShowGeomChkBox->setChecked( false );

  myListWidget->blockSignals( true );
  myListWidget->clear();
  myListWidget->blockSignals( false );

  myChainBox->blockSignals( true );
  myChainBox->setChecked( false );
  myChainBox->blockSignals( false );

  if ( myActor )
    myActor->SetVisibility( false );

  if ( myModelActor )
    myModelActor->SetVisibility( false );
}

//================================================================================
/*!
 * \brief SLOT called when 'Show Geometry' is checked
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::onShowGeometry(bool toShow)
{
  if ( ! myModelActor )
  {
    TopoDS_Shape shape     = mySubSelectWdg->GetGeomShape();
    TopoDS_Shape mainShape = mySubSelectWdg->GetMainShape();
    if ( shape.IsNull() && mainShape.IsNull() ) return;
    if ( mainShape.IsNull() ) mainShape = shape;

    SUIT_OverrideCursor wc;

    TopoDS_Compound aCompound;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound( aCompound );
    
    TopTools_MapOfShape edgesMap;
    TopExp_Explorer edge( mainShape, TopAbs_EDGE );
    for ( ; edge.More(); edge.Next() )
      if ( edgesMap.Add( edge.Current() ))
        aBuilder.Add( aCompound, edge.Current() );

    myModelActor = GEOM_Actor::New();
    myModelActor->SetShape( aCompound, 0, 0 );
    myModelActor->SetPickable( false );
    myModelActor->SetIsolatedEdgeColor( 0.5, 0.5, 0.5 );

    if (( myRenderer = mySubSelectWdg->GetRenderer() ))
      myModelActor->AddToRender( myRenderer );
  }

  if ( myModelActor )
    myModelActor->SetVisibility( toShow );

  SMESH::RepaintCurrentView();
}

//================================================================================
/*!
 * \brief Build propagation chains. Return false if no chains found
 */
//================================================================================

bool StdMeshersGUI_PropagationHelperWdg::buildChains()
{
  if ( !myChains.empty() ) return false;

  if ( !mySubSelectWdg ) return false;

  TopoDS_Shape shape     = mySubSelectWdg->GetGeomShape();
  TopoDS_Shape mainShape = mySubSelectWdg->GetMainShape();
  if ( shape.IsNull() && mainShape.IsNull() ) return false;

  if ( shape.IsNull() )     shape = mainShape;
  if ( mainShape.IsNull() ) mainShape = shape;

  SUIT_OverrideCursor wc;

  // aPreviewActor holds a map od all sub-shapes of mainShape
  SMESH_PreviewActorsCollection* previewActor = mySubSelectWdg->GetActorCollection();
  if ( !previewActor ) return false;
  const QList<int>& egdeIDs = previewActor->GetIndices();

  // Make a 'map' of WIREs of EDGE with quadrilateral WIREs only

  typedef int                        TGeomID; // index in the mainShape
  typedef std::vector< TGeomID >     TWire; // signed IDs of EDGEs, sign means orientation
  typedef std::pair< int, TWire* >   TEdgeInWire; // index in TWire + TWire*
  typedef std::vector< TEdgeInWire > TWiresOfEdge;// WIREs including an EDGE

  std::vector< TWire > quadWires;
  quadWires.reserve( previewActor->NbShapesOfType( TopAbs_FACE ));
  NCollection_DataMap< TGeomID, TWiresOfEdge >
    wiresOfEdge( previewActor->NbShapesOfType( TopAbs_EDGE ));

  TopExp_Explorer wire;
  TopTools_MapOfShape faceMap;
  for ( TopExp_Explorer face( mainShape, TopAbs_FACE ); face.More(); face.Next() )
  {
    if ( !faceMap.Add( face.Current() )) continue;

    wire.Init( face.Current(), TopAbs_WIRE );
    TopoDS_Shape W = wire.Current().Oriented( TopAbs_FORWARD );

    wire.Next();
    if ( wire.More() ) continue;

    // count EDGEs
    int nbE = 0;
    for ( TopoDS_Iterator edge( W, false, false ); edge.More() && nbE < 5; ++nbE )
      edge.Next();
    if ( nbE != 4 ) continue;

    // fill a TWire and TWiresOfEdge
    quadWires.push_back( TWire() );
    TWire& wire = quadWires.back();
    wire.reserve( 4 );
    for ( BRepTools_WireExplorer edge( TopoDS::Wire( W )); edge.More(); edge.Next() )
    {
      const TopoDS_Shape& E = edge.Current();
      int iE = previewActor->GetIndexByShape( E );
      if ( iE < 1 )
        continue;
      if ( !wiresOfEdge.IsBound( iE ))
        wiresOfEdge.Bind( iE, TWiresOfEdge() );
      wiresOfEdge( iE ).push_back( TEdgeInWire( wire.size(), & wire ));

      wire.push_back( E.Orientation() == TopAbs_REVERSED ? -iE : iE );
    }
  }
  if ( quadWires.empty() )
    return false;

  // Find chains

  TColStd_IndexedMapOfInteger chain, chainedEdges;

  TColStd_MapOfInteger shapeEdges;
  if ( !shape.IsSame( mainShape ))
    for ( QList<TGeomID>::const_iterator ieIt = egdeIDs.begin(); ieIt != egdeIDs.end(); ++ieIt )
      shapeEdges.Add( *ieIt );

  // loop on all EDGEs in mainShape
  for ( QList<TGeomID>::const_iterator ieIt = egdeIDs.begin(); ieIt != egdeIDs.end(); ++ieIt )
  {
    if ( chainedEdges.Contains( *ieIt ))
      continue;
    // start a new chain
    chain.Clear();
    chain.Add( *ieIt );
    chainedEdges.Add( *ieIt );
    for ( int iC = 1; iC <= chain.Extent(); ++iC ) // loop on EDGE's in chain
    {
      TGeomID iE = chain( iC ), iEAbs = Abs( iE );
      if ( !wiresOfEdge.IsBound( iEAbs ))
        continue;
      const TWiresOfEdge& wires = wiresOfEdge( iEAbs );
      for (size_t i = 0; i < wires.size(); ++i ) // loop on WIREs sharing iE
      {
        const TEdgeInWire& eInW = wires[i];
        const TWire&    W = *eInW.second;
        if ( W.size() != 4 ) continue;
        const int    iInW = eInW.first;
        const int iInWOpp = ( iInW + 2 ) % 4; // an opposite edge index
        TGeomID  iEOppAbs = Abs( W[ iInWOpp ] );

        int prevNbChained = chainedEdges.Extent();
        if ( prevNbChained < chainedEdges.Add( iEOppAbs ))
        {
          int dir = iE / iEAbs;
          bool isSameDir = ( W[ iInW ] * W[ iInWOpp ] < 0 );
          if ( !isSameDir )
            dir *= -1;
          chain.Add( dir * iEOppAbs );
        }
      }
    }
    // store a chain
    if ( chain.Extent() > 1 )
    {
      myChains.push_back( std::vector< TGeomID >() );
      std::vector< TGeomID > & ch = myChains.back();
      for ( int iC = 1; iC <= chain.Extent(); ++iC )
      {
        TGeomID iE = chain( iC );
        if ( shapeEdges.IsEmpty() || shapeEdges.Contains( Abs( iE )))
          ch.push_back( iE );
      }
      if ( ch.size() < 2 )
        myChains.pop_back();
    }
  } // loop on egdeIDs

  return !myChains.empty();
}

//================================================================================
/*!
 * \brief Fills myListWidget
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::updateList(bool enable)
{
  buildChains();

  myListWidget->clear();

  if ( enable )
  {
    QListWidgetItem* item;
    if ( myChains.empty() || !enable )
    {
      item = new QListWidgetItem(tr("NO_CHAINS"), myListWidget );
      item->setData( Qt::UserRole, -1 );
    }
    else
      for ( size_t i = 0; i < myChains.size(); ++i )
      {
        QString text = tr( "CHAIN_NUM_NB_EDGES" ).arg( i+1 ).arg( myChains[i].size() );
        item = new QListWidgetItem( text, myListWidget );
        item->setData( Qt::UserRole, (int) i );
      }
  }
  else
  {
    onListSelectionChanged();
  }
}

//================================================================================
/*!
 * \brief Returns ids of a selected chain
 */
//================================================================================

std::vector< int > * StdMeshersGUI_PropagationHelperWdg::getSelectedChain()
{
  std::vector< int > * chain = 0;
  if ( QListWidgetItem * item = myListWidget->currentItem() )
  {
    size_t i = (size_t) item->data( Qt::UserRole ).toInt();
    if ( 0 <= i && i < myChains.size() )
      chain = & myChains[i];
  }
  return chain;
}

//================================================================================
/*!
 * \brief SLOT called when a selected chain changes
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::onListSelectionChanged()
{
  if ( !mySubSelectWdg ) return;
  SMESH_PreviewActorsCollection* previewActor = mySubSelectWdg->GetActorCollection();
  if ( !previewActor ) return;

  bool hasReversedEdges = false;
  const std::vector< int > * chain = getSelectedChain();
  if ( chain )
  {
    SUIT_OverrideCursor wc;

    TopoDS_Compound aCompound;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound( aCompound );

    for ( size_t i = 0; i < chain->size(); ++i )
    {
      int iE = Abs( (*chain)[ i ]);
      TopoDS_Shape E = previewActor->GetShapeByIndex( iE );
      if ( !E.IsNull() && E.ShapeType() == TopAbs_EDGE )
      {
        E.Orientation( (*chain)[ i ] < 0 ? TopAbs_REVERSED : TopAbs_FORWARD );
        aBuilder.Add( aCompound, E );
        if ( (*chain)[ i ] < 0 )
          hasReversedEdges = true;
      }
    }
    if ( myActor )
    {
      // SetShape() to an existing actor leads to a wrong FitAll
      myActor->RemoveFromRender( myRenderer );
      myActor->Delete();
    }
    myActor = GEOM_Actor::New();
    myActor->SetShape( aCompound, 0, true );
    myActor->SetIsolatedEdgeColor( 1, 0, 1 );
    myActor->SetWidth( 2 );
    myActor->SetVectorMode( true );
    myActor->SetPickable( false );

    if (( myRenderer = mySubSelectWdg->GetRenderer() ))
      myActor->AddToRender( myRenderer );

    if ( LightApp_SelectionMgr* selMrg = SMESHGUI::selectionMgr())
    {
      selMrg->clearSelected();
      mySubSelectWdg->ClearSelected(); /* call this because the above call does not
                                          lead to currentSelectionChanged signal (bug?)*/
    }
  }
  bool enableButtons = chain;
  if ( chain )
    enableButtons = myListWidget->currentItem()->data( Qt::UserRole+1 ).isNull();

  myAddButton->setEnabled( enableButtons && hasReversedEdges );
  myReverseButton->setEnabled( enableButtons );


  bool toShowChain = chain;

  if ( myActor )
    myActor->SetVisibility( toShowChain );

  previewActor->SetShown( !toShowChain );

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow())
    aViewWindow->Repaint();
}

//================================================================================
/*!
 * \brief SLOT called when 'Add' button is clicked
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::onAdd()
{
  const std::vector< int > * chain = getSelectedChain();
  if ( !chain || !mySubSelectWdg ) return;

  // join current and new IDs

  SMESH::long_array_var ids = mySubSelectWdg->GetListOfIDs();

  std::set< int > idSet;
  for ( int i = 0, nb = ids->length(); i < nb; ++i )
    idSet.insert( idSet.end(), ids[i] );

  for ( size_t i = 0; i < chain->size(); ++i )
    if ( (*chain)[ i ] < 0 )
      idSet.insert( -1 * (*chain)[ i ]);

  if ( ids->length() != idSet.size() )
  {
    ids->length( idSet.size() );
    std::set< int >::iterator id = idSet.begin();
    for ( int i = 0, nb = ids->length(); i < nb; ++i, ++id )
      ids[ i ] = *id;

    mySubSelectWdg->SetListOfIDs( ids );
  }
  mySubSelectWdg->ClearSelected();

  if ( QListWidgetItem * item = myListWidget->currentItem() )
  {
    //delete item;
    item->setForeground( QBrush( QColor( 100, 100, 100 )));
    item->setData( Qt::UserRole+1, 1 );
  }
  myAddButton->setEnabled( false );
  myReverseButton->setEnabled( false );
}

//================================================================================
/*!
 * \brief SLOT called when 'Reverse' button is clicked
 */
//================================================================================

void StdMeshersGUI_PropagationHelperWdg::onReverse()
{
  if ( std::vector< int > * chain = getSelectedChain())
  {
    for ( size_t i = 0; i < chain->size(); ++i )
      (*chain)[ i ] *= -1;

    onListSelectionChanged();
  }
}
