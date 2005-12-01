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
//  File   : SMESHGUI_Displayer.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: /home/server/cvs/SMESH/SMESH_SRC/src/SMESHGUI/SMESHGUI_Displayer.h

#ifndef SMESHGUI_DISPLAYER_HEADER
#define SMESHGUI_DISPLAYER_HEADER

#include <LightApp_Displayer.h>

class SalomeApp_Application;
class SalomeApp_Study;

class SMESHGUI_Displayer : public LightApp_Displayer
{
public:
  SMESHGUI_Displayer( SalomeApp_Application* );
  ~SMESHGUI_Displayer();

  virtual SALOME_Prs* buildPresentation( const QString&, SALOME_View* = 0 );
  virtual bool        canBeDisplayed( const QString& /*entry*/, const QString& /*viewer_type*/ ) const;

protected:
  SalomeApp_Study* study() const;

private:
  SalomeApp_Application*   myApp;
};

#endif
