#ifndef MG_ADAPT_I_HXX
#define MG_ADAPT_I_HXX

#include "MG_ADAPT.hxx"
#include "SMESH.hxx"
#include <SALOMEconfig.h>
//~#include CORBA_SERVER_HEADER(ADAPT_Gen)
#include CORBA_SERVER_HEADER(MG_ADAPT)

#include "SALOME_GenericObj_i.hh"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <string>

//~struct MgAdaptHypothesisData;
//~static void copyHypothesisDataToImpl(SMESH::MgAdaptHypothesisData& from, MG_ADAPT::MgAdaptHypothesisData* to);
//~static void copyHypothesisDataFromImpl(MG_ADAPT::MgAdaptHypothesisData* from, SMESH::MgAdaptHypothesisData& to);
//~class MgAdapt;
namespace SMESH
{
class SMESH_I_EXPORT MG_ADAPT_i :
    public virtual SALOME::GenericObj_i,
    public virtual POA_SMESH::MG_ADAPT
{
public:
    //~MG_ADAPT_i( CORBA::ORB_ptr orb, ADAPT::ADAPT_Gen_var gen_i );
    //~static SMESH::MG_ADAPT_ptr CreateMG_ADAPT();
    //~MG_ADAPT_i(PortableServer::POA_var poa);
    MG_ADAPT_i();
    virtual ~MG_ADAPT_i();
	//~void setData( SMESH::MgAdaptHypothesisData* data);
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

	void setFieldName(const char* str);
	char* getFieldName();

	void setTimeStep(CORBA::Long t);
	CORBA::Long getTimeStep() ;

	void setRankTimeStep(CORBA::Long t, CORBA::Long r );
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

	void setUseLocalMap(bool mybool);
	bool getUseLocalMap();

	void setUseBackgroundMap(bool mybool);
	bool getUseBackgroundMap();

	void setUseConstantValue(bool mybool);
	bool getUseConstantValue();

	void setConstantValue(double value);
	double getConstantValue();

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
	CORBA::Long compute(::CORBA::String_out errStr);
	char* getFileName();
	char* getExeName();
	void copyMgAdaptHypothesisData( const SMESH::MgAdaptHypothesisData& data ) ;
	//~void copyMgAdaptHypothesisData( const SMESH::MgAdaptHypothesisData& data ) {
		//~copyMgAdaptHypothesisData(&data);
	//~}

	//~void checkDirPath(char*& str);
	
	bool hasOptionDefined( const char* optionName ) ;
	void setOptionValue(const char* optionName,
						const char* optionValue) throw (std::invalid_argument);
	char* getOptionValue(const char* optionName,
							   bool&              isDefault)  throw (std::invalid_argument);
	str_array* getCustomOptionValuesStrVec() ;
	str_array* getOptionValuesStrVec() ;
	void copyHypothesisDataFromImpl(const ::MG_ADAPT::MgAdaptHypothesisData* from, SMESH::MgAdaptHypothesisData* to) const;
	void copyHypothesisDataToImpl(const SMESH::MgAdaptHypothesisData& from, ::MG_ADAPT::MgAdaptHypothesisData* to) const;
	//~TOptionValues        getOptionValues()       const;
   	//~const TOptionValues& getCustomOptionValues() const ;
private:
  ::MG_ADAPT::MgAdapt*          myMgAdapt;

  //~CORBA::ORB_ptr         _orb;
  //~ADAPT::ADAPT_Gen_var _gen_i;
	
};
}
#endif // MG_ADAPT_I_HXX
