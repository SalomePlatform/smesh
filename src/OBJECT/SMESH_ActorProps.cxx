// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMESH_ActorProps.h"

#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>

SMESH_ActorProps* SMESH_ActorProps::props()
{
  static SMESH_ActorProps* g_props = 0;
  if (!g_props)
    g_props = new SMESH_ActorProps();
  return g_props;
}

void SMESH_ActorProps::reset()
{
  props()->initialize();
}

SMESH_ActorProps::SMESH_ActorProps()
{
  initialize();
}

void SMESH_ActorProps::initialize()
{
  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();
  m_selection_color = mgr->colorValue("SMESH", "selection_object_color", Qt::white);
  m_highlight_color = mgr->colorValue("SMESH", "highlight_color", Qt::cyan);
  m_selection_increment = qBound(0, mgr->integerValue("SMESH", "selection_increment", 0), 5);
  m_controls_increment = qBound(0, mgr->integerValue("SMESH", "controls_increment", 2), 5);
}

QColor SMESH_ActorProps::selectionColor() const
{
  return m_selection_color;
}

QColor SMESH_ActorProps::highlightColor() const
{
  return m_highlight_color;
}

int SMESH_ActorProps::selectionIncrement() const
{
  return m_selection_increment;
}

int SMESH_ActorProps::controlsIncrement() const
{
  return m_controls_increment;
}
