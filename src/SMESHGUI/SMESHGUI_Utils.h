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

#ifndef SMESHGUI_Utils_HeaderFile
#define SMESHGUI_Utils_HeaderFile

#include <CORBA.h>

#include "SALOMEDSClient_definitions.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "LightApp_DataOwner.h"

class QString;

class SUIT_ViewWindow;
class SUIT_Desktop;
class SUIT_Study;
class SUIT_ResourceMgr;

class CAM_Module;

class SALOMEDSClient_Study;
class SALOMEDSClient_SObject;

class SalomeApp_Study;
class SalomeApp_Module;
class LightApp_SelectionMgr;

namespace SMESH {

  SUIT_Desktop*
  GetDesktop(const CAM_Module* theModule);

  LightApp_SelectionMgr*
  GetSelectionMgr(const SalomeApp_Module* theModule);

  SalomeApp_Study*
  GetAppStudy(const CAM_Module* theModule);

  SUIT_ResourceMgr*
  GetResourceMgr( const SalomeApp_Module* );
  
  _PTR(Study)
  GetCStudy(const SalomeApp_Study* theStudy);

  CORBA::Object_var DataOwnerToObject(const LightApp_DataOwnerPtr& theOwner);

  template<class TInterface> typename TInterface::_var_type
    DataOwnerToInterface(const LightApp_DataOwnerPtr& theDataOwner)
    {
      CORBA::Object_var anObj = DataOwnerToObject(theDataOwner);
      if(!CORBA::is_nil(anObj))
	return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }


  SUIT_Study* GetActiveStudy();

  SUIT_ViewWindow* GetActiveWindow();

  _PTR(Study) GetActiveStudyDocument();

  _PTR(SObject) FindSObject(CORBA::Object_ptr theObject);

  void SetName  (_PTR(SObject) theSObject, const char* theName);
  void SetValue (_PTR(SObject) theSObject, const char* theValue);
  void setFileType (_PTR(SObject) theSObject, const char* theValue);
  void setFileName (_PTR(SObject) theSObject, const char* theValue);

  CORBA::Object_var SObjectToObject (_PTR(SObject) theSObject,
				     _PTR(Study) theStudy);

  CORBA::Object_var SObjectToObject(_PTR(SObject) theSObject);

  template<class TInterface> typename TInterface::_var_type
    SObjectToInterface(_PTR(SObject) theSObject)
    {
      CORBA::Object_var anObj = SObjectToObject(theSObject);
      if(!CORBA::is_nil(anObj))
	return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

  CORBA::Object_var IObjectToObject(const Handle(SALOME_InteractiveObject)& theIO);

  template<class TInterface> typename TInterface::_var_type
    IObjectToInterface(const Handle(SALOME_InteractiveObject)& theIO)
    {
      CORBA::Object_var anObj = IObjectToObject(theIO);
      if(!CORBA::is_nil(anObj))
	return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

  CORBA::Object_var IORToObject(const char* theIOR);

  template<class TInterface> typename TInterface::_var_type
    IORToInterface(const char* theIOR)
    {
      CORBA::Object_var anObj = IORToObject(theIOR);
      if(!CORBA::is_nil(anObj))
	return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

  int GetNameOfSelectedIObjects (LightApp_SelectionMgr*, QString& theName);

  _PTR(SObject) GetMeshOrSubmesh (_PTR(SObject) theSObject);

  void ModifiedMesh (_PTR(SObject) theSObject, bool theIsRight);

//  void UpdateObjBrowser (bool);
}

#endif
