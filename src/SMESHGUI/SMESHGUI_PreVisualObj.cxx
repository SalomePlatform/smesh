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

#include "SMESHGUI_PreVisualObj.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>

SMESHGUI_PreVisualObj::SMESHGUI_PreVisualObj()
{
  myMesh = new SMDS_Mesh();
}

bool SMESHGUI_PreVisualObj::Update( int theIsClear = true )
{
  return false;
}

void SMESHGUI_PreVisualObj::UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor )
{
  if ( theFunctor ) theFunctor->SetMesh( GetMesh() );
}

int SMESHGUI_PreVisualObj::GetElemDimension( const int theObjId )
{
  if ( const SMDS_MeshElement* anElem = myMesh->FindElement( theObjId ))
  {
    switch ( anElem->GetType() )
    {
    case SMDSAbs_Edge  :     return 1;
    case SMDSAbs_Face  :     return 2;
    case SMDSAbs_Volume:     return 3;
    // case SMDSAbs_0DElement : return 0;
    // case SMDSAbs_Ball :      return 0;
    default            :     return 0;
    }
  }
  return -1;
}

int SMESHGUI_PreVisualObj::GetNbEntities( const SMDSAbs_ElementType theType ) const
{
  return myMesh->GetMeshInfo().NbElements( theType );
}

SMESH::SMESH_Mesh_ptr SMESHGUI_PreVisualObj::GetMeshServer()
{
  return SMESH::SMESH_Mesh::_nil();
}

//=================================================================================
// function : GetEdgeNodes
// purpose  : Retrieve ids of nodes from edge of elements ( edge is numbered from 1 )
//=================================================================================

bool SMESHGUI_PreVisualObj::GetEdgeNodes( const int theElemId,
                                          const int theEdgeNum,
                                          int&      theNodeId1,
                                          int&      theNodeId2 ) const
{
  const SMDS_MeshElement* e = myMesh->FindElement( theElemId );
  if ( !e || e->GetType() != SMDSAbs_Face )
    return false;

  int nbNodes = e->NbCornerNodes();
  if ( theEdgeNum < 0 || theEdgeNum > nbNodes )
    return false;

  theNodeId1 = e->GetNode( theEdgeNum-1 )->GetID();
  theNodeId2 = e->GetNode( theEdgeNum % nbNodes )->GetID();

  return true;
}

bool SMESHGUI_PreVisualObj::IsValid() const
{
  return GetNbEntities( SMDSAbs_All ) > 0;
}

vtkUnstructuredGrid* SMESHGUI_PreVisualObj::GetUnstructuredGrid()
{
  return myMesh->getGrid();
}


vtkIdType SMESHGUI_PreVisualObj::GetNodeObjId( int theVTKID )
{
  const SMDS_MeshNode* aNode = myMesh->FindNodeVtk( theVTKID );
  return aNode ? aNode->GetID() : -1;
}

vtkIdType SMESHGUI_PreVisualObj::GetNodeVTKId( int theObjID )
{
  const SMDS_MeshNode* aNode = myMesh->FindNode( theObjID );
  return aNode ? aNode->GetID() : -1;
}

vtkIdType SMESHGUI_PreVisualObj::GetElemObjId( int theVTKID )
{
  return this->GetMesh()->fromVtkToSmds(theVTKID);
}

vtkIdType SMESHGUI_PreVisualObj::GetElemVTKId( int theObjID )
{
  const SMDS_MeshElement* e = myMesh->FindElement(theObjID);
  return e ? e->getVtkId() : -1;
}

void SMESHGUI_PreVisualObj::ClearEntitiesFlags()
{
  myEntitiesState = SMESH_Actor::eAllEntity;
  myEntitiesFlag = false;
}

bool SMESHGUI_PreVisualObj::GetEntitiesFlag()
{
  return myEntitiesFlag;
}

unsigned int SMESHGUI_PreVisualObj::GetEntitiesState()
{
  return myEntitiesState;
}
