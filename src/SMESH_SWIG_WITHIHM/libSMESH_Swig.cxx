// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// SMESH SMESH : GUI for SMESH component
// File   : libSMESH_Swig.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "libSMESH_Swig.h"

#include <SMESHGUI.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_Displayer.h>

// SALOME KERNEL includes
#include <Utils_ORB_INIT.hxx>
#include <Utils_SINGLETON.hxx>
#include <SALOMEDSClient_ClientFactory.hxx>

#include <utilities.h>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <VTKViewer_ViewModel.h>
#include <SALOME_Event.h>
#include <SalomeApp_Application.h>

// OCCT includes
#include <TopAbs.hxx>

// Qt includes
#include <QApplication>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

static CORBA::ORB_var anORB;

namespace
{
  //---------------------------------------------------------------
  inline
  CORBA::Object_var
  StringToObject(const std::string& theIOR)
  {
    return anORB->string_to_object(theIOR.c_str());
  }


  //---------------------------------------------------------------
  inline
  SALOMEDS::SObject_var
  GetDomainRoot(const SALOMEDS::SComponent_var& theSComponentMesh,
                const SALOMEDS::StudyBuilder_var& theStudyBuilder,
                CORBA::Long theDomainRootTag,
                const QString& theName,
                const QString& thePixmap)
  {
    SALOMEDS::SObject_var aDomainRoot;
    if (!theSComponentMesh->FindSubObject(theDomainRootTag,aDomainRoot)) {
      aDomainRoot = theStudyBuilder->NewObjectToTag(theSComponentMesh,theDomainRootTag);
      SALOMEDS::GenericAttribute_var anAttr = theStudyBuilder->FindOrCreateAttribute(aDomainRoot,"AttributeName");
      SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(anAttr);
      aName->SetValue(theName.toLatin1().data());
      anAttr = theStudyBuilder->FindOrCreateAttribute(aDomainRoot,"AttributePixMap");
      SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
      aPixmap->SetPixMap(thePixmap.toLatin1().data());
      anAttr = theStudyBuilder->FindOrCreateAttribute(aDomainRoot,"AttributeSelectable");
      SALOMEDS::AttributeSelectable_var aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
      aSelAttr->SetSelectable(false);
    }

    return aDomainRoot;
  }


  //---------------------------------------------------------------
  inline
  SALOMEDS::SObject_var
  GetHypothesisRoot(const SALOMEDS::SComponent_var& theSComponentMesh,
                    const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    return GetDomainRoot(theSComponentMesh,
                         theStudyBuilder,
                         SMESH::Tag_HypothesisRoot,
                         QObject::tr("SMESH_MEN_HYPOTHESIS"),
                         "ICON_SMESH_TREE_HYPO");
  }


  //---------------------------------------------------------------
  inline
  SALOMEDS::SObject_var
  GetAlgorithmsRoot(const SALOMEDS::SComponent_var& theSComponentMesh,
                    const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    return GetDomainRoot(theSComponentMesh,
                         theStudyBuilder,
                         SMESH::Tag_AlgorithmsRoot,
                         QObject::tr("SMESH_MEN_ALGORITHMS"),
                         "ICON_SMESH_TREE_ALGO");
  }


  //---------------------------------------------------------------
  inline
  SALOMEDS::SObject_var
  AddToDomain(const std::string& theIOR,
              const SALOMEDS::SComponent_var& theSComponentMesh,
              const SALOMEDS::StudyBuilder_var& theStudyBuilder,
              CORBA::Long theDomainRootTag,
              const QString& theDomainName,
              const QString& theDomainPixmap)
  {
    SALOMEDS::SObject_var aDomain = GetDomainRoot(theSComponentMesh,
                                                  theStudyBuilder,
                                                  SMESH::Tag_AlgorithmsRoot,
                                                  theDomainName,
                                                  theDomainPixmap);
    // Add New Hypothesis
    SALOMEDS::SObject_var aSObject = theStudyBuilder->NewObject(aDomain);
    SALOMEDS::GenericAttribute_var anAttr = theStudyBuilder->FindOrCreateAttribute(aSObject,"AttributePixMap");
    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    CORBA::Object_var anObject = StringToObject(theIOR);
    SMESH::SMESH_Hypothesis_var aDomainItem = SMESH::SMESH_Hypothesis::_narrow(anObject.in());
    CORBA::String_var aType = aDomainItem->GetName();
    QString aPixmapName = theDomainPixmap + "_" + aType.in();
    aPixmap->SetPixMap(aPixmapName.toLatin1().data());
    anAttr = theStudyBuilder->FindOrCreateAttribute(aSObject,"AttributeIOR");
    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR->SetValue(theIOR.c_str());

    return aSObject;
  }


  //---------------------------------------------------------------
  SALOMEDS::SObject_var
  AddHypothesis(const std::string& theIOR,
                const SALOMEDS::SComponent_var& theSComponentMesh,
                const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    return AddToDomain(theIOR,
                       theSComponentMesh,
                       theStudyBuilder,
                       SMESH::Tag_HypothesisRoot,
                       QObject::tr("SMESH_MEN_HYPOTHESIS"),
                       "ICON_SMESH_TREE_HYPO");
  }


  //---------------------------------------------------------------
  SALOMEDS::SObject_var
  AddAlgorithms(const std::string& theIOR,
                const SALOMEDS::SComponent_var& theSComponentMesh,
                const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    return AddToDomain(theIOR,
                       theSComponentMesh,
                       theStudyBuilder,
                       SMESH::Tag_AlgorithmsRoot,
                       QObject::tr("SMESH_MEN_ALGORITHMS"),
                       "ICON_SMESH_TREE_ALGO");
  }


  //---------------------------------------------------------------
  void
  SetDomain(const char* theMeshOrSubMeshEntry,
            const char* theDomainEntry,
            const SALOMEDS::Study_var& theStudy,
            const SALOMEDS::StudyBuilder_var& theStudyBuilder,
            long theRefOnAppliedDomainTag,
            const QString& theAppliedDomainMEN,
            const QString& theAppliedDomainICON)
  {
    SALOMEDS::SObject_var aMeshOrSubMeshSO = theStudy->FindObjectID(theMeshOrSubMeshEntry);
    SALOMEDS::SObject_var aHypothesisSO = theStudy->FindObjectID(theDomainEntry);

    if(!aMeshOrSubMeshSO->_is_nil() && !aHypothesisSO->_is_nil()){
      //Find or Create Applied Hypothesis root
      SALOMEDS::SObject_var anAppliedDomainSO;
      if(!aMeshOrSubMeshSO->FindSubObject(theRefOnAppliedDomainTag,anAppliedDomainSO)){
        anAppliedDomainSO = theStudyBuilder->NewObjectToTag(aMeshOrSubMeshSO,theRefOnAppliedDomainTag);
        SALOMEDS::GenericAttribute_var anAttr =
          theStudyBuilder->FindOrCreateAttribute(anAppliedDomainSO,"AttributeName");
        SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(anAttr);
        aName->SetValue(theAppliedDomainMEN.toLatin1().data());
        anAttr = theStudyBuilder->FindOrCreateAttribute(anAppliedDomainSO,"AttributeSelectable");
        SALOMEDS::AttributeSelectable_var aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
        aSelAttr->SetSelectable(false);
        anAttr = theStudyBuilder->FindOrCreateAttribute(anAppliedDomainSO,"AttributePixMap");
        SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
        aPixmap->SetPixMap(theAppliedDomainICON.toLatin1().data());
      }
      SALOMEDS::SObject_var aSObject = theStudyBuilder->NewObject(anAppliedDomainSO);
      theStudyBuilder->Addreference(aSObject,aHypothesisSO);
    }
  }


  //---------------------------------------------------------------
  void
  SetHypothesis(const char* theMeshOrSubMeshEntry,
                const char* theDomainEntry,
                const SALOMEDS::Study_var& theStudy,
                const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    SetDomain(theMeshOrSubMeshEntry,
              theDomainEntry,
              theStudy,
              theStudyBuilder,
              SMESH::Tag_RefOnAppliedHypothesis,
              QObject::tr("SMESH_MEN_APPLIED_HYPOTHESIS"),
              "ICON_SMESH_TREE_HYPO");
  }


  //---------------------------------------------------------------
  void
  SetAlgorithms(const char* theMeshOrSubMeshEntry,
                const char* theDomainEntry,
                const SALOMEDS::Study_var& theStudy,
                const SALOMEDS::StudyBuilder_var& theStudyBuilder)
  {
    SetDomain(theMeshOrSubMeshEntry,
              theDomainEntry,
              theStudy,
              theStudyBuilder,
              SMESH::Tag_RefOnAppliedAlgorithms,
              QObject::tr("SMESH_MEN_APPLIED_ALGORIHTMS"),
              "ICON_SMESH_TREE_ALGO");
  }
}


//===============================================================
SMESH_Swig::SMESH_Swig()
{
  class TEvent: public SALOME_Event
  {
    CORBA::ORB_var& myORB;
  public:

    TEvent(CORBA::ORB_var& theORB):
      myORB(theORB)
    {}

    virtual
    void
    Execute()
    {
      try {
        ORB_INIT &anORBInit = *SINGLETON_<ORB_INIT>::Instance();
        ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting());
        myORB = anORBInit( 0, 0 );
      } catch (...) {
        INFOS("internal error : orb not found");
      }
    }
  };

  MESSAGE("Constructeur");

  if(CORBA::is_nil(anORB))
    ProcessVoidEvent(new TEvent(anORB));

  ASSERT(!CORBA::is_nil(anORB));
}


//===============================================================
void
SMESH_Swig::Init(int theStudyID)
{
  class TEvent: public SALOME_Event
  {
    int myStudyID;
    SALOMEDS::Study_var& myStudy;
    SALOMEDS::StudyBuilder_var& myStudyBuilder;
    SALOMEDS::SComponent_var& mySComponentMesh;
  public:
    TEvent(int theStudyID,
           SALOMEDS::Study_var& theStudy,
           SALOMEDS::StudyBuilder_var& theStudyBuilder,
           SALOMEDS::SComponent_var& theSComponentMesh):
      myStudyID(theStudyID),
      myStudy(theStudy),
      myStudyBuilder(theStudyBuilder),
      mySComponentMesh(theSComponentMesh)
    {}

    virtual
    void
    Execute()
    {
      SUIT_Session* aSession = SUIT_Session::session();
      SUIT_Application* anApplication = aSession->activeApplication();
      SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>(anApplication);

      SALOME_NamingService* aNamingService = anApp->namingService();
      CORBA::Object_var anObject = aNamingService->Resolve("/myStudyManager");
      SALOMEDS::StudyManager_var aStudyMgr = SALOMEDS::StudyManager::_narrow(anObject);
      myStudy = aStudyMgr->GetStudyByID(myStudyID);

      SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
      aSMESHGen->SetCurrentStudy( myStudy.in() );

      myStudyBuilder = myStudy->NewBuilder();

      SALOMEDS::GenericAttribute_var anAttr;
      SALOMEDS::AttributeName_var    aName;
      SALOMEDS::AttributePixMap_var  aPixmap;

      SALOMEDS::SComponent_var aSComponent = myStudy->FindComponent("SMESH");
      if(aSComponent->_is_nil()){
        bool aLocked = myStudy->GetProperties()->IsLocked();
        if (aLocked)
          myStudy->GetProperties()->SetLocked(false);
        
        aSComponent = myStudyBuilder->NewComponent("SMESH");
        anAttr = myStudyBuilder->FindOrCreateAttribute(aSComponent,"AttributeName");
        aName = SALOMEDS::AttributeName::_narrow(anAttr);

        SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI(); //SRN: BugID IPAL9186, load a SMESH gui if it hasn't been loaded
        if (!aSMESHGUI){
          CAM_Module* aModule = anApp->module("Mesh");
          if(!aModule)
              aModule = anApp->loadModule("Mesh");
          aSMESHGUI = dynamic_cast<SMESHGUI*>(aModule);
        } //SRN: BugID IPAL9186: end of a fix
        aName->SetValue(aSMESHGUI->moduleName().toLatin1().data());
        anAttr = myStudyBuilder->FindOrCreateAttribute(aSComponent,"AttributePixMap");
        aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
        aPixmap->SetPixMap( "ICON_OBJBROWSER_SMESH" );
        myStudyBuilder->DefineComponentInstance(aSComponent,aSMESHGen);
        if (aLocked)
          myStudy->GetProperties()->SetLocked(true);
      }

      mySComponentMesh = SALOMEDS::SComponent::_narrow(aSComponent);

      qApp->processEvents(); // Workaround for bug 12662
    }
  };

  MESSAGE("Init");

  ProcessVoidEvent(new TEvent(theStudyID,
                              myStudy,
                              myStudyBuilder,
                              mySComponentMesh));
}


//===============================================================
SMESH_Swig::~SMESH_Swig()
{
  MESSAGE("Destructeur");
}


//===============================================================
const char* SMESH_Swig::AddNewMesh(const char* theIOR)
{
  MESSAGE("AddNewMesh");

  // VSR: added temporarily - to be removed - objects are published automatically by engine
  SALOMEDS::SObject_var aSObject = myStudy->FindObjectIOR(theIOR);
  if (aSObject->_is_nil()){
    //Find or Create Hypothesis root
    GetHypothesisRoot(mySComponentMesh,myStudyBuilder);
    GetAlgorithmsRoot(mySComponentMesh,myStudyBuilder);

    // Add New Mesh
    aSObject = myStudyBuilder->NewObject(mySComponentMesh);
    SALOMEDS::GenericAttribute_var anAttr = myStudyBuilder->FindOrCreateAttribute(aSObject,"AttributePixMap");
    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
    anAttr = myStudyBuilder->FindOrCreateAttribute(aSObject, "AttributeIOR");
    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR->SetValue(theIOR);
  }

  CORBA::String_var anEntry = aSObject->GetID();

  return anEntry._retn();
}


//===============================================================
const char* SMESH_Swig::AddNewHypothesis(const char* theIOR)
{
  MESSAGE("AddNewHypothesis");

  SALOMEDS::SObject_var aSObject = ::AddHypothesis(theIOR,
                                                   mySComponentMesh,
                                                   myStudyBuilder);
  CORBA::String_var anEntry = aSObject->GetID();
  return anEntry._retn();
}


//===============================================================
const char* SMESH_Swig::AddNewAlgorithms(const char* theIOR)
{
  MESSAGE("AddNewAlgorithms");

  SALOMEDS::SObject_var aSObject = ::AddAlgorithms(theIOR,
                                                   mySComponentMesh,
                                                   myStudyBuilder);
  CORBA::String_var anEntry = aSObject->GetID();
  return anEntry._retn();
}


//===============================================================
void SMESH_Swig::SetShape(const char* theShapeEntry,
                          const char* theMeshEntry)
{
  SALOMEDS::SObject_var aMeshSO = myStudy->FindObjectID( theMeshEntry );
  SALOMEDS::SObject_var aGeomShapeSO = myStudy->FindObjectID( theShapeEntry );

  if(!aMeshSO->_is_nil() && !aGeomShapeSO->_is_nil()){
    SALOMEDS::SObject_var aSObject = myStudyBuilder->NewObjectToTag(aMeshSO, SMESH::Tag_RefOnShape);
    myStudyBuilder->Addreference(aSObject,aGeomShapeSO);
  }
}


//===============================================================
void SMESH_Swig::SetHypothesis(const char* theMeshOrSubMeshEntry,
                               const char* theDomainEntry)
{
  ::SetHypothesis(theMeshOrSubMeshEntry,
                  theDomainEntry,
                  myStudy,
                  myStudyBuilder);
}


//===============================================================
void SMESH_Swig::SetAlgorithms(const char* theMeshOrSubMeshEntry,
                               const char* theDomainEntry)
{
  ::SetAlgorithms(theMeshOrSubMeshEntry,
                  theDomainEntry,
                  myStudy,
                  myStudyBuilder);
}


//===============================================================
void
SMESH_Swig::UnSetHypothesis(const char* theDomainEntry)
{
  SALOMEDS::SObject_var aDomainSO = myStudy->FindObjectID(theDomainEntry);
  if(!aDomainSO->_is_nil())
    myStudyBuilder->RemoveObject(aDomainSO);
}

const char* SMESH_Swig::AddSubMesh(const char* theMeshEntry,
                                   const char* theSubMeshIOR,
                                   int theShapeType)
{
  SALOMEDS::SObject_var aMeshSO = myStudy->FindObjectID(theMeshEntry);
  if(!aMeshSO->_is_nil()){
    long aShapeTag;
    QString aSubMeshName;
    switch(theShapeType){
    case TopAbs_SOLID:
      aShapeTag = SMESH::Tag_SubMeshOnSolid;
      aSubMeshName = QObject::tr("SMESH_MEN_SubMeshesOnSolid");
      break;
    case TopAbs_FACE:
      aShapeTag = SMESH::Tag_SubMeshOnFace;
      aSubMeshName = QObject::tr("SMESH_MEN_SubMeshesOnFace");
      break;
    case TopAbs_EDGE:
      aShapeTag = SMESH::Tag_SubMeshOnEdge;
      aSubMeshName = QObject::tr("SMESH_MEN_SubMeshesOnEdge");
      break;
    case TopAbs_VERTEX:
      aShapeTag = SMESH::Tag_SubMeshOnVertex;
      aSubMeshName = QObject::tr("SMESH_MEN_SubMeshesOnVertex");
      break;
    default:
      aShapeTag = SMESH::Tag_SubMeshOnCompound;
      aSubMeshName = QObject::tr("SMESH_MEN_SubMeshesOnCompound");
    }

    SALOMEDS::SObject_var aSubMeshesRoot;
    SALOMEDS::GenericAttribute_var anAttr;
    if(!aMeshSO->FindSubObject(aShapeTag,aSubMeshesRoot)){
      aSubMeshesRoot = myStudyBuilder->NewObjectToTag(aMeshSO,aShapeTag);
      anAttr = myStudyBuilder->FindOrCreateAttribute(aSubMeshesRoot,"AttributeName");
      SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(anAttr);
      aName->SetValue(aSubMeshName.toLatin1().data());
      anAttr = myStudyBuilder->FindOrCreateAttribute(aSubMeshesRoot,"AttributeSelectable");
      SALOMEDS::AttributeSelectable_var aSelAttr = SALOMEDS::AttributeSelectable::_narrow(anAttr);
      aSelAttr->SetSelectable(false);
    }

    SALOMEDS::SObject_var aSObject = myStudyBuilder->NewObject(aSubMeshesRoot);
    anAttr = myStudyBuilder->FindOrCreateAttribute(aSObject,"AttributeIOR");
    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR->SetValue(theSubMeshIOR);

    CORBA::String_var aString = aSObject->GetID();
    return aString._retn();
  }

  return "";
}

const char* SMESH_Swig::AddSubMeshOnShape(const char* theMeshEntry,
                                          const char* theGeomShapeEntry,
                                          const char* theSubMeshIOR,
                                          int ShapeType)
{
  SALOMEDS::SObject_var aGeomShapeSO = myStudy->FindObjectID(theGeomShapeEntry);
  if(!aGeomShapeSO->_is_nil()){
    const char * aSubMeshEntry = AddSubMesh(theMeshEntry,theSubMeshIOR,ShapeType);
    SALOMEDS::SObject_var aSubMeshSO = myStudy->FindObjectID(aSubMeshEntry);
    if(!aSubMeshSO->_is_nil()){
      SetShape(theGeomShapeEntry,aSubMeshEntry);
      CORBA::String_var aString = aSubMeshSO->GetID();
      return aString._retn();
    }
  }

  return "";
}

void SMESH_Swig::CreateAndDisplayActor( const char* Mesh_Entry )
{
  //  SMESH_Actor* Mesh = smeshGUI->ReadScript(aM);
  class TEvent: public SALOME_Event
  {
  private:
    const char* _entry;
  public:
    TEvent(const char* Mesh_Entry) {
      _entry = Mesh_Entry;
    }
    virtual void Execute() {
      //SMESH::UpdateView(SMESH::eDisplay, _entry);
      SUIT_Session* aSession = SUIT_Session::session();
      SUIT_Application* anApplication = aSession->activeApplication();
      SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>(anApplication);
      /*SUIT_ViewManager* vman = */anApp->getViewManager(VTKViewer_Viewer::Type(),true);
      SMESHGUI_Displayer* aDisp = new SMESHGUI_Displayer(anApp);
      aDisp->Display(_entry,1);
    }
  };

  ProcessVoidEvent(new TEvent(Mesh_Entry));
}

void SMESH_Swig::SetName(const char* theEntry,
                         const char* theName)
{
  SALOMEDS::SObject_var aSObject = myStudy->FindObjectID(theEntry);
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var aName;
  if(!aSObject->_is_nil()){
    anAttr = myStudyBuilder->FindOrCreateAttribute(aSObject,"AttributeName");
    aName = SALOMEDS::AttributeName::_narrow(anAttr);
    aName->SetValue(theName);
  }
}

//================================================================================
/*!
 * \brief Set mesh icon according to compute status
  * \param Mesh_Entry - entry of a mesh
  * \param isComputed - is mesh computed or not
 */
//================================================================================

void SMESH_Swig::SetMeshIcon(const char* theMeshEntry,
                             const bool theIsComputed,
                             const bool isEmpty)
{
  class TEvent: public SALOME_Event
  {
    SALOMEDS::Study_var myStudy;
    std::string myMeshEntry;
    bool myIsComputed, myIsEmpty;
  public:
    TEvent(const SALOMEDS::Study_var& theStudy,
           const std::string& theMeshEntry,
           const bool theIsComputed,
           const bool isEmpty):
      myStudy(theStudy),
      myMeshEntry(theMeshEntry),
      myIsComputed(theIsComputed),
      myIsEmpty(isEmpty)
    {}

    virtual
    void
    Execute()
    {
      SALOMEDS::SObject_var aMeshSO = myStudy->FindObjectID(myMeshEntry.c_str());
      if(!aMeshSO->_is_nil())
        if(_PTR(SObject) aMesh = ClientFactory::SObject(aMeshSO))
          SMESH::ModifiedMesh(aMesh,myIsComputed,myIsEmpty);
    }
  };

  ProcessVoidEvent(new TEvent(myStudy,
                              theMeshEntry,
                              theIsComputed,
                              isEmpty));
}
