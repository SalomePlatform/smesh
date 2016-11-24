// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_HypothesesUtils.cxx
// Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_HypothesesUtils.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_XmlHandler.h"

#include "SMESH_Actor.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>

#include <SalomeApp_Study.h>
#include <SalomeApp_Tools.h>

// SALOME KERNEL includes
#include <utilities.h>

// STL includes
#include <string>

// Qt includes
#include <QDir>


// Other includes
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef WIN32
#define LibHandle HMODULE
#define LoadLib( name ) LoadLibrary( name )
#define GetProc GetProcAddress
#define UnLoadLib( handle ) FreeLibrary( handle );
#else
#define LibHandle void*
#define LoadLib( name ) dlopen( name, RTLD_LAZY | RTLD_GLOBAL )
#define GetProc dlsym
#define UnLoadLib( handle ) dlclose( handle );
#endif

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

namespace SMESH
{
  typedef IMap<QString,HypothesisData*> THypothesisDataMap;
  THypothesisDataMap myHypothesesMap;
  THypothesisDataMap myAlgorithmsMap;

  // BUG 0020378
  //typedef QMap<QString,SMESHGUI_GenericHypothesisCreator*> THypCreatorMap;
  //THypCreatorMap myHypCreatorMap;

  QList<HypothesesSet*> myListOfHypothesesSets;

  void processHypothesisStatus(const int                   theHypStatus,
                               SMESH::SMESH_Hypothesis_ptr theHyp,
                               const bool                  theIsAddition,
                               const char*                 theError = 0)
  {
    if (theHypStatus > SMESH::HYP_OK) {
      // get Hyp name
      QString aHypName ("NULL Hypothesis");
      if (!CORBA::is_nil(theHyp)) {
        _PTR(SObject) Shyp = SMESH::FindSObject(theHyp);
        if (Shyp) {
          // name in study
          aHypName = Shyp->GetName().c_str();
        }
        else {
          // label in xml file
          CORBA::String_var hypType = theHyp->GetName();
          aHypName = GetHypothesisData( hypType.in() )->Label;
        }
      }

      // message
      bool isFatal = (theHypStatus >= SMESH::HYP_UNKNOWN_FATAL);
      QString aMsg;
      if (theIsAddition)
        aMsg = (isFatal ? "SMESH_CANT_ADD_HYP" : "SMESH_ADD_HYP_WRN");
      else
        aMsg = (isFatal ? "SMESH_CANT_RM_HYP"  : "SMESH_RM_HYP_WRN");

      aMsg = QObject::tr(aMsg.toLatin1().data()).arg(aHypName);

      if ( theError && theError[0] )
      {
        aMsg += theError;
      }
      else
      {
        aMsg += QObject::tr(QString("SMESH_HYP_%1").arg(theHypStatus).toLatin1().data());

        if ( theHypStatus == SMESH::HYP_HIDDEN_ALGO ) { // PAL18501
          CORBA::String_var hypType = theHyp->GetName();
          if ( HypothesisData* hd = GetHypothesisData( hypType.in() ))
            aMsg = aMsg.arg( hd->Dim[0] );
        }
      }
      SUIT_MessageBox::warning(SMESHGUI::desktop(),
                               QObject::tr("SMESH_WRN_WARNING"),
                               aMsg);
    }
  }

  //================================================================================
  /*!
   * \brief Prepends dimension and appends '[custom]' to the name of hypothesis set
   */
  //================================================================================

  static QString mangledHypoSetName(HypothesesSet* hypSet)
  {
    QString name = hypSet->name();

    // prepend 'xD: '
    int dim = hypSet->maxDim();
    if ( dim > -1 )
      name = QString("%1D: %2").arg(dim).arg(name);

    // custom
    if ( hypSet->getIsCustom() )
      name = QString("%1 [custom]").arg(name);

    return name;
  }

  //================================================================================
  /*!
   * \brief Removes dimension and '[custom]' from the name of hypothesis set
   */
  //================================================================================

  static QString demangledHypoSetName(QString name)
  {
    name.remove(QRegExp("[0-3]D: "));
    name.remove(" [custom]");
    return name;
  }

  void InitAvailableHypotheses()
  {
    SUIT_OverrideCursor wc;
    if ( myHypothesesMap.empty() && myAlgorithmsMap.empty() )
    {
      // Resource manager
      SUIT_ResourceMgr* resMgr = SMESHGUI::resourceMgr();
      if (!resMgr) return;

      // Find name of a resource XML file ("SMESH_Meshers.xml");
      QString HypsXml;
      char* cenv = getenv("SMESH_MeshersList");
      if (cenv)
        HypsXml.sprintf("%s", cenv);

      QStringList HypsXmlList = HypsXml.split(":", QString::SkipEmptyParts);
      if (HypsXmlList.count() == 0) {
        SUIT_MessageBox::critical(SMESHGUI::desktop(),
                                  QObject::tr("SMESH_WRN_WARNING"),
                                  QObject::tr("MESHERS_FILE_NO_VARIABLE"));
        return;
      }
      // get full names of xml files from HypsXmlList
      QStringList xmlFiles;
      xmlFiles.append( QDir::home().filePath("CustomMeshers.xml")); // may be inexistent
      for (int i = 0; i < HypsXmlList.count(); i++) {
        QString HypsXml = HypsXmlList[ i ];

        // Find full path to the resource XML file
        QString xmlFile = resMgr->path("resources", "SMESH", HypsXml + ".xml");
        if ( xmlFile.isEmpty() ) // try PLUGIN resources
          xmlFile = resMgr->path("resources", HypsXml, HypsXml + ".xml");
        if ( !xmlFile.isEmpty() )
          xmlFiles.append( xmlFile );
      }

      // loop on xmlFiles
      QString aNoAccessFiles;
      for (int i = 0; i < xmlFiles.count(); i++)
      {
        QString xmlFile = xmlFiles[ i ];
        QFile file (xmlFile);
        if (file.exists() && file.open(QIODevice::ReadOnly))
        {
          file.close();

          SMESHGUI_XmlHandler* aXmlHandler = new SMESHGUI_XmlHandler();
          ASSERT(aXmlHandler);

          QXmlInputSource source (&file);
          QXmlSimpleReader reader;
          reader.setContentHandler(aXmlHandler);
          reader.setErrorHandler(aXmlHandler);
          bool ok = reader.parse(source);
          file.close();
          if (ok) {

            THypothesisDataMap::ConstIterator it1 = aXmlHandler->myHypothesesMap.begin();

            for( ;it1 != aXmlHandler->myHypothesesMap.end(); it1++)
              myHypothesesMap.insert( it1.key(), it1.value() );

            it1 = aXmlHandler->myAlgorithmsMap.begin();
            for( ;it1 != aXmlHandler->myAlgorithmsMap.end(); it1++)
              myAlgorithmsMap.insert( it1.key(), it1.value() );

            QList<HypothesesSet*>::iterator it;
            for ( it = aXmlHandler->myListOfHypothesesSets.begin();
                  it != aXmlHandler->myListOfHypothesesSets.end();
                  ++it )
            {
              (*it)->setIsCustom( i == 0 );
              myListOfHypothesesSets.append( *it );
            }
          }
          else {
            SUIT_MessageBox::critical(SMESHGUI::desktop(),
                                      QObject::tr("INF_PARSE_ERROR"),
                                      QObject::tr(aXmlHandler->errorProtocol().toLatin1().data()));
          }
          delete aXmlHandler;
        }
        else if ( i > 0 ) { // 1st is ~/CustomMeshers.xml
          if (aNoAccessFiles.isEmpty())
            aNoAccessFiles = xmlFile;
          else
            aNoAccessFiles += ", " + xmlFile;
        }
      } // end loop on xmlFiles


      if (!aNoAccessFiles.isEmpty()) {
        QString aMess = QObject::tr("MESHERS_FILE_CANT_OPEN") + " " + aNoAccessFiles + "\n";
        aMess += QObject::tr("MESHERS_FILE_CHECK_VARIABLE");
        wc.suspend();
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 QObject::tr("SMESH_WRN_WARNING"),
                                 aMess);
        wc.resume();
      }
    }
  }


  QStringList GetAvailableHypotheses( const bool isAlgo,
                                      const int  theDim,
                                      const bool isAux,
                                      const bool hasGeometry,
                                      const bool isSubMesh)
  {
    QStringList aHypList;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();
    bool checkGeometry = ( isAlgo );
    const char* context = isSubMesh ? "LOCAL" : "GLOBAL";
    // fill list of hypotheses/algorithms
    THypothesisDataMap& pMap = isAlgo ? myAlgorithmsMap : myHypothesesMap;
    THypothesisDataMap::ConstIterator anIter;
    for ( anIter = pMap.begin(); anIter != pMap.end(); anIter++ )
    {
      HypothesisData* aData = anIter.value();
      if (( aData && !aData->Label.isEmpty() ) &&
          ( theDim < 0              || aData->Dim.contains( theDim )) &&
          ( isAlgo                  || aData->IsAuxOrNeedHyp == isAux ) &&
          ( aData->Context == "ANY" || aData->Context == context ) &&
          ( !checkGeometry          || (!aData->IsNeedGeometry  ||
                                        ( aData->IsNeedGeometry > 0 ) == hasGeometry)))
      {
        aHypList.append(anIter.key());
      }
    }
    return aHypList;
  }


  QStringList GetHypothesesSets(int maxDim)
  {
    QStringList aSetNameList;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();
    QList<HypothesesSet*>::iterator hypoSet;
    for ( hypoSet  = myListOfHypothesesSets.begin();
        hypoSet != myListOfHypothesesSets.end();
        ++hypoSet ) {
      HypothesesSet* aSet = *hypoSet;
      if ( aSet && ( aSet->count( true ) || aSet->count( false )) &&
          aSet->maxDim() <= maxDim)
      {
        aSetNameList.append( mangledHypoSetName( aSet ));
      }
    }
    aSetNameList.removeDuplicates();
    aSetNameList.sort();

    //  reverse order of aSetNameList
    QStringList reversedNames;
    for ( int i = 0; i < aSetNameList.count(); ++i )
      reversedNames.prepend( aSetNameList[i] );

    return reversedNames;
  }

  HypothesesSet* GetHypothesesSet(const QString& theSetName)
  {
    QString name = demangledHypoSetName( theSetName );
    QList<HypothesesSet*>::iterator hypoSet;
    for ( hypoSet  = myListOfHypothesesSets.begin();
          hypoSet != myListOfHypothesesSets.end();
          ++hypoSet ) {
      HypothesesSet* aSet = *hypoSet;
      if ( aSet && aSet->name() == name )
        return aSet;
    }
    return 0;
  }

  HypothesisData* GetHypothesisData (const QString& aHypType)
  {
    HypothesisData* aHypData = 0;

    // Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    if (myHypothesesMap.contains(aHypType)) {
      aHypData = myHypothesesMap[aHypType];
    }
    else if (myAlgorithmsMap.contains(aHypType)) {
      aHypData = myAlgorithmsMap[aHypType];
    }
    return aHypData;
  }

  //================================================================================
  /*!
   * \brief Return the HypothesisData holding a name of a group of hypotheses
   *        a given hypothesis belongs to
   */
  //================================================================================

  HypothesisData* GetGroupTitle( const HypothesisData* hyp, const bool isAlgo )
  {
    static std::vector< std::vector< HypothesisData > > theGroups;
    if ( theGroups.empty() )
    {
      theGroups.resize(14); // 14: isAlgo * 10 + dim

      QString dummyS("GROUP");
      QList<int> dummyIL; dummyIL << 1;
      QStringList dummySL;
      HypothesisData group( dummyS,dummyS,dummyS,dummyS,dummyS,dummyS,dummyS,-1,-1,
                            dummyIL, 0, dummySL,dummySL,dummySL,dummySL,0,0 );
      // no group
      int key = 0;
      theGroups[ key ].push_back( group );

      // 1D algo
      key = 11;
      //        0: Basic
      group.Label = "GROUP:" + QObject::tr( "SMESH_1D_ALGO_GROUP_BASIC" );
      theGroups[ key ].push_back( group );
      //        1: Advanced
      group.Label = "GROUP:" + QObject::tr( "SMESH_1D_ALGO_GROUP_ADVANCED" );
      theGroups[ key ].push_back( group );

      // 1D hypotheses
      key = 01;
      //        0: Basic
      group.Label = "GROUP:" + QObject::tr( "SMESH_1D_HYP_GROUP_BASIC" );
      theGroups[ key ].push_back( group );
      //        1: Progression
      group.Label = "GROUP:" + QObject::tr( "SMESH_1D_HYP_GROUP_PROGRESSION" );
      theGroups[ key ].push_back( group );
      //        2: Advanced
      group.Label = "GROUP:" + QObject::tr( "SMESH_1D_HYP_GROUP_ADVANCED" );
      theGroups[ key ].push_back( group );

      // 2D algo
      key = 12;
      //        0: Regular
      group.Label = "GROUP:" + QObject::tr( "SMESH_2D_ALGO_GROUP_REGULAR" );
      theGroups[ key ].push_back( group );
      //        1: Free
      group.Label = "GROUP:" + QObject::tr( "SMESH_2D_ALGO_GROUP_FREE" );
      theGroups[ key ].push_back( group );
      //        2: Advanced
      group.Label = "GROUP:" + QObject::tr( "SMESH_2D_ALGO_GROUP_ADVANCED" );
      theGroups[ key ].push_back( group );

      // 3D algo
      key = 13;
      //        0: Regular
      group.Label = "GROUP:" + QObject::tr( "SMESH_3D_ALGO_GROUP_REGULAR" );
      theGroups[ key ].push_back( group );
      //        1: Free
      group.Label = "GROUP:" + QObject::tr( "SMESH_3D_ALGO_GROUP_FREE" );
      theGroups[ key ].push_back( group );
      //        2: Advanced
      group.Label = "GROUP:" + QObject::tr( "SMESH_3D_ALGO_GROUP_ADVANCED" );
      theGroups[ key ].push_back( group );
    }

    size_t key = 0, groupID = 0;
    if ( hyp && !hyp->Dim.isEmpty() )
    {
      key     = hyp->Dim[0] + isAlgo * 10;
      groupID = hyp->GroupID;
    }

    if ( key < theGroups.size() && !theGroups[ key ].empty() )
    {
      std::vector< HypothesisData > & group = theGroups[ key ];
      return & ( groupID < group.size() ? group[ groupID ] : group.back() );
    }
    return & theGroups[ 0 ][ 0 ];
  }

  bool IsAvailableHypothesis(const HypothesisData* algoData,
                             const QString&        hypType,
                             bool&                 isAuxiliary)
  {
    isAuxiliary = false;
    if ( !algoData )
      return false;
    if ( algoData->BasicHypos.contains( hypType ))
      return true;
    if ( algoData->OptionalHypos.contains( hypType )) {
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

  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator(const QString& aHypType)
  {
    if(MYDEBUG) MESSAGE("Get HypothesisCreator for " << aHypType.toLatin1().data());

    SMESHGUI_GenericHypothesisCreator* aCreator = 0;

    // check, if creator for this hypothesis type already exists
    // BUG 0020378
    //if (myHypCreatorMap.find(aHypType) != myHypCreatorMap.end()) {
    //  aCreator = myHypCreatorMap[aHypType];
    //}
    //else
    {
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
        LibHandle libHandle = LoadLib( aClientLibName.toLatin1().data() );
        if (!libHandle) {
          // report any error, if occured
          {
#ifdef WIN32
            const char* anError = "Can't load client meshers plugin library";
#else
            const char* anError = dlerror();
#endif
            INFOS(anError); // always display this kind of error !
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
            if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << aHypType.toLatin1().data());
            aCreator = procHandle( aHypType );
            if (!aCreator) {
              if(MYDEBUG) MESSAGE("no such a hypothesis in this plugin");
            }
            else {
              // map hypothesis creator to a hypothesis name
              // BUG 0020378
              //myHypCreatorMap[aHypType] = aCreator;

              //rnv : This dynamic property of the QObject stores the name of the plugin.
              //      It is used to obtain plugin root dir environment variable
              //      in the SMESHGUI_HypothesisDlg class. Plugin root dir environment
              //      variable is used to display documentation.
              aCreator->setProperty(SMESH::Plugin_Name(),aHypData->PluginName);
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


  SMESH::SMESH_Hypothesis_ptr CreateHypothesis(const QString& aHypType,
                                               const QString& aHypName,
                                               const bool isAlgo)
  {
    if(MYDEBUG) MESSAGE("Create " << aHypType.toLatin1().data() <<
                        " with name " << aHypName.toLatin1().data());
    HypothesisData* aHypData = GetHypothesisData(aHypType);
    QString aServLib = aHypData->ServerLibName;
    try {
      SMESH::SMESH_Hypothesis_var aHypothesis;
      aHypothesis = SMESHGUI::GetSMESHGen()->CreateHypothesis(aHypType.toLatin1().data(),
                                                              aServLib.toLatin1().data());
      if (!aHypothesis->_is_nil()) {
        _PTR(SObject) aHypSObject = SMESH::FindSObject(aHypothesis.in());
        if (aHypSObject) {
          if (!aHypName.isEmpty())
            SMESH::SetName(aHypSObject, aHypName);
          SMESHGUI::Modified();
          SMESHGUI::GetSMESHGUI()->updateObjBrowser();
          return aHypothesis._retn();
        }
      }
    } catch (const SALOME::SALOME_Exception & S_ex) {
      SalomeApp_Tools::QtCatchCorbaException(S_ex);
    }

    return SMESH::SMESH_Hypothesis::_nil();
  }

  bool IsApplicable(const QString&        aHypType,
                    GEOM::GEOM_Object_ptr theGeomObject,
                    const bool            toCheckAll)
  {
    if ( getenv("NO_LIMIT_ALGO_BY_SHAPE")) // allow a workaround for a case if
      return true;                         // IsApplicable() returns false due to a bug

    HypothesisData* aHypData = GetHypothesisData(aHypType);
    QString aServLib = aHypData->ServerLibName;
    return SMESHGUI::GetSMESHGen()->IsApplicable( aHypType.toLatin1().data(),
                                                  aServLib.toLatin1().data(),
                                                  theGeomObject,
                                                  toCheckAll);
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
        CORBA::String_var error;
        res = aMesh->AddHypothesis(aShapeObject, aHyp, error.out());
        if (res < SMESH::HYP_UNKNOWN_FATAL) {
          _PTR(SObject) aSH = SMESH::FindSObject(aHyp);
          if (SM && aSH) {
            SMESH::ModifiedMesh(SM, false, aMesh->NbNodes()==0);
          }
        }
        if (res > SMESH::HYP_OK) {
          wc.suspend();
          processHypothesisStatus(res, aHyp, true, error.in() );
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
        if (!aMesh->_is_nil() && SsubM && !aShapeObject->_is_nil())
        {
          CORBA::String_var error;
          res = aMesh->AddHypothesis( aShapeObject, aHyp, error.out() );
          if (res < SMESH::HYP_UNKNOWN_FATAL)  {
            _PTR(SObject) meshSO = SMESH::FindSObject(aMesh);
            if (meshSO)
              SMESH::ModifiedMesh(meshSO, false, aMesh->NbNodes()==0);
          }
          if (res > SMESH::HYP_OK) {
            wc.suspend();
            processHypothesisStatus( res, aHyp, true, error.in() );
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
          for( size_t i = 0; i < meshList.size(); i++ )
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

  bool RemoveHypothesisOrAlgorithmOnMesh (_PTR(SObject)               MorSM,
                                          SMESH::SMESH_Hypothesis_ptr anHyp)
  {
    int res = SMESH::HYP_UNKNOWN_FATAL;
    SUIT_OverrideCursor wc;

    if (MorSM) {
      try {
        GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh(MorSM);
        SMESH::SMESH_Mesh_var aMesh        = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(MorSM);
        SMESH::SMESH_subMesh_var aSubMesh  = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(MorSM);

        if (!aSubMesh->_is_nil())
          aMesh = aSubMesh->GetFather();

        if (!aMesh->_is_nil()) {
          if (aMesh->HasShapeToMesh() && !aShapeObject->_is_nil()) {
            res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
          }
          else if(!aMesh->HasShapeToMesh()){
            res = aMesh->RemoveHypothesis(aShapeObject, anHyp);
          }
          if (res > SMESH::HYP_OK) {
            wc.suspend();
            processHypothesisStatus(res, anHyp, false);
            wc.resume();
          }
          if ( _PTR(SObject) meshSO = SMESH::FindSObject(aMesh) )
          {
            if ( res < SMESH::HYP_UNKNOWN_FATAL )
              SMESH::ModifiedMesh(meshSO, false, aMesh->NbNodes()==0);

            if ( SMESH_Actor* actor = SMESH::FindActorByEntry( meshSO->GetID().c_str() ))
              if( actor->GetVisibility() )
                actor->Update();
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
    for ( size_t i = 0; i < errors.length(); ++i ) {
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
} // end of namespace SMESH
