//  SMESH SMESHFiltersSelection : filter selector for viewer
//
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
//
//
//
//  File   : SMESH_TypeFilter.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_TypeFilter.ixx"

#include "SALOME_InteractiveObject.hxx"
#include "SALOME_TypeFilter.hxx"

#include "utilities.h"
#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_Study.h"


SMESH_TypeFilter::SMESH_TypeFilter(MeshObjectType aType) 
{
  myKind = aType;
}

Standard_Boolean SMESH_TypeFilter::IsOk(const Handle(SALOME_InteractiveObject)& anObj) const 
{
  Handle(SALOME_TypeFilter) meshFilter = new SALOME_TypeFilter( "MESH" );
  if ( !meshFilter->IsOk(anObj) ) 
    return false;

  if ( anObj->hasEntry() ) {
    QAD_Study* ActiveStudy = QAD_Application::getDesktop()->getActiveStudy();
    SALOMEDS::Study_var aStudy = ActiveStudy->getStudyDocument();
    SALOMEDS::SObject_var obj = aStudy->FindObjectID( anObj->getEntry() );

    bool Ok = false;

    switch ( myKind )
      {
      case HYPOTHESIS:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  if (( objFather->Tag() == 1 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case ALGORITHM:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  if (( objFather->Tag() == 2 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case MESH:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() >= 3 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) == 0 ) )
	    Ok = true;
	  break;
	}
      case SUBMESH:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( objFather->Tag() >= 4 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case MESHorSUBMESH:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() >= 3 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) == 0 ) )
	    Ok = true;

	   if (( objFather->Tag() >= 4 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;

	  break;
	}
      case SUBMESH_VERTEX:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() == 4 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_EDGE:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() == 5 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_FACE:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() == 6 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_SOLID:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() == 7 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_COMPOUND:
	{
	  SALOMEDS::SObject_var objFather = obj->GetFather();
	  SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
	  
	  if (( obj->Tag() == 8 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      }

    if ( Ok )
      return true;
  }
  return false;
}
