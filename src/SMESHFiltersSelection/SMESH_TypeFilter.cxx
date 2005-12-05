// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
#include "SMESH_TypeFilter.hxx"

#include <SUIT_Session.h>

#include <SalomeApp_Study.h>
#include <LightApp_DataOwner.h>

SMESH_TypeFilter::SMESH_TypeFilter (MeshObjectType theType) 
{
  myType = theType;
}

SMESH_TypeFilter::~SMESH_TypeFilter() 
{
}

bool SMESH_TypeFilter::isOk (const SUIT_DataOwner* theDataOwner) const
{
  bool Ok = false, extractReference = true;

  const LightApp_DataOwner* owner =
    dynamic_cast<const LightApp_DataOwner*>(theDataOwner);
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>
    (SUIT_Session::session()->activeApplication()->activeStudy());

  if (owner && appStudy) {
    _PTR(Study) study = appStudy->studyDS();
    QString entry = owner->entry();

    _PTR(SObject) obj (study->FindObjectID(entry.latin1())), aRefSO;
    if( extractReference && obj && obj->ReferencedObject( aRefSO ) )
      obj = aRefSO;
    if (!obj) return false;

    _PTR(SObject) objFather = obj->GetFather();
    _PTR(SComponent) objComponent = obj->GetFatherComponent();

    if( objComponent->ComponentDataType()!="SMESH" )
      return false;

    int aLevel = obj->Depth() - objComponent->Depth();

    // Max level under the component is 5:
    //
    // 0    Mesh Component
    // 1    |- Hypotheses
    // 2    |  |- Regular 1D
    //      |- Algorithms
    //      |- Mesh 1
    //         |- * Main Shape
    //         |- Applied Hypotheses
    //         |- Applied Algorithms
    //         |- Submeshes on Face
    // 3       |  |- SubmeshFace
    // 4       |     |- * Face 1
    // 4       |     |- Applied algorithms ( selectable in Use Case Browser )
    // 5       |          |- Regular 1D
    //         |- Group Of Nodes

    if (aLevel <= 0)
      return false;

    switch (myType)
      {
      case HYPOTHESIS:
	{
	  if ( aLevel == 2 && ( objFather->Tag() == 1 )) // hypo definition
	    Ok = true;
	  else if ( aLevel == 3 && ( objFather->Tag() == 2 )) // applied global hypo
	    Ok = true;
	  else if ( aLevel == 5 && ( objFather->Tag() == 2 )) // applied local hypo
	    Ok = true;
	  break;
	}
      case ALGORITHM:
	{
	  if ( aLevel == 2 && ( objFather->Tag() == 2 )) // algo definition
	    Ok = true;
	  else if ( aLevel == 3 && ( objFather->Tag() == 3 )) // applied global algo
	    Ok = true;
	  else if ( aLevel == 5 && ( objFather->Tag() == 3 )) // applied local algo
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

MeshObjectType SMESH_TypeFilter::type() const 
{
  return myType;
}
