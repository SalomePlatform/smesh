using namespace std;
//  File      : SMESH_TypeFilter.cxx
//  Created   : Fri Dec 07 09:57:24 2001
//  Author    : Nicolas REJNERI
//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE
//  $Header$

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
