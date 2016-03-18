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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : SMESH_Group_i.hxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//
#ifndef SMESH_Group_i_HeaderFile
#define SMESH_Group_i_HeaderFile

#include "SMESH.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_Filter_i.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include "SALOME_GenericObj_i.hh"

class SMESH_Group;
class SMESHDS_GroupBase;
class SMESH_PreMeshInfo;

// ===========
// Group Base
// ===========
class SMESH_I_EXPORT SMESH_GroupBase_i:
  public virtual POA_SMESH::SMESH_GroupBase,
  public virtual SALOME::GenericObj_i,
  public SMESH::NotifyerAndWaiter // defined in SMESH_Filter_i.hxx
{
 public:
  SMESH_GroupBase_i(PortableServer::POA_ptr thePOA,
                    SMESH_Mesh_i*           theMeshServant,
                    const int               theLocalID );
  virtual ~SMESH_GroupBase_i();

  // CORBA interface implementation
  void SetName(const char* name);
  char* GetName();
  SMESH::ElementType GetType();
  CORBA::Long Size();
  CORBA::Boolean IsEmpty();
  CORBA::Boolean Contains(CORBA::Long elem_id);
  CORBA::Long GetID(CORBA::Long elem_index);
  SMESH::long_array* GetListOfID();
  SMESH::long_array* GetNodeIDs();
  CORBA::Long GetNumberOfNodes();
  CORBA::Boolean IsNodeInfoAvailable(); // for gui

  virtual SMESH::SMESH_Mesh_ptr GetMesh();

  /*!
   * Returns number of mesh elements of each \a EntityType
   * Result array of number of elements per \a EntityType
   * Inherited from SMESH_IDSource
   */
  virtual SMESH::long_array* GetMeshInfo();
  /*!
   * Returns number of mesh elements of each \a ElementType
   */
  virtual SMESH::long_array* GetNbElementsByType();
  /*!
   * Returns a sequence of all element IDs
   */
  virtual SMESH::long_array* GetIDs();
  /*!
   * Returns types of elements it contains
   * Inherited from SMESH_IDSource interface
   */
  virtual SMESH::array_of_ElementType* GetTypes();
  /*!
   * Returns false if GetMeshInfo() returns incorrect information that may
   * happen if mesh data is not yet fully loaded from the file of study.
   */
  virtual bool IsMeshInfoCorrect();
  /*!
   * Returns mesh unstructed grid information.
   */
  virtual SALOMEDS::TMPFile* GetVtkUgStream();

  /*!
   * Returns \c true if \c this group depends on the \a other via
   * FT_BelongToMeshGroup predicate or vice versa
   */
  virtual CORBA::Boolean IsInDependency( SMESH::SMESH_GroupBase_ptr other );


  // Internal C++ interface
  int GetLocalID() const { return myLocalID; }
  SMESH_Mesh_i* GetMeshServant() const { return myMeshServant; }
  SMESH_Group* GetSmeshGroup() const;
  SMESHDS_GroupBase* GetGroupDS() const;

  void SetColor(const SALOMEDS::Color& color);
  SALOMEDS::Color GetColor();

  void SetColorNumber(CORBA::Long color);
  CORBA::Long GetColorNumber();

protected:

  SMESH_PreMeshInfo* & changePreMeshInfo() { return myPreMeshInfo; }
  SMESH_PreMeshInfo* myPreMeshInfo; // mesh info before full loading from study file
  friend class SMESH_PreMeshInfo;

  int myNbNodes, myGroupDSTic;

private:
  SMESH_Mesh_i* myMeshServant;
  int myLocalID;

  void changeLocalId(int localId) { myLocalID = localId; }
  friend class SMESH_Mesh_i;
};

// ======
// Group
// ======

class SMESH_I_EXPORT SMESH_Group_i:
  public virtual POA_SMESH::SMESH_Group,
  public SMESH_GroupBase_i
{
 public:
  SMESH_Group_i( PortableServer::POA_ptr thePOA,
                 SMESH_Mesh_i*           theMeshServant,
                 const int               theLocalID );
  // CORBA interface implementation
  void Clear();
  CORBA::Long Add( const SMESH::long_array& theIDs );
  CORBA::Long Remove( const SMESH::long_array& theIDs );

  CORBA::Long AddByPredicate( SMESH::Predicate_ptr thePredicate );
  CORBA::Long RemoveByPredicate( SMESH::Predicate_ptr thePredicate );

  CORBA::Long AddFrom( SMESH::SMESH_IDSource_ptr theSource );
};

// =========================
// Group linked to geometry
// =========================

class SMESH_I_EXPORT SMESH_GroupOnGeom_i:
  public virtual POA_SMESH::SMESH_GroupOnGeom,
  public SMESH_GroupBase_i
{
 public:
  SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA,
                       SMESH_Mesh_i*           theMeshServant,
                       const int               theLocalID );
  // CORBA interface implementation
  GEOM::GEOM_Object_ptr GetShape();
};

// =========================
// Group deined by filter
// =========================

class SMESH_I_EXPORT SMESH_GroupOnFilter_i:
  public virtual POA_SMESH::SMESH_GroupOnFilter,
  public SMESH_GroupBase_i
{
 public:
  SMESH_GroupOnFilter_i( PortableServer::POA_ptr thePOA,
                         SMESH_Mesh_i*           theMeshServant,
                         const int               theLocalID );
  ~SMESH_GroupOnFilter_i();

  // Persistence
  static SMESH::Filter_ptr StringToFilter(const std::string& thePersistentString );
  std::string FilterToString() const;

  static SMESH_PredicatePtr GetPredicate( SMESH::Filter_ptr );

  // CORBA interface implementation
  void SetFilter(SMESH::Filter_ptr theFilter) throw (SALOME::SALOME_Exception);
  SMESH::Filter_ptr GetFilter();
  virtual SMESH::long_array* GetListOfID();
  virtual SMESH::long_array* GetMeshInfo();

  // method of SMESH::NotifyerAndWaiter to update self when myFilter changes
  virtual void OnBaseObjModified(NotifyerAndWaiter* filter, bool);

 private:
  SMESH::Filter_var myFilter;
};
#endif
