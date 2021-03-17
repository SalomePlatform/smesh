// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_ElementHolder.cxx
//  Module : SMESH
//

#include "SMDS_ElementHolder.hxx"

#include "ObjectPool.hxx"
#include "SMDS_CellOfNodes.hxx"
#include "SMDS_Mesh.hxx"

//=======================================================================
//function : SMDS_ElementHolder
//purpose  : register self in the mesh
//=======================================================================

SMDS_ElementHolder::SMDS_ElementHolder( const SMDS_Mesh* mesh )
  : myMesh( const_cast< SMDS_Mesh* >( mesh ))
{
  if ( myMesh )
    myPtrInMesh = myMesh->myElemHolders.insert( this ).first;
}

//=======================================================================
//function : ~SMDS_ElementHolder
//purpose  : un-register self from the mesh
//=======================================================================

SMDS_ElementHolder::~SMDS_ElementHolder()
{
  if ( myMesh )
    myMesh->myElemHolders.erase( myPtrInMesh );
}

//=======================================================================
//function : beforeCompacting
//purpose  : store vtkIDs of elements
//=======================================================================

void SMDS_ElementHolder::beforeCompacting()
{
  for ( SMDS_ElemIteratorPtr it = getElements(); it->more(); )
  {
    const SMDS_MeshElement* e = it->next();
    if ( !e ) continue;
    if ( e->IsNull() && !dynamic_cast<const SMDS_CellOfNodes*>( e ))
      continue; // removed element
    myIsNode.push_back( e->GetType() == SMDSAbs_Node );
    if ( myMesh->Contains( e ))
    {
      myVtkIDs.push_back( e->GetVtkID() );
    }
    else
    {
      myExternalElems.push_back( e );
      myVtkIDs.push_back( -1 * (vtkIdType) myExternalElems.size() );
    }
  }
}

//=======================================================================
//function : restoreElements
//purpose  : restore pointers to elements
//=======================================================================

void SMDS_ElementHolder::restoreElements( const std::vector<smIdType>& idNodesOldToNew,
                                          const std::vector<smIdType>& idCellsOldToNew )
{
  tmpClear();

  const SMDS_MeshElement* elem;

  std::vector< bool >::iterator isNode = myIsNode.begin();
  for ( size_t i = 0; i < myVtkIDs.size(); ++i, ++isNode )
  {
    vtkIdType vtkID = myVtkIDs[i];
    if ( vtkID < 0 )
    {
      elem = myExternalElems[ (-vtkID)-1 ];
    }
    else if ( *isNode )
    {
      if ( vtkID < (vtkIdType)idNodesOldToNew.size() )
        elem = myMesh->FindNodeVtk( idNodesOldToNew[ vtkID ]);
      else
        elem = myMesh->FindNodeVtk( vtkID );
    }
    else
    {
      if ( vtkID < (vtkIdType)idCellsOldToNew.size() )
        elem = myMesh->FindElementVtk( idCellsOldToNew[ vtkID ]);
      else
        elem = myMesh->FindElementVtk( vtkID );
    }
    if ( elem )
      add( elem );
  }
  clearVector( myExternalElems );
  clearVector( myVtkIDs );
  clearVector( myIsNode );

  compact();
}
