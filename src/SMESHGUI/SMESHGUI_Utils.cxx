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

#include "QAD_Desktop.h"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

#include "utilities.h"

using namespace std;

namespace SMESH{

  QAD_Study* GetActiveStudy(){
    if(QAD_Desktop* aDesktop = QAD_Application::getDesktop())
      return aDesktop->getActiveStudy();
    return NULL;
  }


  QAD_StudyFrame* GetActiveStudyFrame(){
    if(QAD_Study* aStudy = GetActiveStudy())
      return aStudy->getActiveStudyFrame();
    return NULL;
  }


  SALOMEDS::Study_var GetActiveStudyDocument(){
    if(QAD_Study* aStudy = GetActiveStudy())
      return aStudy->getStudyDocument();
    return SALOMEDS::Study::_nil();
  }


  SALOMEDS::SObject_var FindSObject(CORBA::Object_ptr theObject){
    if(!CORBA::is_nil(theObject)){
      SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
      CORBA::String_var anIOR = aStudy->ConvertObjectToIOR(theObject);
      if(strcmp(anIOR.in(),"") != 0)
	return aStudy->FindObjectIOR(anIOR);
    }
    return SALOMEDS::SObject::_nil();
  }


  void SetName(SALOMEDS::SObject_ptr theSObject, const char* theName){
    using namespace SALOMEDS;
    Study_var aStudy = GetActiveStudyDocument();
    if(aStudy->GetProperties()->IsLocked())
      return;
    StudyBuilder_var aBuilder = aStudy->NewBuilder();
    GenericAttribute_var anAttr = aBuilder->FindOrCreateAttribute(theSObject,"AttributeName");
    AttributeName_var aName = AttributeName::_narrow(anAttr);
    if(!aName->_is_nil())
      aName->SetValue(theName);
  }

  void SetValue(SALOMEDS::SObject_ptr theSObject, const char* theValue){
    using namespace SALOMEDS;
    Study_var aStudy = GetActiveStudyDocument();
    if(aStudy->GetProperties()->IsLocked())
      return;
    StudyBuilder_var aBuilder = aStudy->NewBuilder();
    GenericAttribute_var anAttr = aBuilder->FindOrCreateAttribute(theSObject,"AttributeComment");
    AttributeComment_var aComment = AttributeComment::_narrow(anAttr);
    if(!aComment->_is_nil())
      aComment->SetValue(theValue);
  }
  

  CORBA::Object_var SObjectToObject(SALOMEDS::SObject_ptr theSObject,
				    SALOMEDS::Study_ptr theStudy)
  {
    if(!theSObject->_is_nil()){
      SALOMEDS::GenericAttribute_var anAttr;
      if(theSObject->FindAttribute(anAttr, "AttributeIOR")){
	SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	CORBA::String_var aVal = anIOR->Value();
	return theStudy->ConvertIORToObject(aVal.in());
      }
    }
    return CORBA::Object::_nil();
  }


  CORBA::Object_var SObjectToObject(SALOMEDS::SObject_ptr theSObject){
    SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
    return SObjectToObject(theSObject,aStudy);
  }


  CORBA::Object_var IObjectToObject(const Handle(SALOME_InteractiveObject)& theIO){
    if(!theIO.IsNull()){
      if(theIO->hasEntry()){
	SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
	SALOMEDS::SObject_var anObj = aStudy->FindObjectID(theIO->getEntry());
	return SObjectToObject(anObj,aStudy);
      }
    }
    return CORBA::Object::_nil();
  }


  CORBA::Object_var IORToObject(const char* theIOR){
    SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
    return aStudy->ConvertIORToObject(theIOR);
  }


  int GetNameOfSelectedIObjects(SALOME_Selection* theSel, QString& theName)
  {
    int aNbSel = theSel->IObjectCount();
    if (aNbSel == 1) {
      Handle(SALOME_InteractiveObject) anIObject = theSel->firstIObject();
      theName = anIObject->getName();
    }else{
      theName = QObject::tr("SMESH_OBJECTS_SELECTED").arg(aNbSel);
    }
    return aNbSel;
  }


  SALOMEDS::SObject_var GetMeshOrSubmesh(SALOMEDS::SObject_ptr theSObject){
    GEOM::GEOM_Object_var aShape = SObjectToInterface<GEOM::GEOM_Object>(theSObject);
    if(!aShape->_is_nil()){ //It s a shape
      return theSObject->GetFather();
    }
    SALOMEDS::SObject_var aSObject;
    if(theSObject->ReferencedObject(aSObject)){
      aSObject = theSObject->GetFather();
      return aSObject->GetFather();
    }
    return theSObject->GetFather();
  }


  void ModifiedMesh(SALOMEDS::SObject_ptr theSObject, bool theIsRight)
  {
    SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
    if(aStudy->GetProperties()->IsLocked())
      return ;

    SALOMEDS::StudyBuilder_var aBuilder = aStudy->NewBuilder();
    SALOMEDS::GenericAttribute_var anAttr = 
      aBuilder->FindOrCreateAttribute(theSObject,"AttributePixMap");
    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    if(theIsRight){
      aPixmap->SetPixMap("ICON_SMESH_TREE_MESH");
    }else{
      aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_WARN");
    }
    
    SALOMEDS::ChildIterator_var anIter = aStudy->NewChildIterator(theSObject);
    for (int i = 1; anIter->More(); anIter->Next(), i++) {
      SALOMEDS::SObject_var aSObj = anIter->Value();
      if(i >= 4){
	SALOMEDS::ChildIterator_var anIter1 = aStudy->NewChildIterator(aSObj);
	for(; anIter1->More(); anIter1->Next()){
	  SALOMEDS::SObject_var aSObj1 = anIter1->Value();
	  anAttr = aBuilder->FindOrCreateAttribute(aSObj1,"AttributePixMap");
	  aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
	  if(theIsRight){
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH");
	  }else{
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_WARN");
	  }
	}
      }
    }
  }


}
