// Copyright (C) 2011-2016  EDF R&D
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

// Authors : Guillaume Boulant (EDF) - 01/03/2011

#ifndef _SPADDER_PLUGINTESTER_HXX_
#define _SPADDER_PLUGINTESTER_HXX_

// include the stubs generating from SPADDERPluginTest.idl
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SPADDERPluginTest)
#include <SALOME_Component.hh>
#include "SALOME_Component_i.hxx"

class SPADDERPluginTester_i:
  public virtual POA_SPADDERPluginTest::SPADDERPluginTester,
  public Engines_Component_i
{
public:
  SPADDERPluginTester_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa,
                        PortableServer::ObjectId * contId,
                        const char *instanceName, const char *interfaceName);
  virtual ~SPADDERPluginTester_i();
  
  void demo(CORBA::Double a,CORBA::Double b,CORBA::Double& c);
  bool testkernel();
  bool testsmesh(CORBA::Long studyId);

};

#endif

