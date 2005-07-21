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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org

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

#include <dlfcn.h>

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

  void addMap(const THypothesisDataMap& theMap,
	       THypothesisDataMap& toMap)
  {
    THypothesisDataMap::const_iterator it;
    for (it = theMap.begin(); it != theMap.end(); it++)
      toMap.insert(*it);
  }


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
	    addMap(aXmlHandler->myHypothesesMap, myHypothesesMap);
	    addMap(aXmlHandler->myAlgorithmsMap, myAlgorithmsMap);
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


  QStringList GetAvailableHypotheses(const bool isAlgo)
  {
    QStringList aHypList;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    // fill list of hypotheses/algorithms
    THypothesisDataMap::iterator anIter;
    if (isAlgo) {
      anIter = myAlgorithmsMap.begin();
      for (; anIter != myAlgorithmsMap.end(); anIter++) {
	aHypList.append(((*anIter).first).c_str());
      }
    }
    else {
      anIter = myHypothesesMap.begin();
      for (; anIter != myHypothesesMap.end(); anIter++) {
	aHypList.append(((*anIter).first).c_str());
      }
    }

    return aHypList;
  }


  HypothesisData* GetHypothesisData (const char* aHypType)
  {
    HypothesisData* aHypData = 0;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    if (myHypothesesMap.find(aHypType) == myHypothesesMap.end()) {
      if (myAlgorithmsMap.find(aHypType) != myAlgorithmsMap.end()) {
	aHypData = myAlgorithmsMap[aHypType];
      }
    }
    else {
      aHypData = myHypothesesMap[aHypType];
    }
    return aHypData;
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
      if (!aHypData) {
	return aCreator;
      }
      QString aClientLibName = aHypData->ClientLibName;
      QString aServerLibName = aHypData->ServerLibName;

      // 3. Load Client Plugin Library
      try {
	// load plugin library
	if(MYDEBUG) MESSAGE("Loading client meshers plugin library ...");
	void* libHandle = dlopen (aClientLibName, RTLD_LAZY);
	if (!libHandle) {
	  // report any error, if occured
	  const char* anError = dlerror();
	  if(MYDEBUG) MESSAGE(anError);
	}
	else {
	  // get method, returning hypothesis creator
	  if(MYDEBUG) MESSAGE("Find GetHypothesisCreator() method ...");
	  typedef SMESHGUI_GenericHypothesisCreator* (*GetHypothesisCreator) \
	    (QString aHypType, QString aServerLibName, SMESHGUI* aSMESHGUI);
	  GetHypothesisCreator procHandle =
	    (GetHypothesisCreator)dlsym(libHandle, "GetHypothesisCreator");
	  if (!procHandle) {
	    if(MYDEBUG) MESSAGE("bad hypothesis client plugin library");
	    dlclose(libHandle);
	  }
	  else {
	    // get hypothesis creator
	    if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << aHypType);
	    aCreator = procHandle(aHypType, aServerLibName, SMESHGUI::GetSMESHGUI());
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

    SMESH::SMESH_Hypothesis_var Hyp;

    HypothesisData* aHypData = GetHypothesisData(aHypType);
    QString aServLib = aHypData->ServerLibName;

    try {
      Hyp = SMESHGUI::GetSMESHGen()->CreateHypothesis(aHypType, aServLib);
      if (!Hyp->_is_nil()) {
	_PTR(SObject) SHyp = SMESH::FindSObject(Hyp.in());
	if (SHyp) {
	  //if (strcmp(aHypName,"") != 0)
	  if (strlen(aHypName) > 0)
	    SMESH::SetName(SHyp, aHypName);
	  //SalomeApp_Application* app =
	  //  dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication());
	  //if (app)
	  //  app->objectBrowser()->updateTree();
          SMESHGUI::GetSMESHGUI()->updateObjBrowser();
	  return Hyp._retn();
	}
      }
    }
    catch (const SALOME::SALOME_Exception & S_ex) {
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
	  _PTR(SObject) SH = SMESH::FindSObject(aHyp);
	  if (SM && SH) {
	    SMESH::ModifiedMesh(SM, false);
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
              SMESH::ModifiedMesh(meshSO, false);
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
      SMESH_Hypothesis_var anHyp = IObjectToInterface<SMESH_Hypothesis>(IObject);
      if (IObject->hasReference())
      {
        if(MYDEBUG) MESSAGE("HAS REFERENCE " << IObject->getEntry());
	_PTR(SObject) aHypSObj = aStudy->FindObjectID(IObject->getReference());
	if (aHypSObj) {
	  _PTR(SObject) MorSM = SMESH::GetMeshOrSubmesh(aHypSObj);
	  if (MorSM) {
	    GEOM::GEOM_Object_var aShape = SMESH::GetShapeOnMeshOrSubMesh(MorSM);
	    if (!aShape->_is_nil()){
	      SMESH::SMESH_Mesh_var aMesh =
		SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
	      SMESH::SMESH_subMesh_var aSubMesh =
		SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);

	      if (!aSubMesh->_is_nil())
		aMesh = aSubMesh->GetFather();

	      if (!aMesh->_is_nil()) {
		res = aMesh->RemoveHypothesis(aShape, anHyp);
		if (res < SMESH::HYP_UNKNOWN_FATAL) {
                  _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
                  if (meshSO)
                    SMESH::ModifiedMesh(meshSO, false);
                }
		if (res > SMESH::HYP_OK) {
		  wc.suspend();
		  processHypothesisStatus(res, anHyp, false);
		  wc.resume();
		}
	      }
	    }
	  }
	}
      }
      else if (IObject->hasEntry())
      {
        if(MYDEBUG) MESSAGE("IObject entry HAS ENTRY " << IObject->getEntry());
        SObjectList meshList = GetMeshesUsingAlgoOrHypothesis(anHyp);
        for ( int i = 0; i < meshList.size(); ++i ) {
          SMESH::SMESH_Mesh_var aMesh = 
            SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( meshList[ i ]);
          GEOM::GEOM_Object_var aShape = SMESH::GetShapeOnMeshOrSubMesh( meshList[ i ]);
          if ( ! aMesh->_is_nil() && !aShape->_is_nil() ) {
            res = aMesh->RemoveHypothesis(aShape, anHyp);
            if ( res < SMESH::HYP_UNKNOWN_FATAL )
              SMESH::ModifiedMesh( meshList[ i ], false);
            if ( res > SMESH::HYP_OK ) {
              wc.suspend();
              processHypothesisStatus( res, anHyp, false );
              wc.resume();
            }
          }
        }
      }
    }
    catch(const SALOME::SALOME_Exception& S_ex) {
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
        if (!aShapeObject->_is_nil()) {
          SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
	  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);

	  if (!aSubMesh->_is_nil())
	    aMesh = aSubMesh->GetFather();

	  if (!aMesh->_is_nil()) {
	    res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
	    if (res < SMESH::HYP_UNKNOWN_FATAL) {
              _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
              if (meshSO)
                SMESH::ModifiedMesh(meshSO, false);
            }
	    if (res > SMESH::HYP_OK) {
	      wc.suspend();
	      processHypothesisStatus(res, anHyp, false);
	      wc.resume();
	    }
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
	  if (!SO) {
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
}
