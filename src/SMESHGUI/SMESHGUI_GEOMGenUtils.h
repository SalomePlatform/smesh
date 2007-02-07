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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com

#ifndef SMESHGUI_GEOMGenUtils_HeaderFile
#define SMESHGUI_GEOMGenUtils_HeaderFile

#include "SMESH_SMESHGUI.hxx"

#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(GEOM_Gen)

#include "SALOMEDSClient_definitions.hxx"

class SALOMEDSClient_SObject;

namespace SMESH
{
  SMESHGUI_EXPORT GEOM::GEOM_Gen_var GetGEOMGen();

  SMESHGUI_EXPORT GEOM::GEOM_Object_var GetShapeOnMeshOrSubMesh (_PTR(SObject) theSObject);

  SMESHGUI_EXPORT GEOM::GEOM_Object_ptr GetGeom (_PTR(SObject) theSO);
}

#endif
