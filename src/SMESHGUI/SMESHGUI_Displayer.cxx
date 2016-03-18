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

// SMESH SMESHGUI : Displayer for SMESH module
// File   : SMESHGUI_Displayer.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
// SMESH includes
//

#include "SMESHGUI_Displayer.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <SalomeApp_Study.h>
#include <SalomeApp_Application.h>
#include <SUIT_ViewManager.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>


// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Mesh)


SMESHGUI_Displayer::SMESHGUI_Displayer( SalomeApp_Application* app )
: LightApp_Displayer(),
  myApp( app )
{
}

SMESHGUI_Displayer::~SMESHGUI_Displayer()
{
}

SALOME_Prs* SMESHGUI_Displayer::buildPresentation( const QString& entry, SALOME_View* theViewFrame )
{
  SALOME_Prs* prs = 0;

  SALOME_View* aViewFrame = theViewFrame ? theViewFrame : GetActiveView();

  if ( aViewFrame )
  {
    SVTK_Viewer* vtk_viewer = dynamic_cast<SVTK_Viewer*>( aViewFrame );
    if( vtk_viewer )
    {
      SUIT_ViewWindow* wnd = vtk_viewer->getViewManager()->getActiveView();
      SMESH_Actor* anActor = SMESH::FindActorByEntry( wnd, entry.toLatin1().data() );
      if( !anActor )
        anActor = SMESH::CreateActor( study()->studyDS(), entry.toLatin1().data(), true );
      if( anActor )
      {
        SMESH::DisplayActor( wnd, anActor );
        prs = LightApp_Displayer::buildPresentation( entry.toLatin1().data(), aViewFrame );
      }
      if( prs )
        UpdatePrs( prs );
      else if( anActor )
        SMESH::RemoveActor( vtk_viewer->getViewManager()->getActiveView(), anActor );
    }
  }

  return prs;
}

SalomeApp_Study* SMESHGUI_Displayer::study() const
{
  return dynamic_cast<SalomeApp_Study*>( myApp->activeStudy() );
}

bool SMESHGUI_Displayer::canBeDisplayed( const QString& entry, const QString& viewer_type ) const {
  bool res = false;
  if(viewer_type != SVTK_Viewer::Type())
    return res;
  
  SalomeApp_Study* study = dynamic_cast<SalomeApp_Study*>( myApp->activeStudy() );
  if( !study )
    return res;
  
  
  _PTR(SObject) obj = study->studyDS()->FindObjectID( (const char*)entry.toLatin1() );
  CORBA::Object_var anObj = SMESH::SObjectToObject( obj );
  
    /*
    if( !CORBA::is_nil( anObj ) ) {
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( anObj );
    if ( ! mesh->_is_nil() )
    res = (mesh->NbNodes() > 0);
    
    SMESH::SMESH_subMesh_var aSubMeshObj = SMESH::SMESH_subMesh::_narrow( anObj );
    if ( !aSubMeshObj->_is_nil() )
    res = (aSubMeshObj->GetNumberOfNodes(true) > 0);
    
    SMESH::SMESH_GroupBase_var aGroupObj = SMESH::SMESH_GroupBase::_narrow( anObj );
    if ( !aGroupObj->_is_nil() )
    res = !aGroupObj->IsEmpty();
    }*/
  if( !CORBA::is_nil( anObj ) ) {
    if(!SMESH::SMESH_Mesh::_narrow( anObj )->_is_nil() ||
       !SMESH::SMESH_subMesh::_narrow( anObj )->_is_nil() ||
       !SMESH::SMESH_GroupBase::_narrow( anObj )->_is_nil())
    res = true;
  }
  return res;
}
