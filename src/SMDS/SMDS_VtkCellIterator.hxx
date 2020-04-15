// Copyright (C) 2010-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef _SMDS_VTKCELLITERATOR_HXX_
#define _SMDS_VTKCELLITERATOR_HXX_

#include "SMDS_ElemIterator.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDSAbs_ElementType.hxx"

#include <vtkCell.h>

typedef std::vector< vtkIdType > TVtkIdList;

//--------------------------------------------------------------------------------
/*!
 * \brief Retrieve nodes of a cell
 */
struct _GetVtkNodes
{
  _GetVtkNodes( TVtkIdList& nodeIds, SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType type);
};
struct _GetVtkNodesToUNV
{
  _GetVtkNodesToUNV( TVtkIdList& nodeIds, SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType type);
};
struct _GetVtkNodesPolyh
{
  _GetVtkNodesPolyh( TVtkIdList& nodeIds, SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType type);
};

//--------------------------------------------------------------------------------
/*!
 * \brief Iterator on nodes of a cell
 */
template< class SMDS_ITERATOR = SMDS_ElemIterator, class GET_VTK_NODES = _GetVtkNodes >
class SMDS_VtkCellIterator: public SMDS_ITERATOR
{
public:
  typedef typename SMDS_ITERATOR::value_type result_type;

  SMDS_VtkCellIterator(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType)
    : _mesh(mesh), _index(0)
  {
    GET_VTK_NODES getNodes( _vtkIdList, mesh, vtkCellId, aType );
  }
  virtual ~SMDS_VtkCellIterator() {}
  virtual bool        more()      {  return ( _index < _vtkIdList.size() ); }
  virtual result_type next()      {
    vtkIdType id = _vtkIdList[ _index++ ];
    return static_cast<result_type>( _mesh->FindNodeVtk( id ));
  }
protected:
  SMDS_Mesh* _mesh;
  size_t     _index;
  TVtkIdList _vtkIdList;
};

//--------------------------------------------------------------------------------
template< class SMDS_ITERATOR = SMDS_ElemIterator >
class SMDS_VtkCellIteratorToUNV: public SMDS_VtkCellIterator< SMDS_ITERATOR, _GetVtkNodesToUNV >
{
  typedef SMDS_VtkCellIterator< SMDS_ITERATOR, _GetVtkNodesToUNV > parent_t;
public:
  SMDS_VtkCellIteratorToUNV(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType type):
    parent_t( mesh, vtkCellId, type ) {}
};

//--------------------------------------------------------------------------------
template< class SMDS_ITERATOR = SMDS_ElemIterator >
class SMDS_VtkCellIteratorPolyH: public SMDS_VtkCellIterator< SMDS_ITERATOR, _GetVtkNodesPolyh >
{
  typedef SMDS_VtkCellIterator< SMDS_ITERATOR, _GetVtkNodesPolyh > parent_t;
public:
  SMDS_VtkCellIteratorPolyH(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType type):
    parent_t( mesh, vtkCellId, type ) {}
};

#endif
