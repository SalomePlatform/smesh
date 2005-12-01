//  SMESH SMESHGUI : Displayer for SMESH module
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
//  File   : SMESHGUI_Displayer.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: /home/server/cvs/SMESH/SMESH_SRC/src/SMESHGUI/SMESHGUI_Displayer.cxx

#include "SMESHGUI_Displayer.h"
#include "SMESHGUI_VTKUtils.h"

#include <SalomeApp_Study.h>
#include <SalomeApp_Application.h>
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
      SMESH_Actor* anActor = SMESH::FindActorByEntry( wnd, entry.latin1() );
      if( !anActor )
	anActor = SMESH::CreateActor( study()->studyDS(), entry.latin1(), true );
      if( anActor )
      {
	SMESH::DisplayActor( wnd, anActor );
        prs = LightApp_Displayer::buildPresentation( entry.latin1(), aViewFrame );
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
