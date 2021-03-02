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

#include "SMESH_Gen_No_Session_i.hxx"
#include "SALOMEDS_Study_i.hxx"
#include "SALOME_KernelServices.hxx"
#include "SALOME_Fake_NamingService.hxx"
#include "SALOME_ModuleCatalog_impl.hxx"

SMESH_Gen_No_Session_i::SMESH_Gen_No_Session_i( CORBA::ORB_ptr orb,
                                                PortableServer::POA_ptr   poa,
                                                PortableServer::ObjectId* contId,
                                                const char*               instanceName,
                                                const char*               interfaceName):SMESH_Gen_i(orb,poa,contId,instanceName,interfaceName,false)
{
  myNS = new SALOME_Fake_NamingService;
}

GEOM::GEOM_Gen_var SMESH_Gen_No_Session_i::GetGeomEngine( bool isShaper )
{
  CORBA::Object_var temp = KERNEL::RetrieveCompo(isShaper ? "SHAPERSTUDY" : "GEOM");
  myGeomGen = GEOM::GEOM_Gen::_narrow( temp );
  return myGeomGen;
}

SALOMEDS::Study_var SMESH_Gen_No_Session_i::getStudyServantVirtual() const
{
  return SALOMEDS::Study::_duplicate(KERNEL::getStudyServantSA());
}

SALOME_ModuleCatalog::ModuleCatalog_var SMESH_Gen_No_Session_i::getModuleCatalog() const
{
  SALOME_ModuleCatalog::ModuleCatalog_var aCat = KERNEL::getModuleComponentServantSA();
  return aCat;
}
