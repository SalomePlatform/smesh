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

#include "SALOME_InteractiveObject.hxx"

class QString;

class QAD_Study;
class QAD_StudyFrame;
class SALOME_Selection;

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS)


namespace SMESH{

  QAD_Study* GetActiveStudy();

  QAD_StudyFrame* GetActiveStudyFrame();

  SALOMEDS::Study_var GetActiveStudyDocument();

  SALOMEDS::SObject_var FindSObject(CORBA::Object_ptr theObject);


  void SetName(SALOMEDS::SObject_ptr theSObject, const char* theName);
  void SetValue(SALOMEDS::SObject_ptr theSObject, const char* theValue);


  CORBA::Object_var SObjectToObject(SALOMEDS::SObject_ptr theSObject,
				    SALOMEDS::Study_ptr theStudy);
  
  CORBA::Object_var SObjectToObject(SALOMEDS::SObject_ptr theSObject);
  

  template<class TInterface> typename TInterface::_var_type
    SObjectToInterface(SALOMEDS::SObject_ptr theSObject)
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
  

  int GetNameOfSelectedIObjects(SALOME_Selection* theSel, QString& theName);

  SALOMEDS::SObject_var GetMeshOrSubmesh(SALOMEDS::SObject_ptr theSObject);

  void ModifiedMesh(SALOMEDS::SObject_ptr theSObject, bool theIsRight);

}


#endif
