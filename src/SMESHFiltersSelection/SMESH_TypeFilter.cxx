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

  bool Ok = false;

  if ( anObj->hasEntry() ) {
    QAD_Study* ActiveStudy = QAD_Application::getDesktop()->getActiveStudy();
    SALOMEDS::Study_var aStudy = ActiveStudy->getStudyDocument();
    SALOMEDS::SObject_var obj = aStudy->FindObjectID( anObj->getEntry() );

    SALOMEDS::SObject_var objFather = obj->GetFather();
    SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
    
    if ( strlen( obj->GetID() ) <= strlen( objComponent->GetID() ) )
      return false;

    switch ( myKind )
      {
      case HYPOTHESIS:
	{
	  if (( objFather->Tag() == 1 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case ALGORITHM:
	{
	  if (( objFather->Tag() == 2 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case MESH:
	{
	  if (( obj->Tag() >= 3 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) == 0 ) )
	    Ok = true;
	  break;
	}
      case SUBMESH:
	{
	  if (( objFather->Tag() >= 4 && objFather->Tag() < 9 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      case MESHorSUBMESH:
	{
	  if (( obj->Tag() >= 3 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) == 0 ) )
	    Ok = true;

	  if (( objFather->Tag() >= 4 && objFather->Tag() < 9 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;

	  break;
	}
      case SUBMESH_VERTEX:  // Label "SubMeshes on vertexes"
	{
	  if (( obj->Tag() == 4 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_EDGE:
	{
	  if (( obj->Tag() == 5 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_FACE:
	{
	  if (( obj->Tag() == 6 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_SOLID:
	{
	  if (( obj->Tag() == 7 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_COMPOUND:
	{
	  if (( obj->Tag() == 8 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) && ( objFather->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case GROUP:
	{
	  if (( objFather->Tag() >= 9 ) && (strcmp( objFather->GetID(), objComponent->GetID() ) != 0 ) )
	    Ok = true;
	  break;
	}
      }
  }
  return Ok;
}
