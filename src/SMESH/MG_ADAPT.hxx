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

#ifndef MG_ADAPT_HXX
#define MG_ADAPT_HXX

#include <string>
#include <map>
#include <vector>
#include <set>

#include "SMESH_SMESH.hxx"

namespace MEDCoupling
{
  class MEDFileMesh;
}

namespace MG_ADAPT
{
typedef std::map< std::string, std::string > TOptionValues;
typedef std::set< std::string > TOptionNames;

std::string remove_extension(const std::string& filename);

struct MgAdaptHypothesisData
{
  std::string myFileInDir, myMeshFileIn, myInMeshName, myMeshFileBackground, myOutMeshName,
              myMeshFileOut, myFileOutDir, myFileSizeMapDir, myFieldName;
  bool fromMedFile;
  bool myPublish, myMeshOutMed;
  bool myUseLocalMap, myUseBackgroundMap, myUseConstantValue;
  double myConstantValue;
  int  myRank, myTimeStep;
  bool myUseNoTimeStep, myUseLastTimeStep, myUseChosenTimeStep;
  std::string myWorkingDir, myLogFile;
  bool myPrintLogInFile, myKeepFiles, myRemoveLogOnSuccess;
  int myVerboseLevel;
};


class SMESH_EXPORT MgAdapt
{

public:

  MgAdapt();
  MgAdapt(MgAdaptHypothesisData*);
  MgAdapt(const MgAdapt&);
  ~MgAdapt();
  void buildModel();
  void setData( MgAdaptHypothesisData* data);

  void setMedFileIn(std::string fileName);
  std::string getMedFileIn();

  void setMedFileOut(std::string fileOut);
  std::string getMedFileOut();

  void setMeshName(std::string name);
  std::string getMeshName();

  void setMeshNameOut(std::string  name);
  std::string getMeshNameOut();

  void setMeshOutMed(bool mybool);
  bool getMeshOutMed();

  void setPublish(bool mybool);
  bool getPublish();

  void setFieldName(std::string myFieldName);
  std::string getFieldName();

  void setTimeStep(int time);
  int getTimeStep() const;

  void setRankTimeStep(int time, int myRank);
  int getRank();

  void setTimeStepRankLast();
  void setNoTimeStep();
  void setChosenTimeStepRank();
  void updateTimeStepRank();

  void setLogFile(std::string);
  std::string getLogFile();

  void setVerbosityLevel(int verbosity);
  int getVerbosityLevel();

  void setRemoveOnSuccess(bool mybool);
  bool getRemoveOnSuccess();

  MgAdaptHypothesisData* getData() const;

  void setUseLocalMap(bool mybool);
  bool getUseLocalMap();

  void setUseBackgroundMap(bool mybool);
  bool getUseBackgroundMap();

  void setUseConstantValue(bool mybool);
  bool getUseConstantValue();

  void setConstantValue(double cnst);
  double getConstantValue() const;

  void setSizeMapFile(std::string mapFile);
  std::string getSizeMapFile();

  void setFromMedFile(bool mybool);
  bool isFromMedFile();

  void setKeepWorkingFiles(bool mybool);
  bool getKeepWorkingFiles();

  void setPrintLogInFile(bool mybool);
  bool getPrintLogInFile();

  void setWorkingDir(std::string dir);
  std::string getWorkingDir() const;


  bool setAll();
  static std::string getCommandToRun(MgAdapt* );
  std::string getCommandToRun() ;
  int compute(std::string& errStr);
  std::string getFileName() const;
  static std::string getExeName();
  void copyMgAdaptHypothesisData( const MgAdaptHypothesisData* from) ;

  void checkDirPath(std::string& dirPath);

  bool hasOptionDefined( const std::string& optionName ) const;
  void setOptionValue(const std::string& optionName,
                      const std::string& optionValue);
  std::string getOptionValue(const std::string& optionName,
                              bool*              isDefault=0) const;
  std::vector <std::string> getCustomOptionValuesStrVec() const;
  std::vector <std::string> getOptionValuesStrVec() const;


  TOptionValues        getOptionValues()       const;
  const TOptionValues& getCustomOptionValues() const ;
  static double toDbl(const std::string&, bool* isOk = 0);
  static bool toBool(const std::string&, bool* isOk = 0);
  static int toInt(const std::string&, bool* isOk = 0 );
  static std::string toLowerStr(const std::string& str);

  /*  default values */
  static std::string defaultWorkingDirectory();
  static std::string defaultLogFile();
  static bool  defaultKeepFiles();
  static bool  defaultRemoveLogOnSuccess();
  static int   defaultVerboseLevel();
  static bool  defaultPrintLogInFile();
  static bool  defaultFromMedFile();
  static bool  defaultMeshOutMed();
  static bool  defaultPublish();
  static bool  defaultUseLocalMap();
  static bool  defaultUseBackgroundMap();
  static bool  defaultUseConstantValue();
  static bool  defaultUseNoTimeStep();
  static bool  defaultUseLastTimeStep();
  static bool  defaultUseChosenTimeStep();
  static double  defaultMaximumMemory();

  enum Status {
    DRS_OK,
    DRS_EMPTY,          // a file contains no mesh with the given name
    DRS_WARN_RENUMBER,  // a file has overlapped ranges of element numbers,
    // so the numbers from the file are ignored
    DRS_WARN_SKIP_ELEM, // some elements were skipped due to incorrect file data
    DRS_WARN_DESCENDING, // some elements were skipped due to descending connectivity
    DRS_FAIL,            // general failure (exception etc.)
    DRS_NO_TIME_STEP            // general failure (exception etc.)
  };

  struct group
  {
    std::string              _name;
    std::vector<int>         _famListId;
    std::vector<std::string> _famNames;
    group(std::string name, std::vector<int> famListId, std::vector<std::string> famNames)
      :_name(name), _famListId( famListId ), _famNames( famNames ) {}
  };

  struct family
  {
    std::string _famName;
    int         _famId;
    family(std::string famName, int famId):_famName(famName), _famId(famId) {}
  };


private :
  bool fromMedFile;
  std::string medFileIn;
  std::string medFileOut;
  std::string meshName;
  std::string meshNameOut;
  bool publish, meshOutMed;
  bool useLocalMap, useBackgroundMap, useConstantValue;
  bool myUseLastTimeStep, myUseNoTimeStep, myUseChosenTimeStep;
  std::string sizeMapFile;
  std::string fieldName;
  double constantValue;
  int rank,  timeStep;

  /* advanced options */


  std::string logFile;
  std::string workingDir;
  int verbosityLevel;
  bool removeOnSuccess;
  bool toKeepWorkingFiles;
  bool printLogInFile;

  /* Model DATA */
  MgAdaptHypothesisData* data;

  /*            */

  TOptionValues _option2value, _customOption2value;         // user defined values
  TOptionValues _defaultOptionValues;                       // default values
  TOptionNames  _doubleOptions, _charOptions, _boolOptions; // to find a type of option

  std::vector <std::string> _myErrorMessages;
  Status _myStatus;
  std::string meshFormatOutputMesh;
  std::vector< std::string> solFormatOutput;
  std::vector <group> groupVec;
  std::vector <family> famVec;
  std::vector< std::string> tmpFilesToBeDeleted;

  /* convert MED-->.mesh format */
  void convertMedFile(std::string& meshIn,std::string& solFileIn,  std::string& sizeMapIn)  ;
  void storeGroups(MEDCoupling::MEDFileMesh* fileMesh);
  void restoreGroups(MEDCoupling::MEDFileMesh* fileMesh) const;
  void storefams(MEDCoupling::MEDFileMesh* fileMesh);
  void restorefams(MEDCoupling::MEDFileMesh* fileMesh) const;
  void storeGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh);
  void restoreGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh) const;
  void convertMeshFile(std::string& meshFormatIn, std::vector< std::string>& solFieldFileNames) const ;
  void buildConstantSizeMapSolFile(const std::string& solFormatFieldFileName, const int dim, const int version, const size_t nbNodes) const;
  void buildBackGroundMeshAndSolFiles(const std::vector<std::string>& fieldFileNames, const std::string& meshFormatsizeMapFile) const;
  Status addMessage(const std::string& msg, const bool isFatal = false);
  void execCmd( const char* cmd, int& err);
  void cleanUp();
  void appendMsgToLogFile(std::string& msg);
  std::vector<std::string> getListFieldsNames(std::string fileIn) ;
  void checkDimensionOptionAdaptation() ;
  void checkFieldName(std::string fileIn) ;
  void checkTimeStepRank(std::string fileIn) ;

};

} // namespace MG_ADAPT

#endif // MG_ADAPT_HXX
