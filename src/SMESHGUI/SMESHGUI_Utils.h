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
// File   : SMESHGUI_Utils.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_UTILS_H
#define SMESHGUI_UTILS_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// CORBA includes
#include <omniORB4/CORBA.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>
#include <SALOME_GenericObj_wrap.hxx>

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>
#include <LightApp_DataOwner.h>

//OCC includes
#include <gp_XYZ.hxx>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)

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

class SMDS_MeshFace;

namespace SMESH
{
SMESHGUI_EXPORT
  SUIT_Desktop*
  GetDesktop( const CAM_Module* );

SMESHGUI_EXPORT
  LightApp_SelectionMgr*
  GetSelectionMgr( const SalomeApp_Module* );

SMESHGUI_EXPORT
  SalomeApp_Study*
  GetAppStudy( const CAM_Module* );

SMESHGUI_EXPORT
  SUIT_ResourceMgr*
  GetResourceMgr( const SalomeApp_Module* );
  
SMESHGUI_EXPORT
  _PTR(Study)
  GetCStudy( const SalomeApp_Study* );

SMESHGUI_EXPORT
  CORBA::Object_var DataOwnerToObject( const LightApp_DataOwnerPtr& );

  template<class TInterface> typename TInterface::_var_type
    DataOwnerToInterface( const LightApp_DataOwnerPtr& theDataOwner )
    {
      CORBA::Object_var anObj = DataOwnerToObject(theDataOwner);
      if(!CORBA::is_nil(anObj))
        return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

SMESHGUI_EXPORT
  SUIT_Study* GetActiveStudy();

SMESHGUI_EXPORT
  SUIT_ViewWindow* GetActiveWindow();

SMESHGUI_EXPORT
  _PTR(Study) GetActiveStudyDocument();

SMESHGUI_EXPORT
  _PTR(SObject) FindSObject( CORBA::Object_ptr );

SMESHGUI_EXPORT
  void SetName( _PTR(SObject), const QString& );

SMESHGUI_EXPORT
  void SetValue( _PTR(SObject), const QString& );
  void setFileType( _PTR(SObject), const QString& );
  void setFileName( _PTR(SObject), const QString& );

SMESHGUI_EXPORT
  CORBA::Object_var SObjectToObject( _PTR(SObject),
                                     _PTR(Study) );

SMESHGUI_EXPORT
  CORBA::Object_var SObjectToObject( _PTR(SObject) );

  template<class TInterface> typename TInterface::_var_type
    SObjectToInterface( _PTR(SObject) theSObject )
    {
      CORBA::Object_var anObj = SObjectToObject(theSObject);
      if(!CORBA::is_nil(anObj))
        return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

SMESHGUI_EXPORT
  _PTR(SObject) ObjectToSObject( CORBA::Object_ptr );

SMESHGUI_EXPORT
  CORBA::Object_var IObjectToObject( const Handle(SALOME_InteractiveObject)& );

  template<class TInterface> typename TInterface::_var_type
    IObjectToInterface( const Handle(SALOME_InteractiveObject)& theIO )
    {
      CORBA::Object_var anObj = IObjectToObject(theIO);
      if(!CORBA::is_nil(anObj))
        return TInterface::_narrow(anObj);
      return TInterface::_nil();
    }

SMESHGUI_EXPORT
  CORBA::Object_var IORToObject( const QString& );

  template<class TInterface> typename TInterface::_var_type
    IORToInterface( const QString& theIOR )
    {
      CORBA::Object_var anObj = IORToObject( theIOR );
      if ( !CORBA::is_nil( anObj ) )
        return TInterface::_narrow( anObj );
      return TInterface::_nil();
    }

SMESHGUI_EXPORT
  int GetNameOfSelectedIObjects( LightApp_SelectionMgr*, QString& );

SMESHGUI_EXPORT
  _PTR(SObject) GetMeshOrSubmesh( _PTR(SObject) );

SMESHGUI_EXPORT
  void ModifiedMesh( _PTR(SObject), bool, bool = false );

SMESHGUI_EXPORT
  void ShowHelpFile( const QString& );

  /*!
   * \brief Return the normal to a face
    * \param theFace - input face
    * \retval gp_XYZ - normal to a face 
   */
SMESHGUI_EXPORT
  gp_XYZ getNormale( const SMDS_MeshFace* theFace );


  // type to use instead of SMESH_IDSource_var for automatic UnRegister()
  typedef SALOME::GenericObj_wrap<SMESH_IDSource> IDSource_wrap;

  /*!
   * \brief Class usefull to convert a string returned from a CORBA call
   *        to other string types w/o memory leak.
   *
   *        Usage (of instantiations): QString s = toQStr( objVar->GetName() );
   *                              std::string ss = toStdStr( objVar->GetName() );
   */
  template < class _STRING >
  class toStrT : public _STRING {
    CORBA::String_var myStr;
  public:
    toStrT( char* s ): _STRING( s ), myStr(s)
    {}
    operator const char*() const
    { return myStr.in(); }
  };
  // Instantiations:
  struct toQStr : public toStrT< QString > {
    toQStr( char* s ): toStrT< QString >(s) {}
  };
  struct toStdStr : public toStrT< std::string > {
    toStdStr( char* s ): toStrT< std::string >(s) {}
  };

  QString fromUtf8( const char* txt );
  QString fromUtf8( const std::string& txt );
  struct toUtf8: public std::string
  {
    toUtf8( const QString& txt );
    operator const char*() const { return c_str(); }
  };
}

#endif // SMESHGUI_UTILS_H
