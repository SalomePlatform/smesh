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
//  File   : SMDS_ElementHolder.hxx
//  Module : SMESH
//
#ifndef _SMDS_ElementHolder_HeaderFile
#define _SMDS_ElementHolder_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_ElemIterator.hxx"

#include <smIdType.hxx>
#include <vtkType.h>

#include <vector>
#include <set>

class SMDS_Mesh;
class SMDS_MeshElement;

//------------------------------------------------------------------------------------
/*!
 * \brief Base class of object holding SMDS_MeshElement pointers.
 *        Registering such an object in SMDS_Mesh assures that the
 *        pointers remain valid after compacting the mesh
 */
class SMDS_EXPORT SMDS_ElementHolder
{
 public:

  //! register self in the mesh
  SMDS_ElementHolder( const SMDS_Mesh* mesh );

  //! un-register self from the mesh
  virtual ~SMDS_ElementHolder();


 protected:

  //!< the descendant object return its elements just before the mesh compacting
  virtual SMDS_ElemIteratorPtr getElements() = 0;

  //!< the descendant object temporary remove its elements
  virtual void tmpClear() = 0;

  //!< the descendant object re-add its elements after the mesh compacting
  virtual void add( const SMDS_MeshElement* element ) = 0;

  //!< the descendant squeeze its element storage after re-adding elements
  virtual void compact() = 0;

  //!< allow the descendant treat its elements before mesh clearing
  virtual void clear() {}

  SMDS_Mesh* myMesh;


 private: // methods called by SMDS_Mesh

  friend class SMDS_Mesh;

  //! store vtkIDs of elements
  void beforeCompacting();

  //! restore pointers to elements
  void restoreElements( const std::vector<smIdType>& idNodessOldToNew,
                        const std::vector<smIdType>& idCellsOldToNew );


  std::vector<const SMDS_MeshElement*>      myExternalElems; //!< elements not contained in the mesh
  std::vector< vtkIdType >                  myVtkIDs;        //!< vtk IDs of elements
  std::vector< bool >                       myIsNode;
  std::set< SMDS_ElementHolder* >::iterator myPtrInMesh;
};

#endif
