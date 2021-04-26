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
//  File   : SMDS_MeshElement.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshElement_HeaderFile
#define _SMDS_MeshElement_HeaderFile

#include "SMESH_SMDS.hxx"
        
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_MeshObject.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_StdIterator.hxx"

#include <iostream>

#include <vtkType.h>
#include <vtkCellType.h>
#include <smIdType.hxx>

class SMDS_ElementChunk;
class SMDS_Mesh;
class SMDS_MeshNode;
class SMDS_UnstructuredGrid;

// ============================================================
/*!
 * \brief Base class for elements
 */
// ============================================================


class SMDS_EXPORT SMDS_MeshElement : public SMDS_MeshObject
{
public:

  // ===========================
  // Access to nodes
  // ===========================
  virtual SMDS_ElemIteratorPtr nodesIterator() const = 0;

  virtual SMDS_NodeIteratorPtr nodeIterator() const = 0;
  virtual SMDS_NodeIteratorPtr interlacedNodesIterator() const { return nodeIterator(); }
  virtual SMDS_NodeIteratorPtr nodesIteratorToUNV() const  { return nodeIterator(); }

  // std-like iteration on nodes
  typedef SMDS_StdIterator< const SMDS_MeshNode*, SMDS_NodeIteratorPtr > iterator;
  iterator begin_nodes() const { return iterator( nodeIterator() ); }
  iterator end_nodes()   const { return iterator(); }

  // ===========================
  // Type of element
  // ===========================
  virtual int NbNodes() const = 0;
  virtual int NbEdges() const = 0;
  virtual int NbFaces() const = 0;

  virtual SMDSAbs_ElementType  GetType() const = 0;
  virtual SMDSAbs_EntityType   GetEntityType() const = 0;
  virtual SMDSAbs_GeometryType GetGeomType() const = 0;
  virtual VTKCellType          GetVtkType() const = 0;

  virtual bool IsPoly() const = 0;
  virtual bool IsQuadratic() const = 0;
  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;
  virtual int  NbCornerNodes() const = 0;

  // ===========================
  //  Access to nodes by index
  // ===========================
  /*!
   * \brief Return node by its index
    * \param ind - node index
    * \retval const SMDS_MeshNode* - the node
   */
  virtual const SMDS_MeshNode* GetNode(const int ind) const = 0;

  /*!
   * \brief Return node by its index
    * \param ind - node index
    * \retval const SMDS_MeshNode* - the node
   * 
   * Index is wrapped if it is out of a valid range of corner nodes
   */
  const SMDS_MeshNode* GetNodeWrap(const int ind) const { return GetNode( WrappedIndex( ind )); }

  /*!
   * \brief Return true if index of node is valid (0 <= ind < NbNodes())
    * \param ind - node index
    * \retval bool - index check result
   */
  virtual bool IsValidIndex(const int ind) const;

  /*!
   * \brief Return a valid corner node index, fixing the given one if necessary
    * \param ind - node index
    * \retval int - valid node index
   */
  int WrappedIndex(const int ind) const;

  /*!
   * \brief Check if a node belongs to the element
    * \param node - the node to check
    * \retval int - node index within the element, -1 if not found
   */
  virtual int GetNodeIndex( const SMDS_MeshNode* node ) const;


  virtual smIdType  GetID() const;
  virtual vtkIdType GetVtkID()   const;
  virtual int  getshapeId() const { return GetShapeID(); }
  virtual int  GetShapeID() const;

  // mark this element; to be used in algos
  virtual void setIsMarked( bool is ) const;
  virtual bool isMarked() const;

  // element can be allocated but "not used"
  bool IsNull() const { return myHolder == 0; }

  SMDS_Mesh* GetMesh() const;

  void Print(std::ostream & OS) const;

  friend SMDS_EXPORT std::ostream & operator <<(std::ostream & OS, const SMDS_MeshElement *);
  friend class SMDS_ElementFactory;
  friend class SMESHDS_SubMesh;

  /*!
   * \brief Filters of elements, to be used with SMDS_SetIterator
   */
  struct Filter
  {
    virtual bool operator()(const SMDS_MeshElement* e) const = 0;
    virtual ~Filter() {}
  };
  struct NonNullFilter: public Filter
  {
    bool operator()(const SMDS_MeshElement* e) const { return e; }
  };
  struct TypeFilter : public Filter
  {
    SMDSAbs_ElementType _type;
    TypeFilter( SMDSAbs_ElementType t = SMDSAbs_NbElementTypes ):_type(t) {}
    bool operator()(const SMDS_MeshElement* e) const { return e && e->GetType() == _type; }
  };
  struct EntityFilter : public Filter
  {
    SMDSAbs_EntityType _type;
    EntityFilter( SMDSAbs_EntityType t = SMDSEntity_Last ):_type(t) {}
    bool operator()(const SMDS_MeshElement* e) const { return e && e->GetEntityType() == _type; }
  };
  struct GeomFilter : public Filter
  {
    SMDSAbs_GeometryType _type;
    GeomFilter( SMDSAbs_GeometryType t = SMDSGeom_NONE ):_type(t) {}
    bool operator()(const SMDS_MeshElement* e) const { return e && e->GetGeomType() == _type; }
  };

 protected:

  SMDS_MeshElement();

  void setVtkID(const vtkIdType vtkID );
  virtual void setShapeID( const int shapeID ) const;

  SMDS_UnstructuredGrid* getGrid() const;

 protected:

  SMDS_ElementChunk* myHolder;
};

// ============================================================
/*!
 * \brief Comparator of elements by ID for usage in std containers
 */
// ============================================================

struct TIDTypeCompare {
  bool operator () (const SMDS_MeshElement* e1, const SMDS_MeshElement* e2) const
  { return e1->GetType() == e2->GetType() ? e1->GetID() < e2->GetID() : e1->GetType() < e2->GetType(); }
};

// WARNING: this comparator makes impossible to store both nodes and elements in the same set
// because there are nodes and elements with the same ID. Use TIDTypeCompare for such containers.
struct TIDCompare {
  bool operator () (const SMDS_MeshElement* e1, const SMDS_MeshElement* e2) const
  { return e1->GetID() < e2->GetID(); }
};

#endif
