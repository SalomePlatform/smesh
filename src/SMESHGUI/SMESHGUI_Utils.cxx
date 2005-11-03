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


#include <qstring.h>

#include "SMESHGUI_Utils.h"

#include "OB_Browser.h"

#include "SUIT_Desktop.h"
#include "SUIT_Application.h"
#include "SUIT_Session.h"

#include "LightApp_SelectionMgr.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Module.h"
#include "SalomeApp_Study.h"

#include "SALOME_ListIO.hxx"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include "utilities.h"

//using namespace std;

namespace SMESH{

  SUIT_Desktop*
  GetDesktop(const CAM_Module* theModule)
  {
    return theModule->application()->desktop();
  }

  LightApp_SelectionMgr*
  GetSelectionMgr(const SalomeApp_Module* theModule)
  {
    return theModule->getApp()->selectionMgr();
  }

  SUIT_ResourceMgr*
  GetResourceMgr( const SalomeApp_Module* )
  {
    return SUIT_Session::session()->resourceMgr();
  }

  _PTR(Study)
  GetCStudy(const SalomeApp_Study* theStudy)
  {
    return theStudy->studyDS();
  }

  CORBA::Object_var 
  DataOwnerToObject(const LightApp_DataOwnerPtr& theOwner)
  {
    CORBA::Object_var anObj;
    if(theOwner){
      const Handle(SALOME_InteractiveObject)& anIO = theOwner->IO();
      if(!anIO.IsNull()){
	if(anIO->hasEntry()){
	  _PTR(Study) aStudy = GetActiveStudyDocument();
	  _PTR(SObject) aSObj = aStudy->FindObjectID(anIO->getEntry());
	  anObj = SObjectToObject(aSObj,aStudy);
	}
      }
    }
    return anObj;
  }


  SUIT_Study* GetActiveStudy()
  {
    SUIT_Application* app = SUIT_Session::session()->activeApplication();
    if (app)
      return app->activeStudy();
    else
      return NULL;
  }

  SUIT_ViewWindow* GetActiveWindow()
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    if (app && app->desktop() )
      return app->desktop()->activeWindow();
    else
      return NULL;
  }

  _PTR(Study) GetActiveStudyDocument()
  {
    SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>(GetActiveStudy());
    if (aStudy)
      return aStudy->studyDS();
    else
      return _PTR(Study)();
  }

  _PTR(SObject) FindSObject (CORBA::Object_ptr theObject)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    if (app && !CORBA::is_nil(theObject)) {
      if(_PTR(Study) aStudy = GetActiveStudyDocument()){
	CORBA::String_var anIOR = app->orb()->object_to_string(theObject);
	if (strcmp(anIOR.in(), "") != 0)
	  return aStudy->FindObjectIOR(anIOR.in());
      }
    }
    return _PTR(SObject)();
  }

  void SetName (_PTR(SObject) theSObject, const char* theName)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject, "AttributeName");
    _PTR(AttributeName) aName = anAttr;
    if (aName)
      aName->SetValue(theName);
  }

  void SetValue (_PTR(SObject) theSObject, const char* theValue)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject, "AttributeComment");
    _PTR(AttributeComment) aComment = anAttr;
    if (aComment)
      aComment->SetValue(theValue);
  }

  CORBA::Object_var SObjectToObject (_PTR(SObject) theSObject,
                                     _PTR(Study)   theStudy)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    if (theSObject) {
      _PTR(GenericAttribute) anAttr;
      if (theSObject->FindAttribute(anAttr, "AttributeIOR")) {
	_PTR(AttributeIOR) anIOR = anAttr;
	CORBA::String_var aVal = anIOR->Value().c_str();
	return app->orb()->string_to_object(aVal);
      }
    }
    return CORBA::Object::_nil();
  }

  CORBA::Object_var SObjectToObject (_PTR(SObject) theSObject)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    return SObjectToObject(theSObject,aStudy);
  }

  CORBA::Object_var IObjectToObject (const Handle(SALOME_InteractiveObject)& theIO)
  {
    if (!theIO.IsNull()) {
      if (theIO->hasEntry()) {
	_PTR(Study) aStudy = GetActiveStudyDocument();
	_PTR(SObject) anObj = aStudy->FindObjectID(theIO->getEntry());
	return SObjectToObject(anObj,aStudy);
      }
    }
    return CORBA::Object::_nil();
  }

  CORBA::Object_var IORToObject (const char* theIOR)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    return app->orb()->string_to_object(theIOR);
  }

  int GetNameOfSelectedIObjects(LightApp_SelectionMgr* theMgr, QString& theName)
  {
    if (!theMgr)
      return 0;

    SALOME_ListIO selected;
    theMgr->selectedObjects(selected);
    int aNbSel = selected.Extent();
    if (aNbSel == 1) {
      Handle(SALOME_InteractiveObject) anIObject = selected.First();
      theName = anIObject->getName();
    } else {
      theName = QObject::tr("SMESH_OBJECTS_SELECTED").arg(aNbSel);
    }
    return aNbSel;
  }

  _PTR(SObject) GetMeshOrSubmesh (_PTR(SObject) theSObject)
  {
    GEOM::GEOM_Object_var aShape = SObjectToInterface<GEOM::GEOM_Object>(theSObject);
    if (!aShape->_is_nil()){ //It s a shape
      return theSObject->GetFather();
    }
    _PTR(SObject) aSObject;
    if (theSObject->ReferencedObject(aSObject)) {
      aSObject = theSObject->GetFather();
      return aSObject->GetFather();
    }
    return theSObject->GetFather();
  }

  void ModifiedMesh (_PTR(SObject) theSObject, bool theIsRight)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;

    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject,"AttributePixMap");
    _PTR(AttributePixMap) aPixmap = anAttr;
    if (theIsRight) {
      aPixmap->SetPixMap("ICON_SMESH_TREE_MESH");
    } else {
      aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_WARN");
    }

    _PTR(ChildIterator) anIter = aStudy->NewChildIterator(theSObject);
    for (int i = 1; anIter->More(); anIter->Next(), i++) {
      _PTR(SObject) aSObj = anIter->Value();
      if (i >= 4) {
	_PTR(ChildIterator) anIter1 = aStudy->NewChildIterator(aSObj);
	for (; anIter1->More(); anIter1->Next()) {
	  _PTR(SObject) aSObj1 = anIter1->Value();
	  anAttr = aBuilder->FindOrCreateAttribute(aSObj1, "AttributePixMap");
	  aPixmap = anAttr;
	  if (theIsRight) {
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH");
	  } else {
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_WARN");
	  }
	}
      }
    }
  }

//  void UpdateObjBrowser (bool)
//  {
//    //SMESHGUI::activeStudy()->updateObjBrowser(true);
//    //SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>
//    //  (SUIT_Session::session()->activeApplication());
//    //if (anApp) anApp->objectBrowser()->updateTree();
//    SMESHGUI::GetSMESHGUI()->updateObjBrowser();
//  }
}
