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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshUtils.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHUTILS_H
#define SMESHGUI_MESHUTILS_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QString>

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Measurements)

class SALOMEDSClient_SObject;

namespace SMESH
{
  SMESHGUI_EXPORT
    SMESH_Mesh_var GetMeshByIO( const Handle(SALOME_InteractiveObject)& );
 
  SMESHGUI_EXPORT
    QString        UniqueMeshName( const QString&, const QString& = QString() );
  SMESHGUI_EXPORT
    QString        UniqueName( const QString&, _PTR(SObject) = _PTR(SObject)(), const QString& = QString() );

  SMESHGUI_EXPORT  SMESH::Measurements_var& GetMeasurements();
}

#endif // SMESHGUI_MESHUTILS_H
