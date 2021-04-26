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

//  SMESH SMDS : implementation of Salome mesh data structure
//  File   : SMDS_MeshVolume.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH
//

#include "SMDS_MeshVolume.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMDS_VtkCellIterator.hxx"

#include <boost/make_shared.hpp>

// init a polyherdon
void SMDS_MeshVolume::init( const std::vector<const SMDS_MeshNode*>& nodes,
                            const std::vector<int>&                  nbNodesPerFace )
{
  std::vector<vtkIdType> ptIds;
  ptIds.reserve( nodes.size() + nbNodesPerFace.size() + 1 );

  size_t nbFaces = nbNodesPerFace.size();
  for ( size_t iN = 0, iF = 0; iF < nbFaces; iF++ )
  {
    int nf = nbNodesPerFace[iF];
    ptIds.push_back(nf);
    for (int n = 0; n < nf; n++)
      ptIds.push_back( nodes[ iN++ ]->GetVtkID() );
  }

  vtkIdType vtkID = getGrid()->InsertNextLinkedCell(VTK_POLYHEDRON, nbFaces, &ptIds[0]);
  setVtkID( vtkID );
}

void SMDS_MeshVolume::init( const std::vector<vtkIdType>& vtkNodeIds )
{
  SMDSAbs_EntityType aType = SMDSEntity_Tetra;
  switch ( vtkNodeIds.size()) // cases are in order of usage frequency
  {
  case 4:  aType = SMDSEntity_Tetra;           break;
  case 5:  aType = SMDSEntity_Pyramid;         break;
  case 8:  aType = SMDSEntity_Hexa;            break;
  case 6:  aType = SMDSEntity_Penta;           break;
  case 10: aType = SMDSEntity_Quad_Tetra;      break;
  case 20: aType = SMDSEntity_Quad_Hexa;       break;
  case 13: aType = SMDSEntity_Quad_Pyramid;    break;
  case 27: aType = SMDSEntity_TriQuad_Hexa;    break;
  case 15: aType = SMDSEntity_Quad_Penta;      break;
  case 18: aType = SMDSEntity_BiQuad_Penta;    break;
  case 12: aType = SMDSEntity_Hexagonal_Prism; break;
  default: throw SALOME_Exception("wrong volume nodes");
  }
  SMDS_MeshCell::init( aType, vtkNodeIds );
}

bool SMDS_MeshVolume::ChangeNodes(const std::vector<const SMDS_MeshNode*>& nodes,
                                  const std::vector<int>&                  quantities) const
{
  if ( !IsPoly() )
    return false;

  vtkIdType nFaces = 0;
  vtkIdType const *tmp(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, tmp );
  vtkIdType *ptIds = const_cast<vtkIdType*>( tmp );

  // stream size and nb faces should not change

  if ((vtkIdType) quantities.size() != nFaces )
  {
    return false;
  }
  size_t id = 0, nbPoints = 0;
  for ( int i = 0; i < nFaces; i++ )
  {
    int nodesInFace = ptIds[id];
    nbPoints += nodesInFace;
    id += (nodesInFace + 1);
  }
  if ( nodes.size() != nbPoints )
  {
    return false;
  }

  // update ptIds
  size_t iP = 0, iN = 0;
  for ( size_t i = 0; i < quantities.size(); ++i )
  {
    ptIds[ iP++ ] = quantities[ i ]; // nb face nodes
    for ( int j = 0; j < quantities[ i ]; ++j )
      ptIds[ iP++ ] = nodes[ iN++ ]->GetVtkID();
  }
  return true;
}

const SMDS_MeshNode* SMDS_MeshVolume::GetNode(const int ind) const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::GetNode( ind );

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  int id = 0, nbPoints = 0;
  for (int i = 0; i < nFaces; i++)
  {
    int nodesInFace = ptIds[id];
    if ( ind < nbPoints + nodesInFace )
      return GetMesh()->FindNodeVtk( ptIds[ 1 + ind + i ]);
    nbPoints += nodesInFace;
    id += (nodesInFace + 1);
  }
  return 0;
}
int SMDS_MeshVolume::NbNodes() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::NbNodes();

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  int id = 0, nbPoints = 0;
  for (int i = 0; i < nFaces; i++)
  {
    int nodesInFace = ptIds[id];
    nbPoints += nodesInFace;
    id += (nodesInFace + 1);
  }
  return nbPoints;
}

int SMDS_MeshVolume::NbFaces() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::NbFaces();

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  return nFaces;
  
}
int SMDS_MeshVolume::NbEdges() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::NbEdges();

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  int id = 0, nbEdges = 0;
  for (int i = 0; i < nFaces; i++)
  {
    int edgesInFace = ptIds[id];
    id += (edgesInFace + 1);
    nbEdges += edgesInFace;
  }
  nbEdges = nbEdges / 2;
  return nbEdges;
}

int SMDS_MeshVolume::GetNodeIndex( const SMDS_MeshNode* node ) const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::GetNodeIndex( node );

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  int id = 0;
  for (int iF = 0; iF < nFaces; iF++)
  {
    int nodesInFace = ptIds[id];
    for ( vtkIdType i = 0; i < nodesInFace; ++i )
      if ( ptIds[id+i+1] == node->GetVtkID() )
        return id+i-iF;
    id += (nodesInFace + 1);
  }
  return -1;
}
bool SMDS_MeshVolume::ChangeNodes(const SMDS_MeshNode* /*nodes*/[], const int /*nbNodes*/)
{
  return false;
}

bool SMDS_MeshVolume::IsMediumNode(const SMDS_MeshNode* node) const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::IsMediumNode( node );

  return false;
}

int SMDS_MeshVolume::NbCornerNodes() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::NbCornerNodes();

  return NbNodes();
}

int SMDS_MeshVolume::NbFaceNodes (const int face_ind) const
{
  if ( !IsPoly() )
    return SMDS_VolumeTool( this ).NbFaceNodes( face_ind-1 );

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
  int id = 0, nbNodes = 0;
  for (int i = 0; i < nFaces; i++)
  {
    int nodesInFace = ptIds[id];
    id += (nodesInFace + 1);
    if (i == face_ind - 1)
    {
      nbNodes = nodesInFace;
      break;
    }
  }
  return nbNodes;
}

const SMDS_MeshNode* SMDS_MeshVolume::GetFaceNode (const int face_ind, const int node_ind) const
{
  if ( !IsPoly() )
    return SMDS_VolumeTool( this ).GetFaceNodes( face_ind-1 )[ node_ind - 1 ];

  vtkIdType nFaces = 0;
  vtkIdType const *ptIds(nullptr);
  getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds);
  int id = 0;
  for (int i = 0; i < nFaces; i++)
  {
    int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
    if (i == face_ind - 1) // first face is number 1
    {
      if ((node_ind > 0) && (node_ind <= nodesInFace))
        return GetMesh()->FindNodeVtk(ptIds[id + node_ind]); // ptIds[id+1] : first node
    }
    id += (nodesInFace + 1);
  }
  return 0;
}

std::vector<int> SMDS_MeshVolume::GetQuantities() const
{
  std::vector<int> quantities;
  if ( IsPoly() )
  {
    vtkIdType nFaces = 0;
    vtkIdType const *ptIds(nullptr);
    getGrid()->GetFaceStream( GetVtkID(), nFaces, ptIds );
    int id = 0;
    for (int i = 0; i < nFaces; i++)
    {
      int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
      quantities.push_back( nodesInFace );
      id += (nodesInFace + 1);
    }
  }
  return quantities;
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on nodes owned by the element.
///////////////////////////////////////////////////////////////////////////////
SMDS_ElemIteratorPtr SMDS_MeshVolume::nodesIterator() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::nodesIterator();

  return boost::make_shared< SMDS_VtkCellIteratorPolyH<> >( GetMesh(), GetVtkID(), GetEntityType());
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on nodes owned by the element.
///////////////////////////////////////////////////////////////////////////////
SMDS_NodeIteratorPtr SMDS_MeshVolume::nodeIterator() const
{
  if ( !IsPoly() )
    return SMDS_MeshCell::nodeIterator();

  return boost::make_shared< SMDS_VtkCellIteratorPolyH< SMDS_NodeIterator> >( GetMesh(), GetVtkID(), GetEntityType());
}
