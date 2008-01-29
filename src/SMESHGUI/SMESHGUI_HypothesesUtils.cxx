//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com

#include "SMESHGUI_HypothesesUtils.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_XmlHandler.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SUIT_Tools.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Session.h"

#include "OB_Browser.h"

#include "SalomeApp_Study.h"
#include "SalomeApp_Tools.h"
#include "SalomeApp_Application.h"

#include <SALOMEDSClient_Study.hxx>
#include <SALOMEDSClient_SObject.hxx>

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

#include <map>
#include <string>

#ifdef WNT
 #include <windows.h>
#else
 #include <dlfcn.h>
#endif

#ifdef WNT
 #define LibHandle HMODULE
 #define LoadLib( name ) LoadLibrary( name )
 #define GetProc GetProcAddress
 #define UnLoadLib( handle ) FreeLibrary( handle );
#else
 #define LibHandle void*
 #define LoadLib( name ) dlopen( name, RTLD_LAZY )
 #define GetProc dlsym
 #define UnLoadLib( handle ) dlclose( handle );
#endif

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

namespace SMESH{

  using namespace std;

  typedef map<string,HypothesisData*> THypothesisDataMap;
  THypothesisDataMap myHypothesesMap;
  THypothesisDataMap myAlgorithmsMap;

  typedef map<string,SMESHGUI_GenericHypothesisCreator*> THypCreatorMap;
  THypCreatorMap myHypCreatorMap;

  list<HypothesesSet*> myListOfHypothesesSets;

  void processHypothesisStatus(const int theHypStatus,
			       SMESH::SMESH_Hypothesis_ptr theHyp,
			       const bool theIsAddition)
  {
    if (theHypStatus > SMESH::HYP_OK) {

      // get Hyp name
      QString aHypName ("NULL Hypothesis");
      if (!CORBA::is_nil(theHyp)) {
	_PTR(SObject) Shyp = SMESH::FindSObject(theHyp);
	if (Shyp)
	  // name in study
	  aHypName = Shyp->GetName().c_str();
	else
	  // label in xml file
	  aHypName = GetHypothesisData(theHyp->GetName())->Label;
      }

      // message
      bool isFatal = (theHypStatus >= SMESH::HYP_UNKNOWN_FATAL);
      QString aMsg;
      if (theIsAddition)
	aMsg = (isFatal ? "SMESH_CANT_ADD_HYP" : "SMESH_ADD_HYP_WRN");
      else
	aMsg = (isFatal ? "SMESH_CANT_RM_HYP"  : "SMESH_RM_HYP_WRN");

      aMsg = QObject::tr(aMsg).arg(aHypName) +
	QObject::tr(QString("SMESH_HYP_%1").arg(theHypStatus));

      if ( theHypStatus == SMESH::HYP_HIDDEN_ALGO ) // PAL18501
        aMsg = aMsg.arg( GetHypothesisData(theHyp->GetName())->Dim[0] );

      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			    QObject::tr("SMESH_WRN_WARNING"),
			    aMsg,
			    QObject::tr("SMESH_BUT_OK"));
    }
  }


  void InitAvailableHypotheses()
  {
    SUIT_OverrideCursor wc;
    if (myHypothesesMap.empty() && myAlgorithmsMap.empty()) {
      // Resource manager
      SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();
      if (!resMgr) return;

      // Find name of a resource XML file ("SMESH_Meshers.xml");
      QString HypsXml;
      char* cenv = getenv("SMESH_MeshersList");
      if (cenv)
	HypsXml.sprintf("%s", cenv);

      QStringList HypsXmlList = QStringList::split(":", HypsXml, false);
      if (HypsXmlList.count() == 0)
	{
	  SUIT_MessageBox::error1(SMESHGUI::desktop(),
				 QObject::tr("SMESH_WRN_WARNING"),
				 QObject::tr("MESHERS_FILE_NO_VARIABLE"),
				 QObject::tr("SMESH_BUT_OK"));
	  return;
	}

      // loop on files in HypsXml
      QString aNoAccessFiles;
      for (int i = 0; i < HypsXmlList.count(); i++) {
	QString HypsXml = HypsXmlList[ i ];

	// Find full path to the resource XML file
	QString xmlFile = resMgr->path("resources", "SMESH", HypsXml + ".xml");
        if ( xmlFile.isEmpty() ) // try PLUGIN resources
          xmlFile = resMgr->path("resources", HypsXml, HypsXml + ".xml");
        
	QFile file (xmlFile);
	if (file.exists() && file.open(IO_ReadOnly)) {
	  file.close();

	  SMESHGUI_XmlHandler* aXmlHandler = new SMESHGUI_XmlHandler();
	  ASSERT(aXmlHandler);

	  QXmlInputSource source (file);
	  QXmlSimpleReader reader;
	  reader.setContentHandler(aXmlHandler);
	  reader.setErrorHandler(aXmlHandler);
	  bool ok = reader.parse(source);
	  file.close();
	  if (ok) {
            myHypothesesMap.insert( aXmlHandler->myHypothesesMap.begin(),
                                    aXmlHandler->myHypothesesMap.end() );
            myAlgorithmsMap.insert( aXmlHandler->myAlgorithmsMap.begin(),
                                    aXmlHandler->myAlgorithmsMap.end() );
            myListOfHypothesesSets.splice( myListOfHypothesesSets.begin(),
                                           aXmlHandler->myListOfHypothesesSets );
	  }
	  else {
	    SUIT_MessageBox::error1(SMESHGUI::desktop(),
				   QObject::tr("INF_PARSE_ERROR"),
				   QObject::tr(aXmlHandler->errorProtocol()),
				   QObject::tr("SMESH_BUT_OK"));
	  }
	}
	else {
	  if (aNoAccessFiles.isEmpty())
	    aNoAccessFiles = xmlFile;
	  else
	    aNoAccessFiles += ", " + xmlFile;
	}
      } // end loop


      if (!aNoAccessFiles.isEmpty()) {
	QString aMess = QObject::tr("MESHERS_FILE_CANT_OPEN") + " " + aNoAccessFiles + "\n";
	aMess += QObject::tr("MESHERS_FILE_CHECK_VARIABLE");
	wc.suspend();
	SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			      QObject::tr("SMESH_WRN_WARNING"),
			      aMess,
			      QObject::tr("SMESH_BUT_OK"));
	wc.resume();
      }
    }
  }


  QStringList GetAvailableHypotheses( const bool isAlgo, 
                                      const int theDim,                          
                                      const bool isAux,
                                      const bool isNeedGeometry)
  {
    QStringList aHypList;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();
    bool checkGeometry = !isNeedGeometry;
    // fill list of hypotheses/algorithms
    THypothesisDataMap* pMap = isAlgo ? &myAlgorithmsMap : &myHypothesesMap;
    THypothesisDataMap::iterator anIter;
    for ( anIter = pMap->begin(); anIter != pMap->end(); anIter++ )
    {
      HypothesisData* aData = (*anIter).second;
      if ( ( theDim < 0 || aData->Dim.contains( theDim ) ) && aData->IsAux == isAux)
        if (checkGeometry){
          if (aData->IsNeedGeometry == isNeedGeometry)
            aHypList.append(((*anIter).first).c_str());
        }
        else
          aHypList.append(((*anIter).first).c_str());
    }
    return aHypList;
  }


  QStringList GetHypothesesSets()
  {
    QStringList aSetNameList;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    list<HypothesesSet*>::iterator hypoSet = myListOfHypothesesSets.begin();
    for ( ; hypoSet != myListOfHypothesesSets.end(); ++hypoSet )
    {
      HypothesesSet* aSet = *hypoSet;
      if ( aSet && aSet->AlgoList.count() ) {
        aSetNameList.append( aSet->HypoSetName );
      }
    }

    return aSetNameList;
  }

  HypothesesSet* GetHypothesesSet(const QString theSetName)
  {
    list<HypothesesSet*>::iterator hypoSet = myListOfHypothesesSets.begin();
    for ( ; hypoSet != myListOfHypothesesSets.end(); ++hypoSet )
    {
      HypothesesSet* aSet = *hypoSet;
      if ( aSet && aSet->HypoSetName == theSetName )
        return aSet;
    }
    return 0;
  }

  HypothesisData* GetHypothesisData (const char* aHypType)
  {
    HypothesisData* aHypData = 0;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    THypothesisDataMap::iterator type_data = myHypothesesMap.find(aHypType);
    if (type_data != myHypothesesMap.end()) {
      aHypData = type_data->second;
    }
    else {
      type_data = myAlgorithmsMap.find(aHypType);
      if (type_data != myAlgorithmsMap.end())
        aHypData = type_data->second;
    }
    return aHypData;
  }

  bool IsAvailableHypothesis(const HypothesisData* algoData,
                             const QString&        hypType,
                             bool&                 isAuxiliary)
  {
    isAuxiliary = false;
    if ( !algoData )
      return false;
    if ( algoData->NeededHypos.contains( hypType ))
      return true;
    if ( algoData->OptionalHypos.contains( hypType)) {
      isAuxiliary = true;
      return true;
    }
    return false;
  }

  bool IsCompatibleAlgorithm(const HypothesisData* algo1Data,
                             const HypothesisData* algo2Data)
  {
    if ( !algo1Data || !algo2Data )
      return false;
    const HypothesisData* algoIn = algo1Data, *algoMain = algo2Data;
    if ( algoIn->Dim.first() > algoMain->Dim.first() ) {
      algoIn = algo2Data; algoMain = algo1Data;
    }
    // look for any output type of algoIn between input types of algoMain
    QStringList::const_iterator inElemType = algoIn->OutputTypes.begin();
    for ( ; inElemType != algoIn->OutputTypes.end(); ++inElemType )
      if ( algoMain->InputTypes.contains( *inElemType ))
        return true;
    return false;
  }

  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator(const char* aHypType)
  {
    if(MYDEBUG) MESSAGE("Get HypothesisCreator for " << aHypType);

    SMESHGUI_GenericHypothesisCreator* aCreator = 0;

    // check, if creator for this hypothesis type already exists
    if (myHypCreatorMap.find(aHypType) != myHypCreatorMap.end()) {
      aCreator = myHypCreatorMap[aHypType];
    }
    else {
      // 1. Init list of available hypotheses, if needed
      InitAvailableHypotheses();

      // 2. Get names of plugin libraries
      HypothesisData* aHypData = GetHypothesisData(aHypType);
      if (!aHypData) 
        return aCreator;
      QString aClientLibName = aHypData->ClientLibName;
      QString aServerLibName = aHypData->ServerLibName;

      // 3. Load Client Plugin Library
      try {
	// load plugin library
	if(MYDEBUG) MESSAGE("Loading client meshers plugin library ...");
	LibHandle libHandle = LoadLib( aClientLibName );
	if (!libHandle) {
	  // report any error, if occured
    if ( MYDEBUG )
    {
#ifdef WIN32
      const char* anError = "Can't load client meshers plugin library";
#else
	    const char* anError = dlerror();	  
#endif
      MESSAGE(anError);
    }
	}
	else {
	  // get method, returning hypothesis creator
	  if(MYDEBUG) MESSAGE("Find GetHypothesisCreator() method ...");
	  typedef SMESHGUI_GenericHypothesisCreator* (*GetHypothesisCreator) \
	    ( const QString& );
	  GetHypothesisCreator procHandle =
	    (GetHypothesisCreator)GetProc(libHandle, "GetHypothesisCreator");
	  if (!procHandle) {
	    if(MYDEBUG) MESSAGE("bad hypothesis client plugin library");
	    UnLoadLib(libHandle);
	  }
	  else {
	    // get hypothesis creator
	    if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << aHypType);
	    aCreator = procHandle( aHypType );
	    if (!aCreator) {
	      if(MYDEBUG) MESSAGE("no such a hypothesis in this plugin");
	    }
	    else {
	      // map hypothesis creator to a hypothesis name
	      myHypCreatorMap[aHypType] = aCreator;
	    }
	  }
	}
      }
      catch (const SALOME::SALOME_Exception& S_ex) {
	SalomeApp_Tools::QtCatchCorbaException(S_ex);
      }
    }

    return aCreator;
  }


  SMESH::SMESH_Hypothesis_ptr CreateHypothesis(const char* aHypType,
					       const char* aHypName,
					       const bool isAlgo)
  {
    if(MYDEBUG) MESSAGE("Create " << aHypType << " with name " << aHypName);
    HypothesisData* aHypData = GetHypothesisData(aHypType);
    QString aServLib = aHypData->ServerLibName;
    try {
      SMESH::SMESH_Hypothesis_var aHypothesis;
      aHypothesis = SMESHGUI::GetSMESHGen()->CreateHypothesis(aHypType, aServLib);
      if (!aHypothesis->_is_nil()) {
	_PTR(SObject) aHypSObject = SMESH::FindSObject(aHypothesis.in());
	if (aHypSObject) {
	  if (strlen(aHypName) > 0)
	    SMESH::SetName(aHypSObject, aHypName);
	  SMESHGUI::GetSMESHGUI()->updateObjBrowser();
	  return aHypothesis._retn();
	}
      }
    } catch (const SALOME::SALOME_Exception & S_ex) {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
    }

    return SMESH::SMESH_Hypothesis::_nil();
  }


  bool AddHypothesisOnMesh (SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp)
  {
    if(MYDEBUG) MESSAGE ("SMESHGUI::AddHypothesisOnMesh");
    int res = SMESH::HYP_UNKNOWN_FATAL;
    SUIT_OverrideCursor wc;

    if (!aMesh->_is_nil()) {
      _PTR(SObject) SM = SMESH::FindSObject(aMesh);
      GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(SM);
      try {
	res = aMesh->AddHypothesis(aShapeObject, aHyp);
	if (res < SMESH::HYP_UNKNOWN_FATAL) {
	  _PTR(SObject) aSH = SMESH::FindSObject(aHyp);
	  if (SM && aSH) {
	    SMESH::ModifiedMesh(SM, false, aMesh->NbNodes()==0);
	  }
	}
	if (res > SMESH::HYP_OK) {
	  wc.suspend();
	  processHypothesisStatus(res, aHyp, true);
	  wc.resume();
	}
      }
      catch(const SALOME::SALOME_Exception& S_ex) {
	wc.suspend();
	SalomeApp_Tools::QtCatchCorbaException(S_ex);
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }


  bool AddHypothesisOnSubMesh (SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp)
  {
    if(MYDEBUG) MESSAGE("SMESHGUI::AddHypothesisOnSubMesh() ");
    int res = SMESH::HYP_UNKNOWN_FATAL;
    SUIT_OverrideCursor wc;

    if (!aSubMesh->_is_nil() && ! aHyp->_is_nil()) {
      try {
	SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
	_PTR(SObject) SsubM = SMESH::FindSObject(aSubMesh);
	GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(SsubM);
	if (!aMesh->_is_nil() && SsubM && !aShapeObject->_is_nil()) {
	  res = aMesh->AddHypothesis(aShapeObject, aHyp);
	  if (res < SMESH::HYP_UNKNOWN_FATAL)  {
            _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
            if (meshSO)
              SMESH::ModifiedMesh(meshSO, false, aMesh->NbNodes()==0);
	  }
	  if (res > SMESH::HYP_OK) {
	    wc.suspend();
	    processHypothesisStatus(res, aHyp, true);
	    wc.resume();
	  }
	}
	else {
	  SCRUTE(aHyp->_is_nil());
	  SCRUTE(aMesh->_is_nil());
	  SCRUTE(!SsubM);
	  SCRUTE(aShapeObject->_is_nil());
	}
      }
      catch(const SALOME::SALOME_Exception& S_ex) {
	wc.suspend();
	SalomeApp_Tools::QtCatchCorbaException(S_ex);
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    else {
      SCRUTE(aSubMesh->_is_nil());
      SCRUTE(aHyp->_is_nil());
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }

  bool RemoveHypothesisOrAlgorithmOnMesh (const Handle(SALOME_InteractiveObject)& IObject)
  {
    int res = SMESH::HYP_UNKNOWN_FATAL;
    SUIT_OverrideCursor wc;

    try {
      _PTR(Study) aStudy = GetActiveStudyDocument();
      _PTR(SObject) aHypObj = aStudy->FindObjectID( IObject->getEntry() );
      if( aHypObj )
      {
	_PTR(SObject) MorSM = SMESH::GetMeshOrSubmesh( aHypObj );
	_PTR(SObject) aRealHypo;
	if( aHypObj->ReferencedObject( aRealHypo ) )
	{
	  SMESH_Hypothesis_var hypo = SMESH_Hypothesis::_narrow( SObjectToObject( aRealHypo ) );
	  RemoveHypothesisOrAlgorithmOnMesh( MorSM, hypo );
	}
	else
	{
	  SMESH_Hypothesis_var hypo = SMESH_Hypothesis::_narrow( SObjectToObject( aHypObj ) );
	  SObjectList meshList = GetMeshesUsingAlgoOrHypothesis( hypo );
	  for( int i = 0; i < meshList.size(); i++ )
	    RemoveHypothesisOrAlgorithmOnMesh( meshList[ i ], hypo );
	}
      }
    }
    catch(const SALOME::SALOME_Exception& S_ex)
    {
      wc.suspend();
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }

  bool RemoveHypothesisOrAlgorithmOnMesh (_PTR(SObject) MorSM,
                                          SMESH::SMESH_Hypothesis_ptr anHyp)
  {
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    int res = SMESH::HYP_UNKNOWN_FATAL;
    SUIT_OverrideCursor wc;

    if (MorSM) {
      try {
        GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(MorSM);
        SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);
        
        if (!aSubMesh->_is_nil())
          aMesh = aSubMesh->GetFather();
        
        if (!aMesh->_is_nil()) {    
          if (aMesh->HasShapeToMesh() && !aShapeObject->_is_nil()) {
            res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
            if (res < SMESH::HYP_UNKNOWN_FATAL) {
              _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
              if (meshSO)
                SMESH::ModifiedMesh(meshSO, false, aMesh->NbNodes()==0);
            }
            
          }
          else if(!aMesh->HasShapeToMesh()){
            res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
	    if (res < SMESH::HYP_UNKNOWN_FATAL) {
              _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
              if (meshSO)
                SMESH::ModifiedMesh(meshSO, false, aMesh->NbNodes()==0);              
            }
          }
          if (res > SMESH::HYP_OK) {
            wc.suspend();
            processHypothesisStatus(res, anHyp, false);
            wc.resume();
          }
        }
      } catch(const SALOME::SALOME_Exception& S_ex) {
	wc.suspend();
	SalomeApp_Tools::QtCatchCorbaException(S_ex);
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }

  SObjectList GetMeshesUsingAlgoOrHypothesis(SMESH::SMESH_Hypothesis_ptr AlgoOrHyp)
  {
    SObjectList listSOmesh;
    listSOmesh.resize(0);

    unsigned int index = 0;
    if (!AlgoOrHyp->_is_nil()) {
      _PTR(SObject) SO_Hypothesis = SMESH::FindSObject(AlgoOrHyp);
      if (SO_Hypothesis) {
	SObjectList listSO =
	  SMESHGUI::activeStudy()->studyDS()->FindDependances(SO_Hypothesis);

	if(MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency number ="<<listSO.size());
	for (unsigned int i = 0; i < listSO.size(); i++) {
	  _PTR(SObject) SO = listSO[i];
	  if (SO) {
	    _PTR(SObject) aFather = SO->GetFather();
	    if (aFather) {
	      _PTR(SObject) SOfatherFather = aFather->GetFather();
	      if (SOfatherFather) {
		if(MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency added to list");
		index++;
		listSOmesh.resize(index);
		listSOmesh[index - 1] = SOfatherFather;
	      }
	    }
	  }
	}
      }
    }
    if (MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): completed");
    return listSOmesh;
  }

#define CASE2MESSAGE(enum) case SMESH::enum: msg = QObject::tr( "STATE_" #enum ); break;
  QString GetMessageOnAlgoStateErrors(const algo_error_array& errors)
  {
    QString resMsg; // PAL14861 = QObject::tr("SMESH_WRN_MISSING_PARAMETERS") + ":\n";
    for ( int i = 0; i < errors.length(); ++i ) {
      const SMESH::AlgoStateError & error = errors[ i ];
      const bool hasAlgo = ( strlen( error.algoName ) != 0 );
      QString msg;
      if ( !hasAlgo )
        msg = QObject::tr( "STATE_ALGO_MISSING" );
      else 
        switch( error.state ) {
          CASE2MESSAGE( HYP_MISSING );
          CASE2MESSAGE( HYP_NOTCONFORM );
          CASE2MESSAGE( HYP_BAD_PARAMETER );
          CASE2MESSAGE( HYP_BAD_GEOMETRY );
        default: continue;
        }
      // apply args to message:
      // %1 - algo name
      if ( hasAlgo )
        msg = msg.arg( error.algoName.in() );
      // %2 - dimension
      msg = msg.arg( error.algoDim );
      // %3 - global/local
      msg = msg.arg( QObject::tr( error.isGlobalAlgo ? "GLOBAL_ALGO" : "LOCAL_ALGO" ));
      // %4 - hypothesis dim == algoDim
      msg = msg.arg( error.algoDim );

      if ( i ) resMsg += ";\n";
      resMsg += msg;
    }
    return resMsg;
  }

}
