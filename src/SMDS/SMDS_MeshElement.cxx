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
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_MeshElement.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_ElementFactory.hxx"

#include "utilities.h"

//================================================================================
/*!
 * \brief Constructor of a non-used element
 */
//================================================================================

SMDS_MeshElement::SMDS_MeshElement(): myHolder(0)
{
}

//================================================================================
/*!
 * \brief Check if a node is a medium node of a quadratic cell
 */
//================================================================================

bool SMDS_MeshElement::IsMediumNode(const SMDS_MeshNode* node) const
{
  return !( GetNodeIndex( node ) < NbCornerNodes() );
}

//================================================================================
/*!
 * \brief Return true if index of node is valid (0 <= ind < NbNodes())
 *  \param ind - node index
 *  \retval bool - index check result
 */
//================================================================================

bool SMDS_MeshElement::IsValidIndex(const int ind) const
{
  return ( ind>-1 && ind<NbNodes() );
}

//================================================================================
/*!
 * \brief Return a valid corner node index, fixing the given one if necessary
 * \param ind - node index
 * \retval int - valid node index
 */
//================================================================================

int SMDS_MeshElement::WrappedIndex(const int ind) const
{
  if ( ind < 0 ) return NbCornerNodes() + ind % NbCornerNodes();
  if ( ind >= NbCornerNodes() ) return ind % NbCornerNodes();
  return ind;
}

//================================================================================
/*!
 * \brief Check if a node belongs to the element
 * \param node - the node to check
 * \retval int - node index within the element, -1 if not found
 */
//================================================================================

int SMDS_MeshElement::GetNodeIndex( const SMDS_MeshNode* node ) const
{
  SMDS_ElemIteratorPtr nIt = nodesIterator();
  for ( int i = 0; nIt->more(); ++i )
    if ( nIt->next() == node )
      return i;
  return -1;
}

//================================================================================
/*!
 * \brief Return ID of an element
 */
//================================================================================

smIdType SMDS_MeshElement::GetID() const
{
  return myHolder ? myHolder->GetID( this ) : -1;
}

//================================================================================
/*!
 * \brief Set ID of a shape this element was generated on
 */
//================================================================================

void SMDS_MeshElement::setShapeID( const int shapeID ) const
{
  const_cast<SMDS_ElementChunk*>( myHolder )->SetShapeID( this, shapeID );
}

//================================================================================
/*!
 * \brief Return ID of a shape this element was generated on
 */
//================================================================================

int SMDS_MeshElement::GetShapeID() const
{
  return myHolder->GetShapeID( this );
}

//================================================================================
/*!
 * \brief Return VTK ID of this element
 */
//================================================================================

vtkIdType SMDS_MeshElement::GetVtkID() const
{
  return myHolder->GetVtkID( this );
}

//================================================================================
/*!
 * \brief Mark this element
 */
//================================================================================

void SMDS_MeshElement::setIsMarked( bool is ) const
{
  const_cast<SMDS_ElementChunk*>( myHolder )->SetIsMarked( this, is );
}

//================================================================================
/*!
 * \brief Check if this element is marked
 */
//================================================================================

bool SMDS_MeshElement::isMarked() const
{
  return myHolder->IsMarked( this );
}

//================================================================================
/*!
 * \brief Store VTK ID
 */
//================================================================================

void SMDS_MeshElement::setVtkID( const vtkIdType vtkID )
{
  myHolder->SetVTKID( this, vtkID );
}

//================================================================================
/*!
 * \brief Return the mesh this element belongs to
 */
//================================================================================

SMDS_Mesh* SMDS_MeshElement::GetMesh() const
{
  return const_cast<SMDS_ElementChunk*>( myHolder )->GetMesh();
}

//================================================================================
/*!
 * \brief Return a SMDS_UnstructuredGrid
 */
//================================================================================

SMDS_UnstructuredGrid* SMDS_MeshElement::getGrid() const
{
  return const_cast<SMDS_ElementChunk*>( myHolder )->GetMesh()->GetGrid();
}

//================================================================================
/*!
 * \brief Print self
 */
//================================================================================

void SMDS_MeshElement::Print(ostream & OS) const
{
  OS << "dump of mesh element" << endl;
}

ostream & operator <<(ostream & OS, const SMDS_MeshElement * e)
{
  e->Print(OS);
  return OS;
}
