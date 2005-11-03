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
//  File   : SMESHGUI_Dialog.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$


#ifndef SMESHGUI_DIALOG_H
#define SMESHGUI_DIALOG_H

#include <LightApp_Dialog.h>

class SUIT_Desktop;
class SUIT_ResourceMgr;

//=================================================================================
// class    : SMESHGUI_Dialog
// purpose  : Base dialog for all SMESHGUI dialogs
//=================================================================================
class SMESHGUI_Dialog : public LightApp_Dialog
{
  Q_OBJECT
  
public:
  SMESHGUI_Dialog( QWidget* = 0, const bool = false, const bool = false, const int = OK | Close | Apply );
  virtual ~SMESHGUI_Dialog();

  virtual void show();

  //! set all content to enable (parameter is true) or disable state
  virtual void setContentActive( const bool ) const;

  //! Return hard-coded prefix using to differ overlapping types
  static int   prefix( const QString& );

protected:
  //! find desktop of active application
  SUIT_Desktop* desktop() const;
  
  //! gets resource manager
  SUIT_ResourceMgr* resourceMgr();
};

#endif
