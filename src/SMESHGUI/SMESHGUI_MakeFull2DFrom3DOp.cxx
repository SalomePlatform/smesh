// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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
// File   : SMESHGUI_MakeFull2DFrom3DOp.cxx
// Author : Cesar Conopoima, Open CASCADE S.A.S. (cesar.conopoima@opencascade.com)

#include "SMESHGUI_MakeFull2DFrom3DOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
//#include <SVTK_ViewWindow.h>
#include <SalomeApp_Study.h>
#include <SalomeApp_Tools.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

// Qt includes
#include <QGroupBox>

#include <Standard_ErrorHandler.hxx>

/*!
  \class SMESHGUI_Make2DFrom3DDlg
  \brief Copy Mesh dialog box
*/

SMESHGUI_MakeFull2DFrom3DDlg::SMESHGUI_MakeFull2DFrom3DDlg( QWidget* parent )
  : SMESHGUI_Make2DFrom3DDlg( parent )
{
  // title
  setWindowTitle( tr("CAPTION") );
  myModeGrp->setVisible(false);
}

SMESHGUI_MakeFull2DFrom3DDlg::~SMESHGUI_MakeFull2DFrom3DDlg()
{
}


/*!
  \class SMESHGUI_MakeFull2DFrom3DOp
  \brief Copy Mesh operation class
*/
SMESHGUI_MakeFull2DFrom3DOp::SMESHGUI_MakeFull2DFrom3DOp()
   : SMESHGUI_SelectionOp(),
    myMeshFilter(SMESH::MESH),
    myGroupFilter(SMESH::GROUP)
{
}

SMESHGUI_MakeFull2DFrom3DOp::~SMESHGUI_MakeFull2DFrom3DOp()
{
}

LightApp_Dialog* SMESHGUI_MakeFull2DFrom3DOp::dlg() const
{
  return myDlg;
}

void SMESHGUI_MakeFull2DFrom3DOp::startOperation()
{
  if( !myDlg )
    myDlg = new SMESHGUI_MakeFull2DFrom3DDlg( desktop() );


  myHelpFileName = "make_2dmesh_from_3d_elements.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
  myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  myDlg->show();

  myDlg->activateObject( SMESHGUI_MakeFull2DFrom3DDlg::MeshOrGroups );
  selectionDone();
}

void SMESHGUI_MakeFull2DFrom3DOp::selectionDone()
{
  myDlg->clearSelection( SMESHGUI_MakeFull2DFrom3DDlg::MeshOrGroups );
  mySrcMesh = SMESH::SMESH_Mesh::_nil();

  if ( !dlg() ) return;
  if ( dlg()->isVisible() ) {
    try {
      QStringList names, ids;
      LightApp_Dialog::TypesList types;
      selected( names, types, ids );
      for ( int i = 0; i < names.count(); ++i )
        names[i] = names[i].trimmed();
      myDlg->selectObject( names, types, ids );

      // enable/disable "new mesh" button
      bool isMesh = true;
      for ( int i = 0; i < ids.count() && isMesh; ++i )
      {
        _PTR(SObject) sobj = SMESH::getStudy()->FindObjectID( ids[i].toUtf8().constData() );
        mySrcMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( sobj );  
      }
      myDlg->setNewMeshEnabled( isMesh );
    }
    catch ( const SALOME_CMOD::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    catch ( ... ) {
    }
  }
}

SUIT_SelectionFilter* SMESHGUI_MakeFull2DFrom3DOp::createFilter( const int /*theId*/ ) const
{
  SMESHGUI_MakeFull2DFrom3DOp* me = (SMESHGUI_MakeFull2DFrom3DOp*) this;

  QList<SUIT_SelectionFilter*> subFilters;
  subFilters.append( & me->myMeshFilter );
  subFilters.append( & me->myGroupFilter );

  SUIT_SelectionFilter* f = new SMESH_LogicalFilter( subFilters, SMESH_LogicalFilter::LO_OR );
  return f;
}

bool SMESHGUI_MakeFull2DFrom3DOp::isValid( QString& msg ) const
{
  if ( !dlg() ) return false;
  
  // check if a mesh is selected
  if ( !myDlg->hasSelection( SMESHGUI_MakeFull2DFrom3DDlg::MeshOrGroups ))
  {
    msg = tr( "SMESH_ERR_NO_INPUT_MESH" );
    return false;
  }

  QStringList entries;
  dlg()->selectedObject( SMESHGUI_MakeFull2DFrom3DDlg::MeshOrGroups, entries );
  const bool isMeshSelected = ( !mySrcMesh->_is_nil() );
  if ( isMeshSelected )
  {
    // only one mesh is allowed
    if ( entries.size() > 1 ) {
      msg = tr( "SMESH_TOO_MANY_MESHES" );
      return false;
    }
  }
  else
  {
    // check if only groups are selected
    for ( int i = 0; i < entries.count(); ++i )
    {
      SMESH::SMESH_GroupBase_var grp;
      if ( _PTR(SObject) sobj = SMESH::getStudy()->FindObjectID( entries[i].toUtf8().constData() ))
        grp = SMESH::SObjectToInterface<SMESH::SMESH_GroupBase>( sobj );
      if ( grp->_is_nil() ) {
        msg = tr( "SMESH_NOT_ONLY_GROUPS" );
        return false;
      }
    }
  }
  // check if the selected objects contains elements of required type
  bool hasVolumes = false;
  
  for ( int i = 0; i < entries.count(); ++i )
  {
    SMESH::SMESH_IDSource_var idSource;
    if ( _PTR(SObject) sobj = SMESH::getStudy()->FindObjectID( entries[i].toUtf8().constData() ))
      idSource = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );
    if ( !idSource->_is_nil() ) {
      SMESH::array_of_ElementType_var types = idSource->GetTypes();
      for ( int j = 0; j < (int) types->length(); ++j )
        if ( types[j] == SMESH::VOLUME )
          hasVolumes = true;
    }
  }
  if ( !hasVolumes ) {
    msg = tr( "SMESH_ERR_NO_3D_ELEMENTS" );
    return false;
  } 

  // check if new mesh name is specified
  if ( myDlg->needNewMesh() && myDlg->getNewMeshName().isEmpty() ) {
    msg = tr( "SMESH_ERR_MESH_NAME_NOT_SPECIFIED" );
    return false;
  }

  // check if group name is specified
  if ( myDlg->needGroup() && myDlg->getGroupName().isEmpty() ) {
    msg = tr( "SMESH_ERR_GRP_NAME_NOT_SPECIFIED" );
    return false;
  }

  return true;
}

bool SMESHGUI_MakeFull2DFrom3DOp::compute2DMesh( QStringList& theEntryList )
{
  SUIT_OverrideCursor wc;

  bool ok = false;
  try {
    QString meshName          = myDlg->needNewMesh() ? myDlg->getNewMeshName() : QString();
    QString groupName         = myDlg->needGroup()   ? myDlg->getGroupName()   : QString();
    bool copyAll              = myDlg->copySource();

    QStringList entries;
    dlg()->selectedObject( SMESHGUI_MakeFull2DFrom3DDlg::MeshOrGroups, entries );
    SMESH::ListOfIDSources_var groups = new SMESH::ListOfIDSources;
    QString wrongGroups = "";

    if ( mySrcMesh->_is_nil() ) // get selected groups, find groups of wrong type
    {
      int nbGroups = 0;      
      groups->length( entries.count() );
      for ( int i = 0; i < entries.count(); ++i )
      {
        _PTR(SObject) sobj = SMESH::getStudy()->FindObjectID( entries[i].toUtf8().constData() );
        SMESH::SMESH_IDSource_var grp = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( sobj );  
        SMESH::array_of_ElementType_var types = grp->GetTypes();
        if ( types->length() < 1 || types[0] != SMESH::VOLUME )
        {
          if ( !wrongGroups.isEmpty() )
            wrongGroups += ", ";
          wrongGroups += sobj->GetName().c_str();
        }
        else
        {
          groups[ nbGroups++ ] = grp;
        }
      }
      groups->length( nbGroups );
      mySrcMesh = groups[0]->GetMesh();
    }

    if ( !CORBA::is_nil( mySrcMesh ) ) {
      SMESH::SMESH_MeshEditor_var aMeshEditor = mySrcMesh->GetMeshEditor();
      SMESH::SMESH_Group_var newGrp;
      SMESH::SMESH_Mesh_var newMesh;
      CORBA::Long nbAdded = aMeshEditor->MakeBoundaryElements( SMESH::BND_2DFROM3D,
                                                               groupName.toUtf8().constData(),
                                                               meshName.toUtf8().constData(),
                                                               copyAll,
                                                               true,
                                                               groups,
                                                               newMesh.out(),
                                                               newGrp.out() );
      QString msg = tr("NB_ADDED").arg( nbAdded );
      if ( !wrongGroups.isEmpty() )
        msg += ".\n" + tr("WRONG_GROUPS").arg( wrongGroups );
      SUIT_MessageBox::information( myDlg, tr("SMESH_INFORMATION"), msg);

      if ( !newMesh->_is_nil() ) {
        if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( newMesh ) )
          theEntryList.append( aSObject->GetID().c_str() );
      }
      ok = true;

      for ( int i = 0; i < entries.count(); ++i )
        if ( SMESH_Actor* actor = SMESH::FindActorByEntry( entries[i].toUtf8().constData() ))
        {
          actor->SetEntityMode( actor->GetEntityMode() | SMESH_Actor::eFaces );
          SMESH::Update( actor->getIO(), actor->GetVisibility() );
        }
      SMESH::RepaintCurrentView();
    }
  }
  catch ( ... ) {
  }
  return ok;
}

bool SMESHGUI_MakeFull2DFrom3DOp::onApply()
{
  if ( SMESHGUI::isStudyLocked() )
    return false;

  QString msg;
  if ( !isValid( msg ) ) {
    dlg()->show();
    if ( msg != "" )
      SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), msg );
    return false;
  }

  QStringList anEntryList;
  bool res = false;
  try {
    res = compute2DMesh( anEntryList );
  }
  catch ( const SALOME_CMOD::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... ) {
  }

  if ( res ) {
    SMESHGUI::Modified();
    update( UF_ObjBrowser | UF_Model );
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    myDlg->setNewMeshName( SMESH::UniqueName( "Mesh_1" ) );
    myDlg->setGroupName( SMESH::UniqueName( "Group" ) );
  }
  else {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), tr( "SMESH_OPERATION_FAILED" ) );
  }

  return res;
}
