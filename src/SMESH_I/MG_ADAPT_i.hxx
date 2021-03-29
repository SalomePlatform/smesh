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

#ifndef MG_ADAPT_I_HXX
#define MG_ADAPT_I_HXX

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(MG_ADAPT)
#include "SALOME_GenericObj_i.hh"
#include <SALOME_GenericObj_wrap.hxx>

#include <string>

namespace MG_ADAPT
{
  class MgAdaptHypothesisData;
  class MgAdapt;
}

namespace SMESH
{

class SMESH_I_EXPORT MG_ADAPT_i :
    public virtual SALOME::GenericObj_i,
    public virtual POA_SMESH::MG_ADAPT
{
public:
  MG_ADAPT_i();
  virtual ~MG_ADAPT_i();
  void setData( SMESH::MgAdaptHypothesisData& data);

  void setMedFileIn(const char* str);
  char* getMedFileIn();

  void setMedFileOut(const char* str);
  char* getMedFileOut();

  void setMeshName(const char* str);
  char* getMeshName();

  void setMeshNameOut(const char* str);
  char* getMeshNameOut();

  void setMeshOutMed(bool mybool);
  bool getMeshOutMed();

  void setPublish(bool mybool);
  bool getPublish();

  void setSizeMapFieldName(const char* str);
  char* getSizeMapFieldName();

  void setTimeStep(CORBA::Long t);
  CORBA::Long getTimeStep() ;

  void setTimeStepRank(CORBA::Long t, CORBA::Long r );
  CORBA::Long getRank();

  void setTimeStepRankLast();
  void setNoTimeStep();

  void setLogFile(const char* str);
  char* getLogFile();

  void setVerbosityLevel(CORBA::Long v);
  CORBA::Long getVerbosityLevel();

  void setRemoveOnSuccess(bool mybool);
  bool getRemoveOnSuccess();

  SMESH::MgAdaptHypothesisData* getData() ;
  void setSizeMapType(const char* type);
  void setUseLocalMap(bool mybool);
  bool getUseLocalMap();

  void setUseBackgroundMap(bool mybool);
  bool getUseBackgroundMap();

  void setUseConstantValue(bool mybool);
  bool getUseConstantValue();

  void setConstantSize(double value);
  double getConstantSize();

  void setSizeMapFile(const char* str);
  char* getSizeMapFile();

  void setFromMedFile(bool mybool);
  bool isFromMedFile();

  void setKeepWorkingFiles(bool mybool);
  bool getKeepWorkingFiles();

  //~void setPrCORBA::LongLogInFile(bool);
  //~bool getPrCORBA::LongLogInFile();

  void setWorkingDir(const char* str);
  char* getWorkingDir() ;

  void setPrintLogInFile(bool mybool);
  bool getPrintLogInFile();

  bool setAll();
  char* getCommandToRun() ;
  void compute();
  char* getFileName();
  char* getExeName();
  void copyMgAdaptHypothesisData( const SMESH::MgAdaptHypothesisData& data ) ;
  //~void copyMgAdaptHypothesisData( const SMESH::MgAdaptHypothesisData& data ) {
          //~copyMgAdaptHypothesisData(&data);
  //~}

  //~void checkDirPath(char*& str);

  bool hasOptionDefined( const char* optionName ) ;
  void setOptionValue(const char* optionName,
                      const char* optionValue);
  char* getOptionValue(const char* optionName,
                       bool&       isDefault);
  SMESH::string_array* getCustomOptionValuesStrVec() ;
  SMESH::string_array* getOptionValuesStrVec() ;
  void copyHypothesisDataFromImpl(const ::MG_ADAPT::MgAdaptHypothesisData* from, SMESH::MgAdaptHypothesisData* to) const;
  void copyHypothesisDataToImpl(const SMESH::MgAdaptHypothesisData& from, ::MG_ADAPT::MgAdaptHypothesisData* to) const;
  //~TOptionValues        getOptionValues()       const;
  //~const TOptionValues& getCustomOptionValues() const ;
  char* getErrMsg();
private:
  ::MG_ADAPT::MgAdapt* myMgAdapt;
  std::string          errStr;

};

class SMESH_I_EXPORT MG_ADAPT_OBJECT_i:
  public virtual SALOME::GenericObj_i,
  public virtual POA_SMESH::MG_ADAPT_OBJECT {
public :
  MG_ADAPT_OBJECT_i();
  void setMeshIn( SMESH::SMESH_Mesh_ptr theMesh );
  void setMEDFileIn(const char* f);
  void setMEDFileOut(const char* f);
  void setMEDFileBackground(const char* f);
  void AddHypothesis(SMESH::MG_ADAPT_ptr);
      CORBA::Long Compute(bool Publish);
private:
std::string medFileIn, medFileOut, medFileBackground;
bool checkMeshFileIn();
bool publish;
SMESH::SMESH_Mesh_var myMesh;
SALOME::GenericObj_wrap<SMESH::MG_ADAPT> hypothesis;
};

}

#endif // MG_ADAPT_I_HXX
