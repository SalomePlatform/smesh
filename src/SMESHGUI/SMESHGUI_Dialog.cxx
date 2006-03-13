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
//  File   : SMESHGUI_Dialog.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include <SMESHGUI_Dialog.h>
#include <SMESH_Type.h>
#include <SUIT_Session.h>
#include <SalomeApp_Application.h>

//=================================================================================
// function : SMESHGUI_Dialog
// purpose  : Constructor
//=================================================================================
SMESHGUI_Dialog::SMESHGUI_Dialog( QWidget* parent, const bool modal,
                                  const bool allowResize, const int flags )
: LightApp_Dialog( parent == 0 ? desktop() : parent, "", modal, allowResize, flags )
{
  int pr = prefix( "SMESH" );
  typeName( pr + MESH ) = tr( "DLG_MESH" );
  typeName( pr + HYPOTHESIS ) = tr( "DLG_HYPO" );
  typeName( pr + ALGORITHM ) = tr( "DLG_ALGO" );
  if ( flags & Close )
    setButtonPosition( Right, Close );
}

//=================================================================================
// function : ~SMESHGUI_Dialog
// purpose  : Destructor
//=================================================================================
SMESHGUI_Dialog::~SMESHGUI_Dialog()
{
}

//=================================================================================
// function : show
// purpose  :
//=================================================================================
void SMESHGUI_Dialog::show()
{
  adjustSize();
  LightApp_Dialog::show();
}

//=================================================================================
// function : setContentActive
// purpose  :
//=================================================================================
void SMESHGUI_Dialog::setContentActive( const bool active ) const
{
  mainFrame()->setEnabled( active );
}

//=================================================================================
// function : desktop
// purpose  :
//=================================================================================
SUIT_Desktop* SMESHGUI_Dialog::desktop() const
{
  SUIT_Desktop* d = 0;
  SUIT_Session* s = SUIT_Session::session();
  if( s )
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( s->activeApplication() );
    if( app )
      d = app->desktop();
  }
  return d;
}

//=======================================================================
// name    : prefix
// Purpose : Get prefix for module types
//=======================================================================
int SMESHGUI_Dialog::prefix( const QString& name )
{
  if( name == "GEOM" )
    return 100;
  else if( name == "SMESH" )
    return 200;
  else if( name == "SMESH element" )
    return 300;
  else
    return 0;
}

//=======================================================================
// name    : resourceMgr
// Purpose : Gets resource manager
//=======================================================================
SUIT_ResourceMgr* resourceMgr()
{
  return SUIT_Session::session()->resourceMgr();
}


















