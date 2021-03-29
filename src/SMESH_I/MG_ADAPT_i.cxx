// Copyright (C) 2020-2021  CEA/DEN, EDF R&D
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

#include "MG_ADAPT_i.hxx"

#include "MG_ADAPT.hxx"
#include "SMESH_File.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_TryCatch.hxx"

using namespace SMESH;

void MG_ADAPT_i::copyHypothesisDataToImpl(const SMESH::MgAdaptHypothesisData& from, ::MG_ADAPT::MgAdaptHypothesisData* to) const
{
  to->myFileInDir = from.myFileInDir;
  to->myMeshFileIn = from.myMeshFileIn;
  to->myMeshFileBackground = from.myMeshFileBackground;
  to->myOutMeshName = from.myOutMeshName;
  to->myMeshFileOut = from.myMeshFileOut;
  to->myFileOutDir = from.myFileOutDir;
  to->myFileSizeMapDir = from.myFileSizeMapDir;
  to->myFieldName = from.myFieldName;
  to->fromMedFile = from.fromMedFile;
  to->myPublish = from.myPublish;
  to->myMeshOutMed = from.myMeshOutMed;
  to->myUseLocalMap = from.myUseLocalMap;
  to->myUseBackgroundMap = from.myUseBackgroundMap;
  to->myUseConstantValue = from.myUseConstantValue;
  to->myConstantValue = from.myConstantValue;
  to->myTimeStep = from.myTimeStep;
  to->myRank = from.myRank;
  to->myUseNoTimeStep = from.myUseNoTimeStep;
  to->myUseLastTimeStep = from.myUseLastTimeStep;
  to->myUseChosenTimeStep = from.myUseChosenTimeStep;
  to->myWorkingDir = from.myWorkingDir;
  to->myLogFile = from.myLogFile;
  to->myPrintLogInFile = from.myPrintLogInFile;
  to->myKeepFiles = from.myKeepFiles;
  to->myRemoveLogOnSuccess = from.myRemoveLogOnSuccess;
  to->myVerboseLevel = from.myVerboseLevel;
}
void MG_ADAPT_i::copyHypothesisDataFromImpl(const ::MG_ADAPT::MgAdaptHypothesisData* from, SMESH::MgAdaptHypothesisData* to) const
{
  to->myFileInDir = CORBA::string_dup(from->myFileInDir.c_str());
  to->myMeshFileIn = CORBA::string_dup(from->myMeshFileIn.c_str());
  to->myMeshFileBackground = CORBA::string_dup(from->myMeshFileBackground.c_str());
  to->myOutMeshName = CORBA::string_dup(from->myOutMeshName.c_str());
  to->myMeshFileOut = CORBA::string_dup(from->myMeshFileOut.c_str());
  to->myFileOutDir = CORBA::string_dup(from->myFileOutDir.c_str());
  to->myFileSizeMapDir = CORBA::string_dup(from->myFileSizeMapDir.c_str());
  to->myFieldName = CORBA::string_dup(from->myFieldName.c_str());
  to->fromMedFile = from->fromMedFile;
  to->myPublish = from->myPublish;
  to->myMeshOutMed = from->myMeshOutMed;
  to->myUseLocalMap = from->myUseLocalMap;
  to->myUseBackgroundMap = from->myUseBackgroundMap;
  to->myUseConstantValue = from->myUseConstantValue;
  to->myConstantValue = from->myConstantValue;
  to->myTimeStep = from->myTimeStep;
  to->myRank = from->myRank;
  to->myUseNoTimeStep = from->myUseNoTimeStep;
  to->myUseLastTimeStep = from->myUseLastTimeStep;
  to->myUseChosenTimeStep = from->myUseChosenTimeStep;
  to->myWorkingDir = CORBA::string_dup(from->myWorkingDir.c_str());
  to->myLogFile = CORBA::string_dup(from->myLogFile.c_str());
  to->myPrintLogInFile = from->myPrintLogInFile;
  to->myKeepFiles = from->myKeepFiles;
  to->myRemoveLogOnSuccess = from->myRemoveLogOnSuccess;
  to->myVerboseLevel = from->myVerboseLevel;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMG_ADAPT
 *
 *  Create measurement instance
 */
//=============================================================================

SMESH::MG_ADAPT_ptr SMESH_Gen_i::CreateMG_ADAPT()
{
#ifndef DISABLE_MG_ADAPT
  SMESH::MG_ADAPT_i* aMGadapt = new SMESH::MG_ADAPT_i();
  SMESH::MG_ADAPT_var anObj = aMGadapt->_this();
  return anObj._retn();
#else
  return SMESH::MG_ADAPT_ptr();
#endif
}
SMESH::MG_ADAPT_ptr SMESH_Gen_i::CreateAdaptationHypothesis()
{
#ifndef DISABLE_MG_ADAPT
  SMESH::MG_ADAPT_i* aMGadapt = new SMESH::MG_ADAPT_i();
  SMESH::MG_ADAPT_var anObj = aMGadapt->_this();
  return anObj._retn();
#else
  return SMESH::MG_ADAPT_ptr();
#endif
}
SMESH::MG_ADAPT_OBJECT_ptr SMESH_Gen_i::Adaptation( const char* adaptationType)
{
#ifndef DISABLE_MG_ADAPT
  if (!strcmp(adaptationType, "MG_Adapt"))
  {
    SMESH::MG_ADAPT_OBJECT_i* mg_adapt_object = new SMESH::MG_ADAPT_OBJECT_i();
    SMESH::MG_ADAPT_OBJECT_var anObj = mg_adapt_object->_this();
    return anObj._retn();
  }
#endif
  return SMESH::MG_ADAPT_OBJECT_ptr();
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
MG_ADAPT_i::MG_ADAPT_i(): SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  myMgAdapt = new ::MG_ADAPT::MgAdapt();
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
MG_ADAPT_i::~MG_ADAPT_i()
{
  delete myMgAdapt;
}
void MG_ADAPT_i::setData( SMESH::MgAdaptHypothesisData& data)
{
  ::MG_ADAPT::MgAdaptHypothesisData* baseData = new ::MG_ADAPT::MgAdaptHypothesisData();
  copyHypothesisDataToImpl(data, baseData);
  myMgAdapt->setData(baseData);
  delete baseData;
}
void MG_ADAPT_i::setMedFileIn(const char* str)
{
  myMgAdapt->setMedFileIn(str);
}
char* MG_ADAPT_i::getMedFileIn()
{
  return CORBA::string_dup(myMgAdapt->getMedFileIn().c_str());
}
void MG_ADAPT_i::setMedFileOut(const char* str)
{
  myMgAdapt->setMedFileOut(str);
}
char* MG_ADAPT_i::getMedFileOut()
{
  return CORBA::string_dup(myMgAdapt->getMedFileOut().c_str());
}
void MG_ADAPT_i::setMeshName(const char* str)
{
  myMgAdapt->setMeshName(str);
}
char* MG_ADAPT_i::getMeshName()
{
  return CORBA::string_dup(myMgAdapt->getMeshName().c_str());
}
void MG_ADAPT_i::setMeshNameOut(const char* str)
{
  myMgAdapt->setMeshNameOut(str);
}
char* MG_ADAPT_i::getMeshNameOut()
{
  return CORBA::string_dup(myMgAdapt->getMeshNameOut().c_str());
}
void MG_ADAPT_i::setMeshOutMed(bool mybool)
{
  myMgAdapt->setMeshOutMed(mybool);
}
bool MG_ADAPT_i::getMeshOutMed()
{
  return myMgAdapt->getMeshOutMed();
}
void MG_ADAPT_i::setPublish(bool mybool)
{
  myMgAdapt->setPublish(mybool);
}
bool MG_ADAPT_i::getPublish()
{
  return myMgAdapt->getPublish();
}
void MG_ADAPT_i::setSizeMapFieldName(const char* str)
{
  myMgAdapt->setFieldName(str);
}
char* MG_ADAPT_i::getSizeMapFieldName()
{
  return CORBA::string_dup(myMgAdapt->getFieldName().c_str());
}
void MG_ADAPT_i::setTimeStep(CORBA::Long t)
{
  myMgAdapt->setTimeStep(t);
}
CORBA::Long MG_ADAPT_i::getTimeStep()
{
  return myMgAdapt->getTimeStep();
}
void MG_ADAPT_i::setTimeStepRank(CORBA::Long t, CORBA::Long r)
{
  myMgAdapt->setChosenTimeStepRank();
  myMgAdapt->setRankTimeStep(t, r);
}
CORBA::Long MG_ADAPT_i::getRank()
{
  return myMgAdapt->getRank();
}
void MG_ADAPT_i::setTimeStepRankLast()
{
  myMgAdapt->setTimeStepRankLast();
}
void MG_ADAPT_i::setNoTimeStep()
{
  myMgAdapt->setNoTimeStep();
}
void MG_ADAPT_i::setLogFile(const char* str)
{
  myMgAdapt->setLogFile(str);
}
char* MG_ADAPT_i::getLogFile()
{
  return CORBA::string_dup(myMgAdapt->getLogFile().c_str());
}

void MG_ADAPT_i::setVerbosityLevel(CORBA::Long v)
{
  myMgAdapt->setVerbosityLevel(v);
}
CORBA::Long MG_ADAPT_i::getVerbosityLevel()
{
  return myMgAdapt->getVerbosityLevel();
}
void MG_ADAPT_i::setRemoveOnSuccess(bool mybool)
{
  myMgAdapt->setRemoveOnSuccess(mybool);
}
bool MG_ADAPT_i::getRemoveOnSuccess()
{
  return myMgAdapt->getRemoveOnSuccess();
}
SMESH::MgAdaptHypothesisData* MG_ADAPT_i::getData()
{
  SMESH::MgAdaptHypothesisData* result = new SMESH::MgAdaptHypothesisData();
  ::MG_ADAPT::MgAdaptHypothesisData* from =  myMgAdapt->getData();
  copyHypothesisDataFromImpl(from, result);
  return result;
}
void MG_ADAPT_i::setUseLocalMap(bool mybool)
{
  myMgAdapt->setUseLocalMap(mybool);
}
bool MG_ADAPT_i::getUseLocalMap()
{
  return myMgAdapt->getUseLocalMap();
}
void MG_ADAPT_i::setUseBackgroundMap(bool mybool)
{
  myMgAdapt->setUseBackgroundMap(mybool);
}
bool MG_ADAPT_i::getUseBackgroundMap()
{
  return myMgAdapt->getUseBackgroundMap();
}
void MG_ADAPT_i::setUseConstantValue(bool mybool)
{
  myMgAdapt->setUseConstantValue(mybool);
}
bool MG_ADAPT_i::getUseConstantValue()
{
  return myMgAdapt->getUseConstantValue();
}
void MG_ADAPT_i::setConstantSize(double value)
{
  myMgAdapt->setConstantValue(value);
}
double MG_ADAPT_i::getConstantSize()
{
  return myMgAdapt->getConstantValue();
}
void MG_ADAPT_i::setSizeMapFile(const char* str)
{
  myMgAdapt->setSizeMapFile(str);
}
char* MG_ADAPT_i::getSizeMapFile()
{
  return CORBA::string_dup(myMgAdapt->getSizeMapFile().c_str());
}
void MG_ADAPT_i::setFromMedFile(bool mybool)
{
  myMgAdapt->setFromMedFile(mybool);
}
bool MG_ADAPT_i::isFromMedFile()
{
  return myMgAdapt->isFromMedFile();
}

void MG_ADAPT_i::setKeepWorkingFiles(bool mybool)
{
  myMgAdapt->setKeepWorkingFiles(mybool);
}
bool MG_ADAPT_i::getKeepWorkingFiles()
{
  return myMgAdapt->getKeepWorkingFiles();
}

//~void MG_ADAPT_i::setPrCORBA::LongLogInFile(bool);
//~bool MG_ADAPT_i::getPrCORBA::LongLogInFile();

void MG_ADAPT_i::setSizeMapType(const char* type)
{
  setUseLocalMap(false);
  setUseBackgroundMap(false);
  setUseConstantValue(false);

  if (!strcmp("Local", type))
    setUseLocalMap(true);
  else if (!strcmp("Background", type))
    setUseBackgroundMap(true);
  else
    setUseConstantValue(true);
}
void MG_ADAPT_i::setWorkingDir(const char* dir)
{
  myMgAdapt->setWorkingDir(dir);
}
char* MG_ADAPT_i::getWorkingDir()
{
  return CORBA::string_dup(myMgAdapt->getWorkingDir().c_str());
}
bool MG_ADAPT_i::setAll()
{
  return myMgAdapt->setAll();
}
char* MG_ADAPT_i::getCommandToRun()
{
  return CORBA::string_dup(myMgAdapt->getCommandToRun().c_str());
}

// macro used to initialize excStr by exception description
// returned by SMESH_CATCH( SMESH::returnError )
#undef SMESH_CAUGHT
#define SMESH_CAUGHT excStr =

void MG_ADAPT_i::compute()
{
  SMESH::TPythonDump noDumpSoFar;

  errStr = "";
  std::string excStr;
  SMESH_TRY;

  myMgAdapt->compute(errStr);

  SMESH_CATCH( SMESH::returnError );

  SMESH_Comment errMsg;
  if ( !excStr.empty() )
  {
    errMsg << "Exception thrown on MG_ADAPT_i::compute invocation with error message \""
           << errStr << "\" with exception \"" << excStr << "\"";
  }
  else if ( !errStr.empty() )
  {
    errMsg << "MG_ADAPT_i::compute invocation returned error message \"" << errStr << "\"";
  }
  if ( !errMsg.empty() )
  {
    THROW_SALOME_CORBA_EXCEPTION( errMsg.c_str(), SALOME::INTERNAL_ERROR);
  }

  if(myMgAdapt->getPublish())
  {
    SMESH_Gen_i* smeshGen_i = SMESH_Gen_i::GetSMESHGen();
    SMESH::DriverMED_ReadStatus theStatus;
    smeshGen_i->CreateMeshesFromMED(myMgAdapt->getMedFileOut().c_str(), theStatus);
  }
}
#undef SMESH_CAUGHT
#define SMESH_CAUGHT

char* MG_ADAPT_i::getErrMsg()
{
  return CORBA::string_dup(errStr.c_str());
}
char* MG_ADAPT_i::getFileName()
{
  return CORBA::string_dup(myMgAdapt->getFileName().c_str());
}
char* MG_ADAPT_i::getExeName()
{
  return CORBA::string_dup(myMgAdapt->getExeName().c_str());
}
void MG_ADAPT_i::copyMgAdaptHypothesisData( const SMESH::MgAdaptHypothesisData& data)
{
  ::MG_ADAPT::MgAdaptHypothesisData* baseData = new ::MG_ADAPT::MgAdaptHypothesisData();
  copyHypothesisDataToImpl(data, baseData);
  myMgAdapt->copyMgAdaptHypothesisData(baseData);
  delete baseData;
}

//~void MG_ADAPT_i::checkDirPath(char*& str)
//~{
  //~myMgAdapt->checkDirPath(str);
//~}

bool MG_ADAPT_i::hasOptionDefined( const char* optionName )
{
  return myMgAdapt->hasOptionDefined(optionName);
}
void MG_ADAPT_i::setOptionValue(const char* optionName,
                                const char* optionValue)
{
  SMESH_TRY;
  myMgAdapt->setOptionValue(optionName, optionValue);
  SMESH_CATCH( SMESH::throwCorbaException );
}

char* MG_ADAPT_i::getOptionValue(const char* optionName,
                                 bool&       isDefault)
{
  SMESH_TRY;
  return CORBA::string_dup(myMgAdapt->getOptionValue(optionName, &isDefault).c_str());
  SMESH_CATCH( SMESH::throwCorbaException );
  return 0;
}
SMESH::string_array* MG_ADAPT_i::getCustomOptionValuesStrVec()
{
  SMESH::string_array_var result = new SMESH::string_array();
  std::vector <std::string> vals = myMgAdapt->getCustomOptionValuesStrVec();
  result->length((CORBA::ULong) vals.size()) ;
  for (CORBA::ULong i = 0; i<vals.size(); i++) result[i] = CORBA::string_dup(vals[i].c_str());
  return result._retn();
}
SMESH::string_array*  MG_ADAPT_i::getOptionValuesStrVec()
{

  SMESH::string_array_var result = new SMESH::string_array();
  std::vector <std::string> vals = myMgAdapt->getOptionValuesStrVec();
  result->length((CORBA::ULong) vals.size());
  for (CORBA::ULong i = 0; i<vals.size(); i++) result[i] = CORBA::string_dup(vals[i].c_str());
  return result._retn();
}

void MG_ADAPT_i::setPrintLogInFile(bool mybool)
{
  myMgAdapt->setPrintLogInFile(mybool);
}
bool MG_ADAPT_i::getPrintLogInFile()
{
  return myMgAdapt->getPrintLogInFile();
}
//~TOptionValues        MG_ADAPT_i::getOptionValues()       const;
//~const TOptionValues& MG_ADAPT_i::getCustomOptionValues() const ;

MG_ADAPT_OBJECT_i::MG_ADAPT_OBJECT_i(): SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  medFileIn="";
  medFileOut="";
  medFileBackground="";
  publish = false;
  //~myMesh = CORBA::nil;
}

void MG_ADAPT_OBJECT_i::setMeshIn(SMESH::SMESH_Mesh_ptr theMesh )
{
  myMesh = SMESH::SMESH_Mesh::_duplicate(theMesh);
}
void MG_ADAPT_OBJECT_i::setMEDFileIn(const char* f)
{
  medFileIn =  f;
}
void MG_ADAPT_OBJECT_i::setMEDFileOut(const char* f)
{
  medFileOut = f;
}
void MG_ADAPT_OBJECT_i::setMEDFileBackground(const char* f)
{
  medFileBackground = f;
}
void MG_ADAPT_OBJECT_i::AddHypothesis(SMESH::MG_ADAPT_ptr mg)
{

  mg->setMedFileIn(medFileIn.c_str());
  mg->setMedFileOut(medFileOut.c_str());
  mg->setSizeMapFile(medFileBackground.c_str());
  hypothesis = SMESH::MG_ADAPT::_duplicate(mg);
  hypothesis->Register();
}
CORBA::Long MG_ADAPT_OBJECT_i::Compute(bool publish)
{
  SMESH::TPythonDump noDumpSoFar;

  if(!checkMeshFileIn()){
    std::cerr<< "\n Error : Please check the MED file input or mesh input. \n";
    return -1;
  }
  hypothesis->setPublish(publish);
  hypothesis->compute();
  return 0;
}

bool MG_ADAPT_OBJECT_i::checkMeshFileIn()
{
  SMESH::TPythonDump noDumpSoFar;

  bool ret = false; // 1 ok , 0 nook
  if ( !( ret = SMESH_File( medFileIn ).exists()))
  {
    if(!myMesh->_is_nil())
    {
      bool toOverwrite  = true;
      bool toFindOutDim = true;
      medFileIn = (CORBA::String_var( hypothesis->getFileName() )).in();
      medFileIn+= ".med";
      myMesh->ExportMED(medFileIn.c_str(), false, -1, toOverwrite, toFindOutDim);
      hypothesis->setMedFileIn(medFileIn.c_str());
      ret = true;
    }
  }

  return ret;
}
