// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_CreateDualMeshOp.h
// Author : Yoann AUDOUIN (EDF)
//
#ifndef SMESHGUI_CREATEDUALMESHOP_H
#define SMESHGUI_CREATEDUALMESHOP_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_SelectionOp.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI_CreateDualMeshDlg;

class SMESHGUI_EXPORT SMESHGUI_CreateDualMeshOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_CreateDualMeshOp();
  virtual ~SMESHGUI_CreateDualMeshOp();

  virtual LightApp_Dialog*       dlg() const;

  static bool     checkMesh( const SMESH::SMESH_IDSource_var& );
protected:
  virtual void                   startOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;

protected slots:
  virtual bool                   onApply();

private:
  SMESHGUI_CreateDualMeshDlg*        myDlg;
};

#endif // SMESHGUI_CREATEDUALMESHOP_H
