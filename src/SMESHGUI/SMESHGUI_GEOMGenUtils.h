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
// File   : SMESHGUI_GEOMGenUtils.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_GEOMGENUTILS_H
#define SMESHGUI_GEOMGENUTILS_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>

// GUI includes
#include <SALOME_InteractiveObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)

class SALOMEDSClient_SObject;
class QString;

namespace SMESH
{
  SMESHGUI_EXPORT GEOM::GEOM_Gen_var    GetGEOMGen();

  SMESHGUI_EXPORT GEOM::GEOM_Object_var GetShapeOnMeshOrSubMesh( _PTR(SObject), bool* isMesh=0 );

  SMESHGUI_EXPORT GEOM::GEOM_Object_var GetGeom( _PTR(SObject) );

  SMESHGUI_EXPORT char* GetGeomName( _PTR(SObject) smeshSO );

  SMESHGUI_EXPORT GEOM::GEOM_Object_ptr GetSubShape( GEOM::GEOM_Object_ptr, long );

  SMESHGUI_EXPORT bool GetGeomEntries( Handle(SALOME_InteractiveObject)& hypIO,
                                       QString& subGeom, QString& meshGeom);
}

#endif // SMESHGUI_GEOMGENUTILS_H
