#ifndef MG_ADAPT_I_HXX
#define MG_ADAPT_I_HXX

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(ADAPT_Gen)
#include CORBA_SERVER_HEADER(MG_ADAPT)

#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <string>

struct MgAdaptHypothesisData;
class MgAdapt;

class MG_ADAPT_i :
    public virtual Engines_Component_i,
    public virtual POA_ADAPT::MG_ADAPT,
    public virtual PortableServer::ServantBase
{
public:
    MG_ADAPT_i( CORBA::ORB_ptr orb, ADAPT::ADAPT_Gen_var gen_i );
    MG_ADAPT_i();
    virtual ~MG_ADAPT_i();
	void setData( MgAdaptHypothesisData*);

	void setMedFileIn(char* str);
	char* getMedFileIn();

	void setMedFileOut(char* str);
	char* getMedFileOut();

	void setMeshName(char* str);
	char* getMeshName();

	void setMeshNameOut(char* str);
	char* getMeshNameOut();

	void setMeshOutMed(bool mybool);
	bool getMeshOutMed();

	void setPublish(bool mybool);
	bool getPublish();

	void setFieldName(char* str);
	char* getFieldName();

	void setTimeStep(CORBA::Long t);
	CORBA::Long getTimeStep() const;

	void setRankTimeStep(CORBA::Long t, CORBA::Long r );
	CORBA::Long getRank();

	void setLogFile(char* str);
	char* getLogFile();

	void setVerbosityLevel(CORBA::Long v);
	CORBA::Long getVerbosityLevel();

	void setRemoveOnSuccess(bool mybool);
	bool getRemoveOnSuccess();

	MgAdaptHypothesisData* getData() const;

	void setUseLocalMap(bool mybool);
	bool getUseLocalMap();

	void setUseBackgroundMap(bool mybool);
	bool getUseBackgroundMap();

	void setUseConstantValue(bool mybool);
	bool getUseConstantValue();

	void setConstantValue(double value);
	double getConstantValue() const;

	void setSizeMapFile(char* str);
	char* getSizeMapFile();

	void setFromMedFile(bool mybool);
	bool isFromMedFile();

	void setKeepWorkingFiles(bool mybool);
	bool getKeepWorkingFiles();

	//~void setPrCORBA::LongLogInFile(bool);
	//~bool getPrCORBA::LongLogInFile();

	void setWorkingDir(char* str);
	char* getWorkingDir() const;


	bool setAll();
	char* getCommandToRun() ;
	CORBA::Long compute(char* errStr);
	char* getFileName() const;
	char* getExeName();
	void copyMgAdaptHypothesisData( MgAdaptHypothesisData* data ) ;

	void checkDirPath(std::string& str);



	bool hasOptionDefined( const char* optionName ) const;
	void setOptionValue(const char* optionName,
						const char* optionValue) throw (std::invalid_argument);
	std::string getOptionValue(const char* optionName,
							   bool*              isDefault=0) const throw (std::invalid_argument);
	std::vector <std::string> getCustomOptionValuesStrVec() const;
	std::vector <std::string> getOptionValuesStrVec() const;


	//~TOptionValues        getOptionValues()       const;
   	//~const TOptionValues& getCustomOptionValues() const ;
private:
  MgAdapt*          myMgAdapt;

  CORBA::ORB_ptr         _orb;
  ADAPT::ADAPT_Gen_var _gen_i;
	
};
#endif // MG_ADAPT_I_HXX
