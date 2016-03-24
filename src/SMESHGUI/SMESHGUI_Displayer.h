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
// File   : SMESHGUI_Displayer.h
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_DISPLAYER_H
#define SMESHGUI_DISPLAYER_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <LightApp_Displayer.h>

class SalomeApp_Application;
class SalomeApp_Study;

class SMESHGUI_EXPORT SMESHGUI_Displayer : public LightApp_Displayer
{
public:
  SMESHGUI_Displayer( SalomeApp_Application* );
  ~SMESHGUI_Displayer();

  virtual SALOME_Prs*      buildPresentation( const QString&, SALOME_View* = 0 );
  virtual bool             canBeDisplayed( const QString&, const QString& ) const;

protected:
  SalomeApp_Study*         study() const;

private:
  SalomeApp_Application*   myApp;
};

#endif // SMESHGUI_DISPLAYER_H
