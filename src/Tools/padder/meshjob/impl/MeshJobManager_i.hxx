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

#ifndef _MESHJOBMANAGER_HXX_
#define _MESHJOBMANAGER_HXX_

// include the stubs generating from MESHJOB.idl
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(MESHJOB)
#include CORBA_SERVER_HEADER(SALOME_Component)
#include "SALOME_Component_i.hxx"

#include "SALOME_Launcher.hxx"
#include <vector>
#include <string>
#include <map>

#ifdef WIN32
 #if defined MESHJOBMANAGERENGINE_EXPORTS || defined MeshJobManagerEngine_EXPORTS
  #define MESHJOBMANAGERENGINE_EXPORT __declspec( dllexport )
 #else
  #define MESHJOBMANAGERENGINE_EXPORT __declspec( dllimport )
 #endif
#else
 #define MESHJOBMANAGERENGINE_EXPORT
#endif


class MESHJOBMANAGERENGINE_EXPORT MeshJobManager_i: public virtual POA_MESHJOB::MeshJobManager,
                                                    public Engines_Component_i
{
public:
  MeshJobManager_i(CORBA::ORB_ptr orb, PortableServer::POA_ptr poa,
                   PortableServer::ObjectId * contId,
                   const char *instanceName, const char *interfaceName);
  ~MeshJobManager_i();

  bool           configure  (const char *configId,
                             const MESHJOB::ConfigParameter & configParameter);
  CORBA::Long    initialize (const MESHJOB::MeshJobParameterList & meshJobParameterList,
                             const char *configId);
  bool           start      (CORBA::Long jobId);
  char*          getState   (CORBA::Long jobId);
  MESHJOB::MeshJobResults * finalize(CORBA::Long jobId);
  MESHJOB::MeshJobPaths *   getPaths(CORBA::Long jobId);
  bool           clean      (CORBA::Long jobId);

  char*          getLastErrorMessage();

  static long JOBID_UNDEFINED;

private:
  Engines::SalomeLauncher_var   _salomeLauncher;
  Engines::ResourcesManager_var _resourcesManager;
  
  // This maps the config identifier to the config parameters. A
  // config is a resource with additionnal data specifying the
  // location of the binary program to be executed by the task
  std::map<std::string, MESHJOB::ConfigParameter> _configMap;

  // This maps a job identifier to its associated datetime tag. When
  // a job is created during the initialize function, a datetime tag
  // is associated to this job and can be used to characterized files
  // and directories associated to this job.
  std::map<long, long> _jobDateTimeMap;
  std::map<long, MESHJOB::MeshJobPaths*> _jobPathsMap;

  const char* _writeDataFile   (std::vector<MESHJOB::MeshJobParameter> listConcreteMesh,
                                std::vector<MESHJOB::MeshJobParameter> listSteelBarMesh);
  const char* _writeScriptFile (const char * dataFileName, const char * configId);

  std::vector<std::string> * _getResourceNames();

  std::string _lastErrorMessage;

};

#endif

