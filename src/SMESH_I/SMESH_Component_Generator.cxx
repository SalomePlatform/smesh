// Copyright (C) 2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMESH_Component_Generator.hxx"

#include "SMESH_Gen_No_Session_i.hxx"
#include "SALOME_Container_i.hxx"
#include "SALOME_KernelServices.hxx"

#include <cstring>

static Engines::EngineComponent_var _unique_compo;

Engines::EngineComponent_var RetrieveSMESHInstance()
{
  if (CORBA::is_nil(_unique_compo))
  {
    CORBA::ORB_var orb;
    {
      int argc(0);
      orb = CORBA::ORB_init(argc, nullptr);
    }
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var pman = poa->the_POAManager();
    CORBA::PolicyList policies;
    policies.length(0);
    Engines_Container_i *cont(KERNEL::getContainerSA());
    PortableServer::ObjectId *conId(cont->getCORBAId());
    //
    pman->activate();
    //
    SMESH_Gen_No_Session_i *servant = new SMESH_Gen_No_Session_i(orb, poa, conId, "SMESH_inst_2", "SMESH");
    PortableServer::ObjectId *zeId = servant->getId();
    CORBA::Object_var zeRef = poa->id_to_reference(*zeId);
    _unique_compo = Engines::EngineComponent::_narrow(zeRef);
  }
  return _unique_compo;
}

Engines::EngineComponent_var GetSMESHInstanceHasThis()
{
  return _unique_compo;
}
