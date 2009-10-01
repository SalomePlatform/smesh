//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : Displayer for SMESH module
// File   : SMESHGUI_Displayer.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Displayer.h"

#include "SMESHGUI_VTKUtils.h"

// SALOME GUI includes
#include <SalomeApp_Study.h>
#include <SalomeApp_Application.h>
#include <SUIT_ViewManager.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

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

bool SMESHGUI_Displayer::canBeDisplayed( const QString& /*entry*/, const QString& viewer_type ) const
{
  return viewer_type==SVTK_Viewer::Type();
}
