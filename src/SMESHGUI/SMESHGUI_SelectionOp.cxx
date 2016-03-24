// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

// File   : SMESHGUI_SelectionOp.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SelectionOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Selection.h"

#include <SMESH_Actor.h>
#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>

// SALOME GUI includes
#include <SUIT_SelectionFilter.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Study.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>

#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes 
#include <SALOMEDS_SObject.hxx>

/*
  Class       : SMESHGUI_SelectionOp
  Description : Base operation for all operations using object selection in viewer or objectbrowser
                through common widgets created by LightApp_Dialog::createObject
*/

//=================================================================================
// name     : SMESHGUI_SelectionOp
// purpose  : 
//=================================================================================
SMESHGUI_SelectionOp::SMESHGUI_SelectionOp( const Selection_Mode mode )
: SMESHGUI_Operation(),
  myDefSelectionMode( mode )
{
}

//=================================================================================
// name     : ~SMESHGUI_SelectionOp
// purpose  :
//=================================================================================
SMESHGUI_SelectionOp::~SMESHGUI_SelectionOp()
{
  removeCustomFilters();
}

//=================================================================================
// name     : startOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::startOperation()
{
  myOldSelectionMode = selectionMode();
  setSelectionMode( myDefSelectionMode );

  SMESHGUI_Operation::startOperation();
  if( dlg() )
  {
    disconnect( dlg(), SIGNAL( objectActivated( int ) ), this, SLOT( onActivateObject( int ) ) );
    disconnect( dlg(), SIGNAL( objectDeactivated( int ) ), this, SLOT( onDeactivateObject( int ) ) );
    disconnect( dlg(), SIGNAL( selectionChanged( int ) ), this, SLOT( onSelectionChanged( int ) ) );
    connect( dlg(), SIGNAL( objectActivated( int ) ), this, SLOT( onActivateObject( int ) ) );
    connect( dlg(), SIGNAL( objectDeactivated( int ) ), this, SLOT( onDeactivateObject( int ) ) );
    connect( dlg(), SIGNAL( selectionChanged( int ) ), this, SLOT( onSelectionChanged( int ) ) );
  }
}

//=================================================================================
// name     : removeCustomFilters
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::removeCustomFilters()
{
  if (myFilters.count() > 0) {
    LightApp_SelectionMgr* mgr = selectionMgr();
    Filters::const_iterator anIt = myFilters.begin(),
                            aLast = myFilters.end();
    for ( ; anIt != aLast; anIt++) {
      if (anIt.value()) {
        if (mgr) mgr->removeFilter(anIt.value());
        delete anIt.value();
      }
    }

    myFilters.clear();
  }
}

//=================================================================================
// name     : commitOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::commitOperation()
{
  SMESHGUI_Operation::commitOperation();  
  removeCustomFilters();
  setSelectionMode( myOldSelectionMode );
}

//=================================================================================
// name     : abortOperation
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::abortOperation()
{
  SMESHGUI_Operation::abortOperation();
  removeCustomFilters();
  setSelectionMode( myOldSelectionMode );  
}

//=================================================================================
// name     : selectionDone
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::selectionDone()
{
  if( !dlg() )
    return;

  if( selectionMode()!=ActorSelection )
  {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects( aList, SVTK_Viewer::Type() );

    if( aList.Extent() != 1 ) //we can select nodes or elements only within one mesh
    {
      dlg()->clearSelection();
      return;
    }    
  }
    
  QStringList names, ids;
  LightApp_Dialog::TypesList types;
  selected( names, types, ids );
  dlg()->selectObject( names, types, ids );
}

//=================================================================================
// name     : createFilter
// purpose  :
//=================================================================================
SUIT_SelectionFilter* SMESHGUI_SelectionOp::createFilter( const int ) const
{
  return 0;
}

//=================================================================================
// name     : onActivateObject
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onActivateObject( int id )
{
  LightApp_SelectionMgr* mgr = selectionMgr();
  if( !mgr )
    return;
    
  if( !myFilters.contains( id ) )
    myFilters[ id ] = createFilter( id );

  if( myFilters[ id ] )
    mgr->installFilter( myFilters[ id ] );

  selectionDone();
}

//=================================================================================
// name     : onDeactivateObject
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onDeactivateObject( int id )
{
  removeCustomFilters();
}

//=================================================================================
// name     : initDialog
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::initDialog()
{
  if( dlg() )
  {
    dlg()->clearSelection();
    dlg()->deactivateAll();
  }
}

//=================================================================================
// name     : initDialog
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onSelectionChanged( int )
{
}

//=======================================================================
// name    : selectionMode
// Purpose : Returns selection mode
//=======================================================================
Selection_Mode SMESHGUI_SelectionOp::selectionMode() const
{
  SVTK_ViewWindow* wnd = viewWindow();
  if( wnd )
    return wnd->SelectionMode();
  else
    return ActorSelection;
}

//=======================================================================
// name    : setSelectionMode
// Purpose : Set selection mode
//=======================================================================
void SMESHGUI_SelectionOp::setSelectionMode( const Selection_Mode mode )
{
  SVTK_ViewWindow* wnd = viewWindow();
  if( wnd )
    wnd->SetSelectionMode( mode );
}

//=======================================================================
// name    : highlight
// Purpose : Highlight object in 3d viewer
//=======================================================================
void SMESHGUI_SelectionOp::highlight( const Handle( SALOME_InteractiveObject )& obj,
                                      const bool hilight, const bool immediately )
{
  SVTK_ViewWindow* wnd = viewWindow();
  if( wnd )
    wnd->highlight( obj, hilight, immediately );
}

//=======================================================================
// name    : addOrRemoveIndex
// Purpose : Select/deselect cells of mesh
//=======================================================================
void SMESHGUI_SelectionOp::addOrRemoveIndex( const Handle( SALOME_InteractiveObject )& obj,
                                             const TColStd_MapOfInteger& indices,
                                             const bool isModeShift )
{
  SVTK_Selector* sel = selector();
  if( sel )
    sel->AddOrRemoveIndex( obj, indices, isModeShift );
}

//=======================================================================
// name    : viewWindow
// Purpose : Get active view window
//=======================================================================
SVTK_ViewWindow* SMESHGUI_SelectionOp::viewWindow() const
{
  return SMESH::GetViewWindow( getSMESHGUI() );
}

//=======================================================================
// name    : selector
// Purpose : Get selector
//=======================================================================
SVTK_Selector* SMESHGUI_SelectionOp::selector() const
{
  SVTK_ViewWindow* wnd = viewWindow();
  return wnd ? wnd->GetSelector() : 0;
}

//=======================================================================
// name    : typeById
// Purpose : Find type by id
//=======================================================================
int SMESHGUI_SelectionOp::typeById( const QString& str, const EntityType objtype ) const
{
  SalomeApp_Study* _study = dynamic_cast<SalomeApp_Study*>( study() );
  if( !_study )
    return -1;

  _PTR( Study ) st = _study->studyDS();

  int res = -1;
  if( objtype == Object )
  {
    SalomeApp_Study* _study = dynamic_cast<SalomeApp_Study*>( study() );
    if( _study )
    {
      int t = SMESHGUI_Selection::type( str, _study->studyDS() );
      if( t<0 )
      {
        //try to get GEOM type
        _PTR( SObject ) sobj = st->FindObjectID( str.toLatin1().data() );
        if( sobj )
        {
          GEOM::GEOM_Object_var obj = GEOM::GEOM_Object::_narrow(
            dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
          if( !CORBA::is_nil( obj ) )
            // as decoding of type id is not realized in LightApp_Dialog,
            //make all GEOM objects have same type id
            res = SMESHGUI_Dialog::prefix( "GEOM" );// + obj->GetType();
        }
      }
      else
        res = SMESHGUI_Dialog::prefix( "SMESH" ) + t;
    }
  }
  else
  {
    int pos = str.indexOf( idChar() );
    QString entry = str.left( pos ),
            _id = str.mid( pos+1 );
    bool ok;
    int id = _id.toInt( &ok );
    if( ok )
    {
      _PTR( SObject ) sobj = st->FindObjectID( entry.toLatin1().data() );
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( 
        dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
      SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( 
        dynamic_cast<SALOMEDS_SObject*>( sobj.get() )->GetObject() );
      if( !CORBA::is_nil( mesh ) )
        res = SMESHGUI_Dialog::prefix( "SMESH element" ) + 
          mesh->GetElementType( id, objtype==MeshElement );

      else if( !CORBA::is_nil( submesh ) )
        res = SMESHGUI_Dialog::prefix( "SMESH element" ) + 
          submesh->GetElementType( id, objtype==MeshElement );
    }
  }

  return res;
}

//=======================================================================
// name    : selected
// Purpose : Get names, types and ids of selected objects
//=======================================================================
void SMESHGUI_SelectionOp::selected( QStringList& names,
                                     LightApp_Dialog::TypesList& types,
                                     QStringList& ids ) const
{
/*  SUIT_DataOwnerPtrList list; selectionMgr()->selected( list );
  SUIT_DataOwnerPtrList::const_iterator anIt = list.begin(),
                                        aLast = list.end();
  for( ; anIt!=aLast; anIt++ )
  {
    LightApp_DataOwner* owner = dynamic_cast<LightApp_DataOwner*>( (*anIt).operator->() );
    LightApp_SVTKDataOwner* vtkowner = dynamic_cast<LightApp_SVTKDataOwner*>( (*anIt).operator->() );

    if( vtkowner )
    {
      QString id_str = QString( "%1%2%3" ).arg( vtkowner->entry() ).arg( idChar() ), current_id_str;
      Selection_Mode mode = vtkowner->GetMode();
      EntityType objtype = mode == NodeSelection ? MeshNode : MeshElement;
      const TColStd_IndexedMapOfInteger& ownerids = vtkowner->GetIds();

      for( int i=1, n=ownerids.Extent(); i<=n; i++ )
      {
        int curid = ownerids( i );
        current_id_str = id_str.arg( curid );
        ids.append( current_id_str );
        types.append( typeById( current_id_str, objtype ) );
        names.append( QString( "%1" ).arg( curid ) );
      }
    }

    else if( owner )
    {
      QString id = owner->entry();
      ids.append( id );
      types.append( typeById( id, Object ) );
      names.append( owner->IO()->getName() );
    }
  }*/

  SALOME_ListIO selObjs;
  TColStd_IndexedMapOfInteger selIndices;
  selectionMgr()->selectedObjects( selObjs );
  Selection_Mode mode = selectionMode();
  EntityType objtype = mode == NodeSelection ? MeshNode : MeshElement;

  for( SALOME_ListIteratorOfListIO anIt( selObjs ); anIt.More(); anIt.Next() )
  {
    selIndices.Clear();
    selectionMgr()->GetIndexes( anIt.Value(), selIndices );
    if( selIndices.Extent() > 0 )
    {
      QString id_str = QString( "%1%2%3" ).arg( anIt.Value()->getEntry() ).arg( idChar() ), current_id_str;
      for( int i=1, n=selIndices.Extent(); i<=n; i++ )
      {
        int curid = selIndices( i );
        current_id_str = id_str.arg( curid );
        ids.append( current_id_str );
        types.append( typeById( current_id_str, objtype ) );
        names.append( QString( "%1" ).arg( curid ) );
      }
    }
    else
    {
      QString id = anIt.Value()->getEntry();
      ids.append( id );
      types.append( typeById( id, Object ) );
      SalomeApp_Study* _study = dynamic_cast<SalomeApp_Study*>( study() );
      if( _study )
      {
        _PTR(SObject) obj = _study->studyDS()->FindObjectID( anIt.Value()->getEntry() );
        if( obj )
          names.append( QString( obj->GetName().c_str() ).trimmed() );
      }
    }
  }
}

//=======================================================================
// name    : idChar
// Purpose : Char using to divide <entry> and <id> in string id representation. By default, '#'
//=======================================================================
QChar SMESHGUI_SelectionOp::idChar() const
{
  return '#';
}

//=================================================================================
// name     : mesh
// purpose  :
//=================================================================================
SMESH::SMESH_Mesh_var SMESHGUI_SelectionOp::mesh() const
{
  if( selectionMode()==ActorSelection )
    return SMESH::SMESH_Mesh::_nil();
    
  SALOME_ListIO sel; selectionMgr()->selectedObjects( sel, SVTK_Viewer::Type() );
  if( sel.Extent()==1 )
    return SMESH::GetMeshByIO( sel.First() );
  else
    return SMESH::SMESH_Mesh::_nil();
}

//=================================================================================
// name     : actor
// purpose  :
//=================================================================================
SMESH_Actor* SMESHGUI_SelectionOp::actor() const
{
  SMESH::SMESH_Mesh_var m = mesh();
  if( !m->_is_nil() )
    return SMESH::FindActorByObject( m.in() );
  else
    return 0;
}

//=================================================================================
// name     : onTextChanged
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::onTextChanged( int, const QStringList& list )
{
    if( !dlg() )
      return;

    TColStd_MapOfInteger newIndices;

    SALOME_ListIO sel; selectionMgr()->selectedObjects( sel );
    SMESH_Actor* anActor = actor();
    if( sel.Extent()==0 || !anActor )
      return;

    SMDS_Mesh* aMesh = anActor->GetObject()->GetMesh();

    IdList ids; extractIds( list, ids, '\0' );
    IdList::const_iterator anIt = ids.begin(),
                           aLast = ids.end();
    if ( selectionMode() == NodeSelection )
      for( ; anIt!=aLast; anIt++ ) {
        if( const SMDS_MeshNode * n = aMesh->FindNode( *anIt ) )
          newIndices.Add( n->GetID() );
      }
    else 
      for( ; anIt!=aLast; anIt++ ) {
        if( const SMDS_MeshElement* e = aMesh->FindElement( *anIt ) )
          newIndices.Add( e->GetID() );
      }

    selector()->AddOrRemoveIndex( sel.First(), newIndices, false );
    highlight( sel.First(), true, true );

    QStringList names, _ids; LightApp_Dialog::TypesList types;
    selected( names, types, _ids );
    dlg()->selectObject( names, types, _ids, false );
}

//=================================================================================
// name     : selectedIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::selectedIds( const int id, IdList& list ) const
{
  if( !dlg() )
    return;

  QStringList ids; dlg()->selectedObject( id, ids );
  extractIds( ids, list );
}

//=================================================================================
// name     : extractIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::extractIds( const QStringList& ids, IdList& list, const QChar idchar )
{
  QStringList::const_iterator anIt = ids.begin(),
                              aLast = ids.end();
  QString id_str;
  for( ; anIt!=aLast; anIt++ )
  {
    id_str = *anIt;
    int pos = idchar=='\0' ? -1 : id_str.indexOf( idchar );
    int id = -1;
    if( idchar=='\0' || pos>=0 )
    {
      id = id_str.mid( pos+1 ).toInt();
      list.append( id );
    }
  }
}

//=================================================================================
// name     : extractIds
// purpose  :
//=================================================================================
void SMESHGUI_SelectionOp::extractIds( const QStringList& ids, IdList& list ) const
{
  extractIds( ids, list, idChar() );
}
