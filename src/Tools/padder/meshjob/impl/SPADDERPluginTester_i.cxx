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

#include "SPADDERPluginTester_i.hxx"

#include <SALOME_NamingService.hxx>
#include <Utils_SALOME_Exception.hxx>

// For standard logging
#include "Basics_Utils.hxx"

//
// ==========================================================================
// Implementation of the SPADDER component interface
// ==========================================================================
//
using namespace std;

/*!
 * Constructor for component "SPADDER" instance
 */
SPADDERPluginTester_i::SPADDERPluginTester_i(CORBA::ORB_ptr orb,
                                             PortableServer::POA_ptr poa,
                                             PortableServer::ObjectId * contId,
                                             const char *instanceName,
                                             const char *interfaceName)
  : Engines_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  LOG("Activating SPADDERPluginTester_i::SPADDERPluginTester object");
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);
}

//! Destructor for component "SPADDER" instance
SPADDERPluginTester_i::~SPADDERPluginTester_i()
{
  std::cerr << "SPADDERPluginTester destruction" << std::endl;
}

/*!
 * This test is just to check the component SPADDER.
 */
void SPADDERPluginTester_i::demo(CORBA::Double a,CORBA::Double b,CORBA::Double& c)
{
  beginService("SPADDERPluginTester_i::demo");
  try {
    //BODY
    
    std::cerr << "a: " << a << std::endl;
    std::cerr << "b: " << b << std::endl;
    c=a+b;
    std::cerr << "c: " << c << std::endl;
    
  }
  catch ( const SALOME_Exception & ex) {
    SALOME::ExceptionStruct es;
    es.text=CORBA::string_dup(ex.what());
    es.type=SALOME::INTERNAL_ERROR;
    throw SALOME::SALOME_Exception(es);
  }
  catch ( const SALOME::SALOME_Exception & ex) {
    throw;
  }
  catch (...) {
    std::cerr << "unknown exception" << std::endl;
    SALOME::ExceptionStruct es;
    es.text=CORBA::string_dup(" unknown exception");
    es.type=SALOME::INTERNAL_ERROR;
    throw SALOME::SALOME_Exception(es);
  }
  endService("SPADDERPluginTester_i::demo");
}

#ifdef LOG
#undef LOG
#endif

#include <SALOME_KernelServices.hxx>
#include "SALOME_Launcher.hxx"
bool SPADDERPluginTester_i::testkernel()
{
  beginService("SPADDERPluginTester_i::testplugin");

  Engines::SalomeLauncher_ptr salomeLauncher = KERNEL::getSalomeLauncher();
  if ( salomeLauncher ) salomeLauncher = NULL;

  endService("SPADDERPluginTester_i::testplugin");
  return true;
}


#include <SMESH_Gen_i.hxx>
#include <SMESH_Gen.hxx>
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SALOMEDS)

#include <utilities.h>

/*!
 * This test checks the constructor of the basic classes of the SMESH
 * plugin for PADDER.
 */
bool SPADDERPluginTester_i::testsmesh(CORBA::Long studyId)
{
  beginService("SPADDERPluginTester_i::testsmesh");

  // Resolve the SMESH engine and the SALOME study
  // _WARN_ The SMESH engine should have been loaded first
  SMESH_Gen_i* smeshGen_i = SMESH_Gen_i::GetSMESHGen();
  CORBA::Object_var anObject = smeshGen_i->GetNS()->Resolve("/myStudyManager");
  SALOMEDS::StudyManager_var aStudyMgr = SALOMEDS::StudyManager::_narrow(anObject);
  SALOMEDS::Study_var myStudy = aStudyMgr->GetStudyByID(studyId);

  //
  // _MEM_ CAUTION: SMESH_Gen define a data structure for local usage
  // while SMESH_Gen_i is the implementation of the SMESH_Gen IDL
  // interface.
  //

  endService("SPADDERPluginTester_i::testsmesh");
  return true;
}

//
// ==========================================================================
// Factory services
// ==========================================================================
//
extern "C"
{
  PortableServer::ObjectId * SPADDERPluginTesterEngine_factory( CORBA::ORB_ptr orb,
                                                                PortableServer::POA_ptr poa,
                                                                PortableServer::ObjectId * contId,
                                                                const char *instanceName,
                                                                const char *interfaceName)
  {
    MESSAGE("PortableServer::ObjectId * SPADDERPluginTesterEngine_factory()");
    SPADDERPluginTester_i * myEngine = new SPADDERPluginTester_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
