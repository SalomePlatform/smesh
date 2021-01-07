// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier
//

#include "MG_ADAPT_i.hxx"
#include "ADAPT_Gen_i.hxx"
#include "MG_ADAPT.hxx"
#include "string.h"

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
MG_ADAPT_i::MG_ADAPT_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
MG_ADAPT_i::MG_ADAPT_i( CORBA::ORB_ptr orb,
                            ADAPT::ADAPT_Gen_var engine )
{

  _gen_i = engine;
  _orb = orb;
  myMgAdapt = new MgAdapt();
  //~ASSERT( myHomardCas );
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
MG_ADAPT_i::~MG_ADAPT_i()
{
}
void MG_ADAPT_i::setData( MgAdaptHypothesisData* data)
{
	myMgAdapt->setData(data);
}	
void MG_ADAPT_i::setMedFileIn(char* str)
{
	myMgAdapt->setMedFileIn(str);
}
char* MG_ADAPT_i::getMedFileIn()
{
	return CORBA::string_dup(myMgAdapt->getMedFileIn().c_str());
}
void MG_ADAPT_i::setMedFileOut(char* str)
{
	myMgAdapt->setMedFileOut(str);
}
char* MG_ADAPT_i::getMedFileOut()
{
	return CORBA::string_dup(myMgAdapt->getMedFileOut().c_str());
}
void MG_ADAPT_i::setMeshName(char* str)
{
	myMgAdapt->setMeshName(str);
}
char* MG_ADAPT_i::getMeshName()
{
	return CORBA::string_dup(myMgAdapt->getMeshName().c_str());
}
void MG_ADAPT_i::setMeshNameOut(char* str)
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
void MG_ADAPT_i::setFieldName(char* str)
{
	myMgAdapt->setFieldName(str);
}
char* MG_ADAPT_i::getFieldName()
{
	return CORBA::string_dup(myMgAdapt->getFieldName().c_str());
}
void MG_ADAPT_i::setTimeStep(CORBA::Long t)
{
	myMgAdapt->setTimeStep(t);
}
CORBA::Long MG_ADAPT_i::getTimeStep() const
{
	return myMgAdapt->getTimeStep();
}
void MG_ADAPT_i::setRankTimeStep(CORBA::Long t, CORBA::Long r)
{
	myMgAdapt->setRankTimeStep(t, r);
}
CORBA::Long MG_ADAPT_i::getRank()
{
	return myMgAdapt->getRank();
}
void MG_ADAPT_i::setLogFile(char* str)
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
	myMgAdapt->getRemoveOnSuccess();
}
MgAdaptHypothesisData* MG_ADAPT_i::getData() const
{
	return myMgAdapt->getData();
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

void MG_ADAPT_i::setConstantValue(double value)
{
	myMgAdapt->setConstantValue(value);
}
double MG_ADAPT_i::getConstantValue() const
{
	return myMgAdapt->getConstantValue();
}
void MG_ADAPT_i::setSizeMapFile(char* str)
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

void MG_ADAPT_i::setWorkingDir(char* dir)
{
	myMgAdapt->setWorkingDir(dir);
}
char* MG_ADAPT_i::getWorkingDir() const
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
CORBA::Long MG_ADAPT_i::compute(char* errStr)
{
	std::string err("");
	CORBA::Long ret = myMgAdapt->compute(err);
	strcpy(errStr, err.c_str());
	return ret;
}
char* MG_ADAPT_i::getFileName() const
{
	return CORBA::string_dup(myMgAdapt->getFileName().c_str());
}
char* MG_ADAPT_i::getExeName()
{
	return CORBA::string_dup(myMgAdapt->getExeName().c_str());
}
void MG_ADAPT_i::copyMgAdaptHypothesisData( MgAdaptHypothesisData* data)
{
	myMgAdapt->copyMgAdaptHypothesisData(data);
}

void MG_ADAPT_i::checkDirPath(std::string& str)
{
	myMgAdapt->checkDirPath(str);
}

bool MG_ADAPT_i::hasOptionDefined( const char* optionName ) const
{
	return myMgAdapt->hasOptionDefined(optionName);
}
void MG_ADAPT_i::setOptionValue(const char* optionName,
					const char* optionValue) throw (std::invalid_argument)
{
	myMgAdapt->setOptionValue(optionName, optionValue);
}					
std::string MG_ADAPT_i::getOptionValue(const char* optionName,
						   bool*              isDefault) const throw (std::invalid_argument)
{
    return myMgAdapt->getOptionValue(optionName, isDefault);
}
std::vector <std::string> MG_ADAPT_i::getCustomOptionValuesStrVec() const
{
   	return myMgAdapt->getCustomOptionValuesStrVec();
}
std::vector <std::string> MG_ADAPT_i::getOptionValuesStrVec() const
{
	return myMgAdapt->getOptionValuesStrVec();
}


//~TOptionValues        MG_ADAPT_i::getOptionValues()       const;
//~const TOptionValues& MG_ADAPT_i::getCustomOptionValues() const ;
