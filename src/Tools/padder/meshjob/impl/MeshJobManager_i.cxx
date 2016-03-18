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

#ifdef WIN32
#include <winsock2.h>
#include <windows.h> 
#else
#include <sys/time.h>
#endif

#include "MeshJobManager_i.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOME_Exception)


#include "Basics_Utils.hxx"         // For standard logging
#undef LOG
#include "SALOME_KernelServices.hxx"   // For CORBA logging
#undef LOG

#define LOG STDLOG

//
// ====================================================================
// General purpose helper functions (to put elsewhere at least)
// ====================================================================
//

/*!
 * This function must be used to associate a datetime tag to a job
 */

#ifndef WIN32
static long timetag() {
  timeval tv;
  gettimeofday(&tv,0);
  long tag = tv.tv_usec + tv.tv_sec*1000000;
  return tag;
}
#endif

/*!
 * This function returns true if the string text starts with the string
 * token.
 */
static bool myStartsWith(const std::string& text,const std::string& token){     
  if(text.length() < token.length())
    return false;
  return (text.compare(0, token.length(), token) == 0);
}

/*!
 * This function returns true if the file exists on the local file
 * system.
 */
#include <iostream>
#include <fstream>
static bool fexists(const char *filename)
{
  std::ifstream ifile(filename);
  if ((bool)ifile && ifile.good()) {
    return true;
  }
  return false;
}

//
// ====================================================================
// Constructor/Destructor
// ====================================================================
//
MeshJobManager_i::MeshJobManager_i(CORBA::ORB_ptr orb,
                                   PortableServer::POA_ptr poa,
                                   PortableServer::ObjectId * contId,
                                   const char *instanceName,
                                   const char *interfaceName)
  : Engines_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  LOG("Activating MESHJOB::MeshJobManager object");
  _thisObj = this ;
  _id = _poa->activate_object(_thisObj);

  _salomeLauncher   = KERNEL::getSalomeLauncher();
  if(CORBA::is_nil(_salomeLauncher)){
    LOG("The SALOME launcher can't be reached ==> STOP");
    throw KERNEL::createSalomeException("SALOME launcher can't be reached");
  }

  _resourcesManager = KERNEL::getResourcesManager();
  if(CORBA::is_nil(_resourcesManager)){
    LOG("The SALOME resource manager can't be reached ==> STOP");
    throw KERNEL::createSalomeException("The SALOME resource manager can't be reached");
  }

  _lastErrorMessage = "";
}

MeshJobManager_i::~MeshJobManager_i() {
  LOG("MeshJobManager_i::~MeshJobManager_i()");
}

//
// ====================================================================
// Helper functions to deals with the local and remote file systems
// ====================================================================
//
#include <fstream>     // to get the file streams
#ifdef WIN32
#include <stdlib.h>    // to get _splitpath
#include <direct.h>    // to get _mkdir
#else
#include <unistd.h>    // to get basename
#include <sys/stat.h>  // to get mkdir
#include <sys/types.h> // to get mkdir options
#endif

#include <stdlib.h>    // to get system and getenv

static std::string OUTPUTFILE("output.med");
static std::string DATAFILE("data.txt");
static std::string SCRIPTFILE("padder.sh");
static std::string SEPARATOR(" ");

#ifdef WIN32
static std::string USER(getenv("USERNAME"));
#else
static std::string USER(getenv("USER"));
#endif

static std::string LOCAL_INPUTDIR("/tmp/spadder.local.inputdir."+USER);
static std::string LOCAL_RESULTDIR("/tmp/spadder.local.resultdir."+USER);
static std::string REMOTE_WORKDIR("/tmp/spadder.remote.workdir."+USER);

/*!
 * This function creates the padder text input file containing the
 * input data (list of filenames and groupnames) and returns the path
 * of the created file. This function is the one that knows the format
 * of the padder input file. If the input file format changes, then
 * this function (and only this one) should be updated. The file
 * format is the following ([] means that the variable is optional):
 *
 * [<concreteMeshFile>   <concreteGroupName>]
 * nbSteelBarMeshes <N>
 * <steelBarMeshFile_1>   <steelBarGroupName_1>
 * <steelBarMeshFile_2>   <steelBarGroupName_2>
 * ...
 * <steelBarMeshFile_N>   <steelBarGroupName_N>
 * <outputMedFile>
 */
const char * MeshJobManager_i::_writeDataFile(std::vector<MESHJOB::MeshJobParameter> listConcreteMesh,
                                              std::vector<MESHJOB::MeshJobParameter> listSteelBarMesh) {
#ifdef WIN32
  _mkdir(LOCAL_INPUTDIR.c_str());
#else
  mkdir(LOCAL_INPUTDIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  // Make it static so that it's allocated once (constant name)
  static std::string * dataFilename = new std::string(LOCAL_INPUTDIR+"/"+DATAFILE);
  std::ofstream dataFile(dataFilename->c_str());

  // Note that we use here the basename of the files because the files
  // are supposed to be copied in the REMOTE_WORKDIR for execution.
  std::string line;

  // We first specify the concrete mesh data (filename and groupname)
  if ( listConcreteMesh.size() > 0 ) {
#ifdef WIN32
    char fname[ _MAX_FNAME ];
    _splitpath( listConcreteMesh[0].file_name, NULL, NULL, fname, NULL );
    char* bname = &fname[0];
#else
    char* bname = basename(listConcreteMesh[0].file_name);
#endif
    line = std::string(bname) + " " + std::string(listConcreteMesh[0].group_name);
    dataFile << line.c_str() << std::endl;
  }
  // Then, we can specify the steelbar mesh data, starting by the
  // number of meshes
  int nbSteelBarMeshes=listSteelBarMesh.size();
  line = std::string("nbSteelBarMeshes") + SEPARATOR + ToString(nbSteelBarMeshes);
  dataFile << line.c_str() << std::endl;
  for (int i=0; i<nbSteelBarMeshes; i++) {
#ifdef WIN32
        char fname[ _MAX_FNAME ];
        _splitpath( listSteelBarMesh[i].file_name, NULL, NULL, fname, NULL );
        char* bname = &fname[0];
#else
        char* bname = basename(listSteelBarMesh[i].file_name);
#endif
    line = std::string(bname) + " " + std::string(listSteelBarMesh[i].group_name);
    dataFile << line.c_str() << std::endl;
  }
  
  // Finally, we conclude with the name of the output file
  line = OUTPUTFILE;
  dataFile << line.c_str() << std::endl;
  dataFile.close();
  return dataFilename->c_str();  
}

/*!
 * This function creates a shell script that runs padder whith the
 * specified data file, and returns the path of the created script
 * file. The config id is used to retrieve the path to the binary file
 * and other required files.
 */
const char* MeshJobManager_i::_writeScriptFile(const char * dataFileName, const char * configId) {
#ifdef WIN32
  _mkdir(LOCAL_INPUTDIR.c_str());
#else
  mkdir(LOCAL_INPUTDIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  // Make it static so that it's allocated once (constant name)
  static std::string * scriptFilename = new std::string(LOCAL_INPUTDIR+"/"+SCRIPTFILE);

  char * binpath = _configMap[configId].binpath;
  char * envpath = _configMap[configId].envpath;

#ifdef WIN32
        char fname[ _MAX_FNAME ];
        _splitpath( dataFileName, NULL, NULL, fname, NULL );
        const char* bname = &fname[0];
#else
        const char* bname = basename(dataFileName);
#endif


  std::ofstream script(scriptFilename->c_str());
  script << "#!/bin/sh"                                   << std::endl;
  script << "here=$(dirname $0)"                          << std::endl;
  script << ". " << envpath                               << std::endl;
  script << binpath << " $here/" << bname                 << std::endl;
  // Note that we use the basename of the datafile because all data
  // files are supposed to have been copied in the REMOTE_WORKDIR.
  script.close();
  return scriptFilename->c_str();
}

//
// ====================================================================
// Functions to initialize and supervise the mesh computation job
// ====================================================================
//
bool MeshJobManager_i::configure(const char *configId,
                                 const MESHJOB::ConfigParameter & configParameter)
{
  beginService("MeshJobManager_i::configure");
  
  _configMap[configId] = configParameter;

  LOG("Adding configuration for " << configId);
  LOG("- binpath = " << _configMap[configId].binpath);
  LOG("- envpath = " << _configMap[configId].envpath);

  endService("MeshJobManager_i::configure");
  return true;
}

long MeshJobManager_i::JOBID_UNDEFINED = -1;

/*! Initialize a smesh computation job and return the job identifier */
CORBA::Long MeshJobManager_i::initialize(const MESHJOB::MeshJobParameterList & meshJobParameterList,
                                         const char * configId)
{
  beginService("MeshJobManager_i::initialize");
  //
  // We first analyse the CORBA sequence to store data in C++ vectors
  //
  std::vector<MESHJOB::MeshJobParameter> listConcreteMesh;
  std::vector<MESHJOB::MeshJobParameter> listSteelBarMesh;
  for(CORBA::ULong i=0; i<meshJobParameterList.length(); i++) {
    MESHJOB::MeshJobParameter currentMesh = meshJobParameterList[i];
    switch ( currentMesh.file_type ) {
    case MESHJOB::MED_CONCRETE:
      listConcreteMesh.push_back(currentMesh);
      break;
    case MESHJOB::MED_STEELBAR:
      listSteelBarMesh.push_back(currentMesh);
      break;
    default:
      _lastErrorMessage =
        std::string("The type of the file ")+
        std::string(currentMesh.file_name)+
        std::string(" is not recognized");
      LOG(_lastErrorMessage);
      return JOBID_UNDEFINED;
    }
  }
  
  // It is not possible to specify more than one concrete
  // file. Converselly, it is possible to specify no concrete file.
  if ( listConcreteMesh.size() > 1 ) {
    // Not consistent with the specification
    _lastErrorMessage = std::string("You specify more than one concrete mesh (not authorized)");
    LOG(_lastErrorMessage);
    return JOBID_UNDEFINED;
  }
  
  LOG("Nb. concrete mesh = " << listConcreteMesh.size());
  LOG("Nb. steelbar mesh = " << listSteelBarMesh.size());

  // We initiate here a datetime to tag the files and folder
  // associated to this job.
#ifdef WIN32
  DWORD jobDatetimeTag = timeGetTime();
#else
  long jobDatetimeTag = timetag();
#endif
  // And a MESHJOB::MeshJobPaths structure to hold the directories
  // where to find data
  MESHJOB::MeshJobPaths * jobPaths = new MESHJOB::MeshJobPaths();
  jobPaths->local_inputdir  = LOCAL_INPUTDIR.c_str();
  jobPaths->local_resultdir = (LOCAL_RESULTDIR + "." + ToString(jobDatetimeTag)).c_str();
  jobPaths->remote_workdir  = (REMOTE_WORKDIR + "." + ToString(jobDatetimeTag)).c_str();  

  //
  // Then, we have to create the padder input data file. This input
  // data is a text file containing the list of file names and group
  // names.
  //
  const char * dataFilename = this->_writeDataFile(listConcreteMesh, listSteelBarMesh);
  LOG("dataFilename = " << dataFilename);
  const char * scriptFilename = this->_writeScriptFile(dataFilename, configId);
  LOG("scriptFilename = " << scriptFilename);

  //
  // Then, the following instructions consists in preparing the job
  // parameters to request the SALOME launcher for creating a new
  // job.
  //
  Engines::JobParameters_var jobParameters = new Engines::JobParameters;
  jobParameters->job_type = CORBA::string_dup("command");
  // CAUTION: the job_file must be a single filename specifying a
  // self-consistent script to be executed without any argument on the
  // remote host.
  jobParameters->job_file = CORBA::string_dup(scriptFilename);

  //
  // Specification of the working spaces:
  //
  // - local_directory: can be used to specify where to find the input
  //   files on the local resource. It's optionnal if you specify the
  //   absolute path name of input files.
  //
  // - result_directory: must be used to specify where to download the
  //   output files on the local resources
  //
  // - work_directory: must be used to specify the remote directory
  //   where to put all the stuff to run the job. Note that the job
  //   will be executed from within this directory, i.e. a change
  //   directory toward this working directory is done by the batch
  //   system before running the specified job script.
  //
  jobParameters->local_directory  = CORBA::string_dup("");
  jobParameters->result_directory = CORBA::string_dup(jobPaths->local_resultdir);
  jobParameters->work_directory   = CORBA::string_dup(jobPaths->remote_workdir);

  // We specify the input files that are required to execute the
  // job_file. If basenames are specified, then the files are supposed
  // to be located in local_directory.
  int nbcmesh = listConcreteMesh.size();
  int nbsmesh = listSteelBarMesh.size();
  int nbFiles = nbsmesh+nbcmesh+1;
  // The number of input file is: 
  //   (nb. of steelbar meshfile)
  // + (1 or 0 concrete meshfile)
  // + (1 padder input file)
  jobParameters->in_files.length(nbFiles);
  for (int i=0; i<nbcmesh; i++) {
    jobParameters->in_files[i] = CORBA::string_dup(listConcreteMesh[i].file_name);
  }
  for (int i=0; i<nbsmesh; i++) {
    jobParameters->in_files[nbcmesh+i] = CORBA::string_dup(listSteelBarMesh[i].file_name);
  }
  jobParameters->in_files[nbcmesh+nbsmesh] = CORBA::string_dup(dataFilename);
  // Note that all these input files will be copied in the
  // REMOTE_WORKDIR on the remote host. At this step, they should
  // all exist, so we can check their presence on the local
  // filesystem.
  for (int i=0; i<nbFiles; i++) {
    if ( fexists(jobParameters->in_files[i]) != true ) {
      _lastErrorMessage = std::string("The input file ") + std::string(jobParameters->in_files[i]);
      _lastErrorMessage+= std::string(" does not exists. Can't initialize the job");
      LOG(_lastErrorMessage);
      return JOBID_UNDEFINED;      
    }
  }

  // Then, we have to specify the existance of an output filename. The
  // path is supposed to be a path on the remote resource, i.e. where
  // the job is executed.
  jobParameters->out_files.length(1);
  std::string outputfile_name = std::string(jobPaths->remote_workdir)+"/"+OUTPUTFILE;
  jobParameters->out_files[0] = CORBA::string_dup(outputfile_name.c_str());

  // CAUTION: the maximum duration has to be set with a format like "hh:mm"
  //jobParameters->maximum_duration = CORBA::string_dup("01:00");
  jobParameters->queue = CORBA::string_dup("");

  // Setting resource and additionnal properties (if needed)
  // The resource parameters can be initiated from scratch, for
  // example by specifying the values in hard coding:
  // >>>
  //jobParameters->resource_required.name = CORBA::string_dup("localhost");
  //jobParameters->resource_required.hostname = CORBA::string_dup("localhost");
  //jobParameters->resource_required.mem_mb = 1024 * 10;
  //jobParameters->resource_required.nb_proc = 1;
  // <<<
  // But it's better to initiate these parameters from a resource
  // definition known by the resource manager. This ensures that the
  // resource will be available:
  //const char * resourceName = "localhost";
  //const char * resourceName = "boulant@claui2p1";
  //const char * resourceName = "nepal@nepal";
  const char * resourceName = _configMap[configId].resname;
  
  Engines::ResourceDefinition * resourceDefinition;
  try {
    resourceDefinition = _resourcesManager->GetResourceDefinition(resourceName);
  }
  catch (const CORBA::SystemException& ex) {
    _lastErrorMessage = std::string("We can not access to the ressource ") + std::string(resourceName);
    _lastErrorMessage+= std::string("(check the file CatalogResource.xml)");
    LOG(_lastErrorMessage);
    return JOBID_UNDEFINED;
  }
  // CAUTION: This resource should have been defined in the
  // CatalogResource.xml associated to the SALOME application.
  //
  // Then, the values can be used to initiate the resource parameters
  // of the job:
  jobParameters->resource_required.name     = CORBA::string_dup(resourceDefinition->name.in());
  // CAUTION: the additionnal two following parameters MUST be
  // specified explicitly, because they are not provided by the
  // resource definition:
  jobParameters->resource_required.mem_mb   = resourceDefinition->mem_mb;
  jobParameters->resource_required.nb_proc  = resourceDefinition->nb_proc_per_node;
  // CAUTION: the parameter mem_mb specifies the maximum memory value
  // that could be allocated for executing the job. This takes into
  // account not only the data that could be loaded by the batch
  // process but also the linked dynamic library.
  //
  // A possible problem, for exemple in the case where you use the ssh
  // emulation of a batch system, is to get an error message as below
  // when libBatch try to run the ssh command:
  //
  // ## /usr/bin/ssh: error while loading shared libraries: libcrypto.so.0.9.8: failed
  // ## to map segment from shared object: Cannot allocate memory
  //
  // In this exemple, the mem_mb was set to 1MB, value that is not
  // sufficient to load the dynamic libraries linked to the ssh
  // executable (libcrypto.so in the error message).
  //
  // So, even in the case of a simple test shell script, you should
  // set this value at least to a standard threshold as 500MB
  int jobId = JOBID_UNDEFINED;
  try {
    jobId = _salomeLauncher->createJob(jobParameters);
    // We register the datetime tag of this job
    _jobDateTimeMap[jobId]=jobDatetimeTag;
    _jobPathsMap[jobId] = jobPaths;
  }
  catch (const SALOME::SALOME_Exception & ex) {
    LOG("SALOME Exception at initialization step !" <<ex.details.text.in());
    _lastErrorMessage = ex.details.text.in();
    return JOBID_UNDEFINED;
  }
  catch (const CORBA::SystemException& ex) {
    LOG("Receive SALOME System Exception: "<<ex);
    LOG("Check SALOME servers...");
    _lastErrorMessage = "Check the SALOME servers (or try to restart SALOME)";
    return JOBID_UNDEFINED;
  }
  
  endService("MeshJobManager_i::initialize");
  return jobId;
}

/*! Submit the job execution and return true if submission is OK */
bool MeshJobManager_i::start(CORBA::Long jobId) {
  beginService("MeshJobManager_i::start");

  try {
    _salomeLauncher->launchJob(jobId);
  }
  catch (const SALOME::SALOME_Exception & ex) {
    LOG("SALOME Exception in launchjob !" <<ex.details.text.in());
    _lastErrorMessage = ex.details.text.in();
    return false;
  }
  catch (const CORBA::SystemException& ex) {
    LOG("Receive SALOME System Exception: "<<ex);
    LOG("Check SALOME servers...");
    _lastErrorMessage = "Check the SALOME servers (or try to restart SALOME)";
    return false;
  }

  endService("MeshJobManager_i::initialize");
  return true;
}

/*! Request the launch manager for the state of the specified job */
char* MeshJobManager_i::getState(CORBA::Long jobId) {
  beginService("MeshJobManager_i::getState");

  std::string state;
  try
  {
    state = _salomeLauncher->getJobState(jobId);
  }
  catch (const SALOME::SALOME_Exception & ex)
  {
    LOG("SALOME Exception in getJobState !");
    _lastErrorMessage = ex.details.text.in();
    state = ex.details.text;
  }
  catch (const CORBA::SystemException& ex)
  {
    LOG("Receive SALOME System Exception: " << ex);
    state="SALOME System Exception - see logs";
  }
  LOG("jobId="<<ToString(jobId)<<" state="<<state);
  endService("MeshJobManager_i::getState");
  return CORBA::string_dup(state.c_str());
}

MESHJOB::MeshJobPaths * MeshJobManager_i::getPaths(CORBA::Long jobId) {

  MESHJOB::MeshJobPaths * jobPaths = _jobPathsMap[jobId];
  if ( jobPaths == NULL ) {
    LOG("You request the working paths for an undefined job (jobId="<<ToString(jobId)<<")");
    return NULL; // Maybe raise an exception?
  }
  return jobPaths;
}


MESHJOB::MeshJobResults * MeshJobManager_i::finalize(CORBA::Long jobId) {
  beginService("MeshJobManager_i::getResults");
  MESHJOB::MeshJobResults * result = new MESHJOB::MeshJobResults();

  MESHJOB::MeshJobPaths * jobPaths = this->getPaths(jobId);
  std::string local_resultdir(jobPaths->local_resultdir);
  result->results_dirname = local_resultdir.c_str();  
  try
  {
    _salomeLauncher->getJobResults(jobId, local_resultdir.c_str());
 
    // __BUG__: to prevent from a bug of the MED driver (SALOME
    // 5.1.5), we change the basename of the output file to force the
    // complete reloading of data by the med driver.
    long jobDatetimeTag = _jobDateTimeMap[jobId];
    std::string outputFileName = "output"+ToString(jobDatetimeTag)+".med";
    rename((local_resultdir+"/"+OUTPUTFILE).c_str(), (local_resultdir+"/"+outputFileName).c_str());

    result->outputmesh_filename = outputFileName.c_str();
    
    if ( fexists( (local_resultdir+"/"+outputFileName).c_str()  ) != true ) {
      _lastErrorMessage = std::string("The result file ")+
        std::string((local_resultdir+"/"+outputFileName).c_str())+
        std::string(" has not been created.");
      result->status = false;
    }
    else {
      result->status = true;
    }
 }
  catch (const SALOME::SALOME_Exception & ex)
  {
    _lastErrorMessage = ex.details.text.in();
    LOG(_lastErrorMessage);
    result->status = false;
  }
  catch (const CORBA::SystemException& ex)
  {
    _lastErrorMessage = "The SALOME launcher can not retrieve the result data";
    LOG(_lastErrorMessage);
    result->status = false;
  }
  endService("MeshJobManager_i::getResults");
  return result;
}


/*! Clean all data associated to this job and remove the job from the launch manager */
bool MeshJobManager_i::clean(CORBA::Long jobId) {
  beginService("MeshJobManager_i::clean");
  
  // __GBO__ WORK IN PROGRESS: we just clean the temporary local
  // directories. The remote working directories are tag with the
  // execution datetime and the we prevent the task from conflict
  // with files of another task.
  MESHJOB::MeshJobPaths * jobPaths = this->getPaths(jobId);
  if ( jobPaths == NULL ) return false;

  // WARN: !!!!!
  // For safety reason (and prevent from bug that could erase the
  // filesystem), we cancel the operation in the case where the
  // directories to delete are not in the /tmp folder.
  std::string shell_command("rm -rf ");
  std::string inputdir(jobPaths->local_inputdir);
  std::string resultdir(jobPaths->local_resultdir);
  if ( !myStartsWith(inputdir,"/tmp/") )  {
    LOG("WRN: The directory "<<inputdir<<" is not in /tmp. NO DELETE is done");
  } else {
    shell_command+=inputdir+" ";
  }
  if ( !myStartsWith(resultdir,"/tmp/"))  {
    LOG("WRN: The directory "<<resultdir<<" is not in /tmp. NO DELETE is done");
  } else {
    shell_command+=resultdir;
  }

  LOG("DBG: clean shell command = "<<shell_command);

  bool cleanOk = false;
  int error = system(shell_command.c_str());
  if (error == 0) cleanOk = true;

  endService("MeshJobManager_i::clean");
  return cleanOk;
}


std::vector<std::string> * MeshJobManager_i::_getResourceNames() {

  //
  // These part is just to control the available resources
  //
  Engines::ResourceParameters params;
  KERNEL::getLifeCycleCORBA()->preSet(params);

  Engines::ResourceList * resourceList = _resourcesManager->GetFittingResources(params);
  Engines::ResourceDefinition * resourceDefinition = NULL;
  LOG("### resource list:");
  std::vector<std::string>* resourceNames = new std::vector<std::string>();
  if (resourceList) {
    for ( size_t i = 0; i < resourceList->length(); i++) {
      const char* aResourceName = (*resourceList)[i];
      resourceNames->push_back(std::string(aResourceName));
      LOG("resource["<<i<<"] = "<<aResourceName);
      resourceDefinition = _resourcesManager->GetResourceDefinition(aResourceName);
      LOG("protocol["<<i<<"] = "<<resourceDefinition->protocol);
    }
  }

  // Note: a ResourceDefinition is used to create a batch configuration
  // in the Launcher. This operation is done at Launcher startup from
  // the configuration file CatalogResources.xml provided by the
  // SALOME application.
  // In the code instructions, you just have to choose a resource
  // configuration by its name and then define the ResourceParameters
  // that specify additionnal properties for a specific job submission
  // (use the attribute resource_required of the JobParameters).

  return resourceNames;
}

char* MeshJobManager_i::getLastErrorMessage() {
  beginService("MeshJobManager_i::getState");
  endService("MeshJobManager_i::getState");
  return CORBA::string_dup(_lastErrorMessage.c_str());
}

//
// ==========================================================================
// Factory services
// ==========================================================================
//
extern "C"
{
  MESHJOBMANAGERENGINE_EXPORT
  PortableServer::ObjectId * MeshJobManagerEngine_factory( CORBA::ORB_ptr orb,
                                                           PortableServer::POA_ptr poa,
                                                           PortableServer::ObjectId * contId,
                                                           const char *instanceName,
                                                           const char *interfaceName)
  {
    LOG("PortableServer::ObjectId * MeshJobManagerEngine_factory()");
    MeshJobManager_i * myEngine = new MeshJobManager_i(orb, poa, contId, instanceName, interfaceName);
    return myEngine->getId() ;
  }
}
