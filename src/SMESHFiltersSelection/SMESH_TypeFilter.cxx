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

using namespace std;

SMESH_TypeFilter::SMESH_TypeFilter(MeshObjectType aType) 
{
  myKind = aType;
}

Standard_Boolean SMESH_TypeFilter::IsOk(const Handle(SALOME_InteractiveObject)& anObj) const 
{
  Handle(SALOME_TypeFilter) meshFilter = new SALOME_TypeFilter( "SMESH" );
  if ( !meshFilter->IsOk(anObj) ) 
    return false;

  bool Ok = false;

  if ( anObj->hasEntry() ) {
    QAD_Study* ActiveStudy = QAD_Application::getDesktop()->getActiveStudy();
    SALOMEDS::Study_var aStudy = ActiveStudy->getStudyDocument();
    SALOMEDS::SObject_var obj = aStudy->FindObjectID( anObj->getEntry() );

    SALOMEDS::SObject_var objFather = obj->GetFather();
    SALOMEDS::SComponent_var objComponent = obj->GetFatherComponent();
    
    int aLevel = obj->Depth() - objComponent->Depth();

    // Max level under the component is 4:
    //
    // 0    Mesh Component
    // 1    |- Hypotheses
    // 2    |  |- Regular 1D
    //      |- Algorithms
    //      |- Mesh 1
    //         |- Applied Hypotheses
    //         |- Applied Algorithms
    //         |- Submeshes on Face
    // 3       |  |- SubmeshFace
    // 4       |     |- Applied algorithms ( selectable in Use Case Browser )
    //         |- Group Of Nodes

    if ( aLevel <= 0 )
      return false;

    switch ( myKind )
      {
      case HYPOTHESIS:
	{
	  if ( aLevel == 2 && ( objFather->Tag() == 1 ))
	    Ok = true;
	  break;
	}
      case ALGORITHM:
	{
	  if ( aLevel == 2 && ( objFather->Tag() == 2 ))
	    Ok = true;
	  break;
	}
      case MESH:
	{
	  if ( aLevel == 1 && ( obj->Tag() >= 3 ))
	    Ok = true;
	  break;
	}
      case SUBMESH:
	{
	  // see SMESH_Gen_i.cxx for tag numbers
	  if ( aLevel == 3 && ( objFather->Tag() >= 4 && objFather->Tag() <= 10 ))
	    Ok = true;
	  break;
	}
      case MESHorSUBMESH:
	{
	  if ( aLevel == 1 && ( obj->Tag() >= 3 ))
	    Ok = true; // mesh
          else if ( aLevel == 3 && ( objFather->Tag() >= 4 && objFather->Tag() <= 10 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_VERTEX:  // Label "SubMeshes on vertexes"
	{
	  if ( aLevel == 3 && ( objFather->Tag() == 4 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_EDGE:
	{
	  if ( aLevel == 3 && ( objFather->Tag() == 5 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_FACE:
	{
	  if ( aLevel == 3 && ( objFather->Tag() == 7 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_SOLID:
	{
	  if ( aLevel == 3 && ( objFather->Tag() == 9 ))
	    Ok = true;
	  break;
	}
      case SUBMESH_COMPOUND:
	{
	  if ( aLevel == 3 && ( objFather->Tag() == 10 ))
	    Ok = true;
	  break;
	}
      case GROUP:
	{
	  if ( aLevel == 3 && ( objFather->Tag() > 10 ))
	    Ok = true;
	  break;
	}
      }
  }
  return Ok;
}
