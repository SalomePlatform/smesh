// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_MeshGroup.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshGroup_HeaderFile
#define _SMDS_MeshGroup_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_ElementHolder.hxx"
#include "SMDS_Mesh.hxx"
#include <set>

class SMDS_EXPORT SMDS_MeshGroup: public SMDS_MeshObject, SMDS_ElementHolder
{
 public:
  SMDS_MeshGroup(const SMDS_Mesh *         theMesh,
                 const SMDSAbs_ElementType theType = SMDSAbs_All);

  void SetType (const SMDSAbs_ElementType theType);
  void Clear();
  void Reserve(size_t nbElems) {}
  bool Add(const SMDS_MeshElement * theElem);
  bool Remove(const SMDS_MeshElement * theElem);
  bool IsEmpty() const { return myElements.empty(); }
  int  Extent() const { return myElements.size(); }
  int  Tic() const { return myTic; }
  bool Contains(const SMDS_MeshElement * theElem) const;

  const SMDS_Mesh*     GetMesh() const { return myMesh; }
  SMDSAbs_ElementType  GetType() const { return myType; }
  SMDS_ElemIteratorPtr GetElements() const; // WARNING: iterator becomes invalid if group changes

  void operator=( SMDS_MeshGroup && other );

 protected: // methods of SMDS_ElementHolder

  virtual SMDS_ElemIteratorPtr getElements() { return GetElements(); }
  virtual void tmpClear();
  virtual void add( const SMDS_MeshElement* element ) { Add( element ); }
  virtual void compact() {}

 private:

  typedef std::set< const SMDS_MeshElement* > TElementSet;
  typedef TElementSet::const_iterator         TIterator;

  const SMDS_Mesh *   myMesh;
  SMDSAbs_ElementType myType;
  TElementSet         myElements; // not sorted by ID because it can contain deleted elements
  int                 myTic;      // to track changes
};
#endif
