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
// File   : SMESHGUI_Utils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Utils.h"
#include "SMESHGUI.h"
#include "SMESHGUI_Selection.h"
#include "SMESH_Type.h"
#include "SMESH_MeshAlgos.hxx"
#include <SMDS_MeshNode.hxx>
#include <SMDS_MeshFace.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Module.h>
#include <SalomeApp_Study.h>

#include <SALOME_ListIO.hxx>

// OCC includes
#include <gp_XYZ.hxx>
#include <TColgp_Array1OfXYZ.hxx>

#include CORBA_SERVER_HEADER(SMESH_Group)

//VSR: uncomment below macro to support unicode text properly in SALOME
//     current commented out due to regressions
//#define PAL22528_UNICODE

namespace SMESH
{
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

  void SetName (_PTR(SObject) theSObject, const QString& theName)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    SMESHGUI::GetSMESHGen()->SetName(theSObject->GetIOR().c_str(), theName.toLatin1().data());
  }

  void SetValue (_PTR(SObject) theSObject, const QString& theValue)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject, "AttributeComment");
    _PTR(AttributeComment) aComment = anAttr;
    if (aComment)
      aComment->SetValue(theValue.toLatin1().data());
  }
  
  void setFileName (_PTR(SObject) theSObject, const QString& theValue)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject, "AttributeExternalFileDef");
    _PTR(AttributeExternalFileDef) aFileName = anAttr;
    if (aFileName)
      aFileName->SetValue(theValue.toLatin1().data());
  }
  
  void setFileType (_PTR(SObject) theSObject, const QString& theValue)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject, "AttributeFileType");
    _PTR(AttributeFileType) aFileType = anAttr;
    if (aFileType)
      aFileType->SetValue(theValue.toLatin1().data());
  }

  CORBA::Object_var SObjectToObject (_PTR(SObject) theSObject,
                                     _PTR(Study)   /*theStudy*/)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    if (theSObject) {
      _PTR(GenericAttribute) anAttr;
      if (theSObject->FindAttribute(anAttr, "AttributeIOR")) {
        _PTR(AttributeIOR) anIOR = anAttr;
        std::string aVal = anIOR->Value();
        // string_to_object() DOC: If the input string is not valid ...
        // a CORBA::SystemException is thrown.
        if ( aVal.size() > 0 )
          return app->orb()->string_to_object( aVal.c_str() );
      }
    }
    return CORBA::Object::_nil();
  }

  CORBA::Object_var SObjectToObject (_PTR(SObject) theSObject)
  {
    _PTR(Study) aStudy;// = GetActiveStudyDocument(); -- aStudy is not used
    return SObjectToObject(theSObject,aStudy);
  }

  _PTR(SObject) ObjectToSObject( CORBA::Object_ptr theObject )
  {
    _PTR(SObject) res;
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    if ( app ) {
      CORBA::String_var ior = app->orb()->object_to_string( theObject );
      SalomeApp_Study* study = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
      if ( study && strlen( ior ) > 0 )
        res = study->studyDS()->FindObjectIOR( ior.in() );
    }
    return res;
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

  CORBA::Object_var IORToObject (const QString& theIOR)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      (SUIT_Session::session()->activeApplication());
    return app->orb()->string_to_object(theIOR.toLatin1().data());
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
      theName = QString( anIObject->getName() ).trimmed();
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

  void ModifiedMesh (_PTR(SObject) theSObject, bool theIsNotModif, bool isEmptyMesh)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    if (aStudy->GetProperties()->IsLocked())
      return;

    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr =
      aBuilder->FindOrCreateAttribute(theSObject,"AttributePixMap");
    _PTR(AttributePixMap) aPixmap = anAttr;

    std::string pmName;
    if (theIsNotModif)
      pmName = "ICON_SMESH_TREE_MESH";
    else if ( isEmptyMesh )
      pmName = "ICON_SMESH_TREE_MESH_WARN";
    else
      pmName = "ICON_SMESH_TREE_MESH_PARTIAL";
    aPixmap->SetPixMap( pmName );

    _PTR(ChildIterator) anIter = aStudy->NewChildIterator(theSObject);
    for ( ; anIter->More(); anIter->Next() ) {
      _PTR(SObject) aSObj = anIter->Value();
      if ( aSObj->Tag() >= SMESH::Tag_FirstSubMesh )
      {
        _PTR(ChildIterator) anIter1 = aStudy->NewChildIterator(aSObj);
        for ( ; anIter1->More(); anIter1->Next())
        {
          _PTR(SObject) aSObj1 = anIter1->Value();
          _PTR(SObject) aSObjectRef;
          if (aSObj1->ReferencedObject(aSObjectRef))
            continue; // reference to an object

          anAttr = aBuilder->FindOrCreateAttribute(aSObj1, "AttributePixMap");
          aPixmap = anAttr;

          std::string entry = aSObj1->GetID();
          int objType = SMESHGUI_Selection::type( entry.c_str(), aStudy );
          if ( objType == SMESH::HYPOTHESIS || objType == SMESH::ALGORITHM )
            continue;

          SMESH::SMESH_IDSource_var idSrc = SObjectToInterface<SMESH::SMESH_IDSource>( aSObj1 );
          if ( !idSrc->_is_nil() )
          {
            SMESH::SMESH_GroupOnFilter_var gof =
              SObjectToInterface<SMESH::SMESH_GroupOnFilter>( aSObj1 );
            const bool isGroupOnFilter = !gof->_is_nil();

            bool isEmpty = false;
            if ( !isGroupOnFilter ) // GetTypes() can be very long on GroupOnFilter!
            {
              SMESH::array_of_ElementType_var elemTypes = idSrc->GetTypes();
              isEmpty = ( elemTypes->length() == 0 );
            }
            if ( isEmpty )
              aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_WARN");
            else if ( objType != GROUP )
              aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
            else if ( isGroupOnFilter )
              aPixmap->SetPixMap( "ICON_SMESH_TREE_GROUP_ON_FILTER" );
            else
              aPixmap->SetPixMap( "ICON_SMESH_TREE_GROUP" );
          }
          else // is it necessary?
          {
            if ( !theIsNotModif )
              aPixmap->SetPixMap( pmName );
            else if ( objType == GROUP )
              aPixmap->SetPixMap( "ICON_SMESH_TREE_GROUP" );
            else
              aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH" );
          }
        }
      }
    }
  }

  void ShowHelpFile (const QString& theHelpFileName)
  {
    LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
    if (app) {
      SMESHGUI* gui = SMESHGUI::GetSMESHGUI();
      app->onHelpContextModule(gui ? app->moduleName(gui->moduleName()) : QString(""),
                               theHelpFileName);
    }
    else {
      SUIT_MessageBox::warning(0, QObject::tr("WRN_WARNING"),
                               QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                               arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                   "application")).
                               arg(theHelpFileName));
    }
  }

  //=======================================================================
  /**
     Return normale to a given face
  */
  //=======================================================================
  gp_XYZ getNormale( const SMDS_MeshFace* theFace )
  {
    gp_XYZ n;
    SMESH_MeshAlgos::FaceNormal( theFace, n, /*normalized=*/true );
    return n;
  }
  
  QString fromUtf8( const char* txt )
  {
#ifdef PAL22528_UNICODE
    return QString::fromUtf8( txt );
#else
    return QString( txt );
#endif
  }

  QString fromUtf8( const std::string& txt )
  {
    return fromUtf8( txt.c_str() );
  }

  toUtf8::toUtf8( const QString& txt )
  {
#ifdef PAL22528_UNICODE
    assign( txt.toUtf8().constData() );
#else
    assign( txt.toLatin1().constData() );
#endif
  }

} // end of namespace SMESH
