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

//
// File   : MG_ADAPT.hxx
//
#ifndef MG_ADAPT_HXX
#define MG_ADAPT_HXX
#include <string>
# include <sstream>
#include <map>
#include <vector>
#include <set>
#include <fstream>

#include "MCAuto.hxx"
#include "MCType.hxx"
#include "MEDFileMesh.hxx"

#include <med.h>
// SMESH includes

//~#include <med.h>


class MgAdapt;

typedef std::map< std::string, std::string > TOptionValues;
typedef std::set< std::string >              TOptionNames;

struct MgAdaptHypothesisData
{
    std::string myFileInDir, myMeshFileIn, myInMeshName, myMeshFileBackground, myOutMeshName,
        myMeshFileOut,  myFileOutDir, myFileSizeMapDir, myFieldName;
    bool    fromMedFile;
    bool    myPublish, myMeshOutMed;
    bool    myUseLocalMap, myUseBackgroundMap, myUseConstantValue;
    double  myConstantValue;
    int     myRank, myTimeStep;
    bool    myUseNoTimeStep, myUseLastTimeStep, myUseChosenTimeStep;
    std::string myWorkingDir, myLogFile;
    bool    myPrintLogInFile, myKeepFiles, myRemoveLogOnSuccess;
    int   myVerboseLevel;

};



/*!
 * \brief Class to generate string from any type
 */
class ToComment : public std::string
{
    std::ostringstream _s ;

public :

    ToComment():std::string("") {}

    ToComment(const ToComment& c):std::string() {
        _s << c.c_str() ;
        this->std::string::operator=( _s.str() );
    }

    ToComment & operator=(const ToComment& c) {
        _s << c.c_str() ;
        this->std::string::operator=( _s.str() );
        return *this;
    }

    template <class T>
    ToComment( const T &anything ) {
        _s << anything ;
        this->std::string::operator=( _s.str() );
    }

    template <class T>
    ToComment & operator<<( const T &anything ) {
        _s << anything ;
        this->std::string::operator=( _s.str() );
        return *this ;
    }

    operator char*() const {
        return (char*)c_str();
    }

    std::ostream& Stream() {
        return _s;
    }
};


class MgAdapt
{
    enum YesNo {YES, NO};
public:

    MgAdapt();
    MgAdapt(MgAdaptHypothesisData*);
    MgAdapt(const MgAdapt&);
    ~MgAdapt();
    void buildModel();
    void setData( MgAdaptHypothesisData*);

    void setMedFileIn(std::string);
    std::string getMedFileIn();

    void setMedFileOut(std::string);
    std::string getMedFileOut();

    void setMeshName(std::string);
    std::string getMeshName();

    void setMeshNameOut(std::string);
    std::string getMeshNameOut();

    void setMeshOutMed(bool);
    bool getMeshOutMed();

    void setPublish(bool);
    bool getPublish();

    void setFieldName(std::string);
    std::string getFieldName();

    void setTimeStep(int);
    int getTimeStep() const;

    void setRankTimeStep(int, int );
    int getRank();

    void setLogFile(std::string);
    std::string getLogFile();

    void setVerbosityLevel(int);
    int getVerbosityLevel();

    void setRemoveOnSuccess(bool);
    bool getRemoveOnSuccess();

    MgAdaptHypothesisData* getData() const;

    void setUseLocalMap(bool);
    bool getUseLocalMap();

    void setUseBackgroundMap(bool);
    bool getUseBackgroundMap();

    void setUseConstantValue(bool);
    bool getUseConstantValue();

    void setConstantValue(double);
    bool getConstantValue();

    void setSizeMapFile(std::string);
    std::string getSizeMapFile();

    void setFromMedFile(bool);
    bool isFromMedFile();

    void setKeepWorkingFiles(bool);
    bool getKeepWorkingFiles();

    void setPrintLogInFile(bool);
    bool getPrintLogInFile();

    void setWorkingDir(std::string);
    std::string getWorkingDir() const;


    bool setAll();
    static std::string getCommandToRun(MgAdapt* );
    std::string getCommandToRun() ;
    int compute(std::string& errStr);
    std::string getFileName() const;
    static std::string getExeName();
    void copyMgAdaptHypothesisData( MgAdaptHypothesisData* ) ;

    void checkDirPath(std::string& );



    bool hasOptionDefined( const std::string& optionName ) const;
    void setOptionValue(const std::string& optionName,
                        const std::string& optionValue) throw (std::invalid_argument);
    std::string getOptionValue(const std::string& optionName,
                               bool*              isDefault=0) const throw (std::invalid_argument);
    std::vector <std::string> getCustomOptionValuesStrVec() const;
    std::vector <std::string> getOptionValuesStrVec() const;


    TOptionValues        getOptionValues()       const;
    const TOptionValues& getCustomOptionValues() const ;
    static double toDbl(const std::string&, bool* isOk = 0) throw (std::invalid_argument);
    static bool toBool(const std::string&, bool* isOk = 0) throw (std::invalid_argument);
    static int toInt(const std::string&, bool* isOk = 0 ) throw (std::invalid_argument);


    /*  default values */
    static std::string defaultWorkingDirectory();
    static std::string defaultLogFile();
    static bool   defaultKeepFiles();
    static bool   defaultRemoveLogOnSuccess();
    static int  defaultVerboseLevel();
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

    struct group {

        std::string _name;
        std::vector<MEDCoupling::mcIdType> _famListId;
        std::vector<std::string> _famNames;
        group(std::string name, std::vector<MEDCoupling::mcIdType> famListId, std::vector<std::string> famNames):_name(name)
        {
            std::vector<MEDCoupling::mcIdType>::iterator it = famListId.begin();
            for (; it!=famListId.end(); ++it)
                _famListId.push_back(*it);

            std::vector<std::string>::iterator itt = famNames.begin();
            for (; itt!=famNames.end(); ++itt)
                _famNames.push_back(*itt);
        }
    };

    struct family {
        std::string _famName;
        mcIdType _famId;
        family(std::string famName, MEDCoupling::mcIdType famId):_famName(famName), _famId(famId) {}
    };


private :
    bool fromMedFile;

    std::string medFileIn;
    std::string medFileOut;
    std::string meshName;
    std::string meshNameOut;
    bool publish, meshOutMed;
    bool useLocalMap, useBackgroundMap, useConstantValue;
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

    /* convert MED-->.mesh format */
    void convertMedFile(std::string& meshIn,std::string& solFileIn,  std::string& sizeMapIn)  ;
    void storeGroups(MEDCoupling::MEDFileMesh* fileMesh);
    void restoreGroups(MEDCoupling::MEDFileMesh* fileMesh) const;
    void storefams(MEDCoupling::MEDFileMesh* fileMesh);
    void restorefams(MEDCoupling::MEDFileMesh* fileMesh) const;
    void storeGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh);
    void restoreGroupsAndFams(MEDCoupling::MEDFileMesh* fileMesh) const;
    void convertMeshFile(std::string& meshFormatIn, std::vector< std::string>& solFieldFileNames) const ;
    void buildConstantSizeMapSolFile(const std::string& solFormatFieldFileName, const int dim, const int version, const mcIdType nbNodes) const;
    void buildBackGroundMeshAndSolFiles(const std::vector<std::string>& fieldFileNames, const std::string& meshFormatsizeMapFile) const;
    void getTimeStepInfos(std::string aFile, med_int& numdt, med_int& numit);
    Status addMessage(const std::string& msg, const bool isFatal = false);
    med_idt openMedFile(const std::string aFile) ;
    bool isFileExist(std::string& fName) const;
    void execCmd( const char* cmd, int& err);
};



#endif // MG_ADAPT_HXX
