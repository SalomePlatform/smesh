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

#include <map>
#include <string>

#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Utils.h"

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

  void InitAvailableHypotheses()
  {
    QAD_WaitCursor wc;
    if (myHypothesesMap.empty() && myAlgorithmsMap.empty()) {
      // Resource manager
      QAD_ResourceMgr* resMgr = QAD_Desktop::createResourceManager();
      if (!resMgr) return;
      
      // Find name of a resource XML file ("SMESH_Meshers.xml");
      QString HypsXml;
      char* cenv = getenv("SMESH_MeshersList");
      if (cenv)
	HypsXml.sprintf("%s", cenv);
      
      QStringList HypsXmlList = QStringList::split( ":", HypsXml, false );
      if (HypsXmlList.count() == 0)
	{
	  QAD_MessageBox::error1(QAD_Application::getDesktop(),
				 tr("SMESH_WRN_WARNING"),
				 tr("MESHERS_FILE_NO_VARIABLE"),
				 tr("SMESH_BUT_OK"));
	  return;
	}
      
      // loop on files in HypsXml
      QString aNoAccessFiles;
      for ( int i = 0; i < HypsXmlList.count(); i++ ) {
	QString HypsXml = HypsXmlList[ i ];
	
	// Find full path to the resource XML file
	QString xmlFile = HypsXml + ".xml";
	xmlFile = QAD_Tools::addSlash(resMgr->findFile(xmlFile, HypsXml)) + xmlFile;
	
	QFile file (QAD_Tools::unix2win(xmlFile));
	if (file.exists() && file.open(IO_ReadOnly)) {
	  file.close();
	  
	  SMESHGUI_XmlHandler* myXmlHandler = new SMESHGUI_XmlHandler();
	  ASSERT(myXmlHandler);
	  
	  QXmlInputSource source (file);
	  QXmlSimpleReader reader;
	  reader.setContentHandler(myXmlHandler);
	  reader.setErrorHandler(myXmlHandler);
	  bool ok = reader.parse(source);
	  file.close();
	  if (ok) {
	    addMap( myXmlHandler->myHypothesesMap, myHypothesesMap );
	    addMap( myXmlHandler->myAlgorithmsMap, myAlgorithmsMap );
	  }
	  else {
	    QAD_MessageBox::error1(myDesktop, 
				   tr("INF_PARSE_ERROR"),
				   tr(myXmlHandler->errorProtocol()),
				   tr("SMESH_BUT_OK"));
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
	QString aMess = tr("MESHERS_FILE_CANT_OPEN") + " " + aNoAccessFiles + "\n";
	aMess += tr("MESHERS_FILE_CHECK_VARIABLE");
	wc.stop();
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"),
			      aMess,
			      tr("SMESH_BUT_OK"));
	wc.start();
      }
    }
  }


  GetAvailableHypotheses (const bool isAlgo)
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


  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator (const QString& aHypType)
  {
    const char* sHypType = aHypType.latin1();
    if(MYDEBUG) MESSAGE("Get HypothesisCreator for " << sHypType);

    SMESHGUI_GenericHypothesisCreator* aCreator = 0;
    
    // check, if creator for this hypothesis type already exists
    if (myHypCreatorMap.find(sHypType) != myHypCreatorMap.end()) {
      aCreator = myHypCreatorMap[sHypType];
    }
    else {
      // 1. Init list of available hypotheses, if needed
      InitAvailableHypotheses();

      // 2. Get names of plugin libraries
      HypothesisData* aHypData = GetHypothesisData(sHypType);
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
	    (GetHypothesisCreator)dlsym( libHandle, "GetHypothesisCreator" );
	  if (!procHandle) {
	    if(MYDEBUG) MESSAGE("bad hypothesis client plugin library");
	    dlclose(libHandle);
	  }
	  else {
	    // get hypothesis creator
	    if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << aHypType);
	    aCreator = procHandle(aHypType, aServerLibName, smeshGUI);
	    if (!aCreator) {
	      if(MYDEBUG) MESSAGE("no such a hypothesis in this plugin");
	    }
	    else {
	      // map hypothesis creator to a hypothesis name
	      myHypCreatorMap[sHypType] = aCreator;
	    }
	  }
	}
      }
      catch (const SALOME::SALOME_Exception& S_ex) {
	QtCatchCorbaException(S_ex);
      }
    }

    return aCreator;
  }


  SMESH::SMESH_Hypothesis_ptr CreateHypothesis (const QString& aHypType,
						const QString& aHypName,
						const bool     isAlgo = false)
  {
    if(MYDEBUG) MESSAGE("Create " << aHypType << " with name " << aHypName);

    SMESH::SMESH_Hypothesis_var Hyp;
    
    HypothesisData* aHypData = GetHypothesisData((char*)aHypType.latin1());
    QString aServLib = aHypData->ServerLibName;
    
    try {
      Hyp = SMESH::GetSMESHGen()->CreateHypothesis(aHypType, aServLib);
      if (!Hyp->_is_nil()) {
	SALOMEDS::SObject_var SHyp = SMESH::FindSObject(Hyp.in());
	if (!SHyp->_is_nil()) {
	  if ( !aHypName.isEmpty() )
	    SMESH::SetName( SHyp, aHypName );
	  myActiveStudy->updateObjBrowser(true);
	  return Hyp._retn();
	}
      }
    }
    catch (const SALOME::SALOME_Exception & S_ex) {
      QtCatchCorbaException(S_ex);
    }

    return SMESH::SMESH_Hypothesis::_nil();
  }


  bool AddHypothesisOnMesh (SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp)
  {
    if(MYDEBUG) MESSAGE ("SMESHGUI::AddHypothesisOnMesh");
    int res = SMESH::HYP_UNKNOWN_FATAL; 
    QAD_WaitCursor wc;
    
    if ( !aMesh->_is_nil() ) {
      SALOMEDS::SObject_var SM = SMESH::FindSObject( aMesh );
      GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(SM);
      try {
	res = aMesh->AddHypothesis( aShapeObject, aHyp );
	if ( res < SMESH::HYP_UNKNOWN_FATAL ) {
	  SALOMEDS::SObject_var SH = SMESH::FindSObject(aHyp);
	  if ( !SM->_is_nil() && !SH->_is_nil() ) {
	    SMESH::ModifiedMesh(SM, false);
	  }
	}
	if ( res > SMESH::HYP_OK ) {
	  wc.stop();
	  processHypothesisStatus( res, aHyp, this, true );
	  wc.start();
	}
      }
      catch( const SALOME::SALOME_Exception& S_ex ) {
	wc.stop();
	QtCatchCorbaException( S_ex );
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }


  bool AddHypothesisOnSubMesh (SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp)
  {
    if(MYDEBUG) MESSAGE( "SMESHGUI::AddHypothesisOnSubMesh() ");
    int res = SMESH::HYP_UNKNOWN_FATAL;
    QAD_WaitCursor wc;
    
    if ( !aSubMesh->_is_nil() && ! aHyp->_is_nil() ) {
      try {
	SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
	SALOMEDS::SObject_var SsubM = SMESH::FindSObject( aSubMesh );
	GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh( SsubM );
	if ( !aMesh->_is_nil() && !SsubM->_is_nil() && !aShapeObject->_is_nil() ) {
	  res = aMesh->AddHypothesis( aShapeObject, aHyp );
	  if ( res < SMESH::HYP_UNKNOWN_FATAL )  {
	    SMESH::ModifiedMesh( SsubM, false );
	  }
	  if ( res > SMESH::HYP_OK ) {
	    wc.stop();
	    processHypothesisStatus( res, aHyp, this, true );
	    wc.start();
	  }
	}
	else {
	  SCRUTE( aHyp->_is_nil() );
	  SCRUTE( aMesh->_is_nil() );
	  SCRUTE( SsubM->_is_nil() );
	  SCRUTE( aShapeObject->_is_nil() );
	}
      }
      catch( const SALOME::SALOME_Exception& S_ex ) {
	wc.stop();
	QtCatchCorbaException( S_ex );
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    else {
      SCRUTE( aSubMesh->_is_nil() );
      SCRUTE( aHyp->_is_nil() );
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }
  
  bool RemoveHypothesisOrAlgorithmOnMesh (const Handle(SALOME_InteractiveObject)& IObject)
  {
    int res = SMESH::HYP_UNKNOWN_FATAL;
    QAD_WaitCursor wc;
    
    if (IObject->hasReference()) {
      try {
	SMESH::SMESH_Hypothesis_var anHyp = 
	  SMESH::IObjectToInterface<SMESH::SMESH_Hypothesis>(IObject);
	SALOMEDS::SObject_var SO_Applied_Hypothesis =
	  smeshGUI->myStudy->FindObjectID(IObject->getReference());
	if (!SO_Applied_Hypothesis->_is_nil()) {
	  SALOMEDS::SObject_var MorSM = SMESH::GetMeshOrSubmesh(SO_Applied_Hypothesis);
	  if (!MorSM->_is_nil()) {
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
		if ( res < SMESH::HYP_UNKNOWN_FATAL )
		  SMESH::ModifiedMesh(MorSM, false);
		if ( res > SMESH::HYP_OK ) {
		  wc.stop();
		  processHypothesisStatus( res, anHyp, this, false );
		  wc.start();
		}
	      }
	    }
	  }
	  
	  SALOMEDS::SObject_var SO_Applied_Hypothesis =
	    smeshGUI->myStudy->FindObjectID(IObject->getReference());
	  if (!SO_Applied_Hypothesis->_is_nil()) {
	    SALOMEDS::SObject_var MorSM = SMESH::GetMeshOrSubmesh(SO_Applied_Hypothesis);
	    if (!MorSM->_is_nil()) {
	      GEOM::GEOM_Object_var aShapeObject =SMESH::GetShapeOnMeshOrSubMesh(MorSM);
	      if (!aShapeObject->_is_nil()){
		SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
		SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);
		
		if (!aSubMesh->_is_nil())
		  aMesh = aSubMesh->GetFather();
		
		if (!aMesh->_is_nil()) {
		  res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
		  if ( res < SMESH::HYP_UNKNOWN_FATAL )
		    SMESH::ModifiedMesh(MorSM, false);
		  if ( res > SMESH::HYP_OK ) {
		    wc.stop();
		    processHypothesisStatus( res, anHyp, this, false );
		    wc.start();
		  }
		}
	      }
	    }
	  }
	}
      }
      catch( const SALOME::SALOME_Exception& S_ex ) {
	wc.stop();
	QtCatchCorbaException( S_ex );
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    } 
    else if (IObject->hasEntry()) {
      if(MYDEBUG) MESSAGE("IObject entry " << IObject->getEntry());
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }
  
  bool RemoveHypothesisOrAlgorithmOnMesh (SALOMEDS::SObject_ptr MorSM,
					  SMESH::SMESH_Hypothesis_ptr anHyp)
  {
    SALOMEDS::SObject_var AHR, aRef;
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    int res = SMESH::HYP_UNKNOWN_FATAL;
    QAD_WaitCursor wc;
    
    if (!MorSM->_is_nil()) {
      try {
	GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(MorSM);
	if (!aShapeObject->_is_nil()) {
	  SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
	  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);
	  
	  if ( !aSubMesh->_is_nil() )
	    aMesh = aSubMesh->GetFather();
	  
	  if (!aMesh->_is_nil()) {
	    res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
	    if ( res < SMESH::HYP_UNKNOWN_FATAL )
	      SMESH::ModifiedMesh(MorSM, false);
	    if ( res > SMESH::HYP_OK ) {
	      wc.stop();
	      processHypothesisStatus( res, anHyp, this, false );
	      wc.start();
	    }
	  }
	}
      } catch( const SALOME::SALOME_Exception& S_ex ) {
	wc.stop();
	QtCatchCorbaException( S_ex );
	res = SMESH::HYP_UNKNOWN_FATAL;
      }
    }
    return res < SMESH::HYP_UNKNOWN_FATAL;
  }

  SALOMEDS::Study::ListOfSObject* GetMeshesUsingAlgoOrHypothesis( SMESH::SMESH_Hypothesis_ptr AlgoOrHyp )
  {
    SALOMEDS::Study::ListOfSObject_var listSOmesh =
      new SALOMEDS::Study::ListOfSObject;
    listSOmesh->length(0);
    unsigned int index = 0;
    if (!AlgoOrHyp->_is_nil()) {
      SALOMEDS::SObject_var SO_Hypothesis = SMESH::FindSObject(AlgoOrHyp);
      if (!SO_Hypothesis->_is_nil()) {
	SALOMEDS::Study::ListOfSObject_var listSO =
	  smeshGUI->myStudy->FindDependances(SO_Hypothesis);
	if(MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency number ="<<listSO->length());
	for (unsigned int i = 0; i < listSO->length(); i++) {
	  SALOMEDS::SObject_ptr SO = listSO[i];
	  if (!SO->_is_nil()) { 
	    SALOMEDS::SObject_var aFather = SO->GetFather();
	    if (!aFather->_is_nil()) {
	      SALOMEDS::SObject_var SOfatherFather = aFather->GetFather();
	      if (!SOfatherFather->_is_nil()) {
		if(MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency added to list");
		index++;
		listSOmesh->length(index);
		listSOmesh[index - 1] = SOfatherFather;
	      }
	    }
	  }
	}
      }
    }
    if(MYDEBUG) MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): completed");
    return listSOmesh._retn();
  }

}
