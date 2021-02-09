// Copyright (C) 2021  CEA/DEN, EDF R&D
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

#pragma once

#include "SMESH_Gen_i.hxx"

class SMESH_I_EXPORT SMESH_Gen_Session_i : public SMESH_Gen_i
{
public:
  SMESH_Gen_Session_i( CORBA::ORB_ptr orb,
               PortableServer::POA_ptr   poa,
               PortableServer::ObjectId* contId,
               const char*               instanceName,
               const char*               interfaceName);
  GEOM::GEOM_Gen_var GetGeomEngine( bool isShaper ) override;
  // Get the SALOMEDS::Study from naming service
  SALOMEDS::Study_var getStudyServantVirtual() const override;
  SALOME_ModuleCatalog::ModuleCatalog_var getModuleCatalog() const override;
};
