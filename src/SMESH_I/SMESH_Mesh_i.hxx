//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_Mesh_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MESH_I_HXX_
#define _SMESH_MESH_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(MED)

#include "SMESH_Hypothesis.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_subMesh.hxx"

#include "SALOME_GenericObj_i.hh"

class SMESH_Gen_i;
class SMESH_GroupBase_i;

#include <map>

class SMESH_Mesh_i:
  public virtual POA_SMESH::SMESH_Mesh,
  public virtual SALOME::GenericObj_i
{
  SMESH_Mesh_i();
  SMESH_Mesh_i(const SMESH_Mesh_i&);
public:
  SMESH_Mesh_i( PortableServer::POA_ptr thePOA,
                SMESH_Gen_i*            myGen_i,
	        CORBA::Long             studyId );

  virtual ~SMESH_Mesh_i();

  // --- CORBA
  void SetShape( GEOM::GEOM_Object_ptr theShapeObject )
    throw (SALOME::SALOME_Exception);

  GEOM::GEOM_Object_ptr GetShapeToMesh()
    throw (SALOME::SALOME_Exception);

  SMESH::Hypothesis_Status AddHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                         SMESH::SMESH_Hypothesis_ptr anHyp)
    throw (SALOME::SALOME_Exception);

  SMESH::Hypothesis_Status RemoveHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                            SMESH::SMESH_Hypothesis_ptr anHyp)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfHypothesis* GetHypothesisList(GEOM::GEOM_Object_ptr aSubShapeObject)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_subMesh_ptr GetSubMesh(GEOM::GEOM_Object_ptr aSubShapeObject, const char* theName)
    throw (SALOME::SALOME_Exception);

  void RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr CreateGroup( SMESH::ElementType theElemType, const char* theName )
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_GroupOnGeom_ptr CreateGroupFromGEOM(SMESH::ElementType    theElemType,
                                                   const char*           theName,
                                                   GEOM::GEOM_Object_ptr theGeomObj )
    throw (SALOME::SALOME_Exception);

  void RemoveGroup( SMESH::SMESH_GroupBase_ptr theGroup )
    throw (SALOME::SALOME_Exception);
  
  void RemoveGroupWithContents( SMESH::SMESH_GroupBase_ptr theGroup )
    throw (SALOME::SALOME_Exception);
  
  SMESH::ListOfGroups* GetGroups()
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr UnionGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                      SMESH::SMESH_GroupBase_ptr theGroup2, 
                                      const char* theName )
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_Group_ptr IntersectGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                          SMESH::SMESH_GroupBase_ptr theGroup2, 
                                          const char* theName )
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_Group_ptr CutGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                    SMESH::SMESH_GroupBase_ptr theGroup2, 
                                   const char* theName )
    throw (SALOME::SALOME_Exception);

//    SMESH::string_array* GetLog(CORBA::Boolean clearAfterGet)
//      throw (SALOME::SALOME_Exception);

  SMESH::log_array* GetLog(CORBA::Boolean clearAfterGet)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor_ptr GetMeshEditor();

  void ClearLog()
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetId()
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetStudyId()
    throw (SALOME::SALOME_Exception);

  // --- C++ interface
  void SetImpl(::SMESH_Mesh* impl);
  ::SMESH_Mesh& GetImpl();         // :: force no namespace here

  SMESH_Gen_i* GetGen() { return _gen_i; }

  int ImportUNVFile( const char* theFileName )
    throw (SALOME::SALOME_Exception);

  int ImportSTLFile( const char* theFileName )
    throw (SALOME::SALOME_Exception);

  /*!
   * consult DriverMED_R_SMESHDS_Mesh::ReadStatus for returned value
   */
  SMESH::DriverMED_ReadStatus ImportMEDFile( const char* theFileName, const char* theMeshName )
    throw (SALOME::SALOME_Exception);

  /*! Check group names for duplications.
   *  Consider maximum group name length stored in MED file.
   */
  CORBA::Boolean HasDuplicatedGroupNamesMED();

  void ExportToMED( const char* file, CORBA::Boolean auto_groups, SMESH::MED_VERSION theVersion )
    throw (SALOME::SALOME_Exception);
  void ExportMED( const char* file, CORBA::Boolean auto_groups )
    throw (SALOME::SALOME_Exception);

  void ExportDAT( const char* file )
    throw (SALOME::SALOME_Exception);
  void ExportUNV( const char* file )
    throw (SALOME::SALOME_Exception);
  void ExportSTL( const char* file, const bool isascii )
    throw (SALOME::SALOME_Exception);

  SALOME_MED::MESH_ptr GetMEDMesh()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbNodes()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbElements()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbEdges()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbEdgesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbFaces()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbFacesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTriangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTrianglesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbQuadrangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbQuadranglesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPolygons()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbVolumes()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbVolumesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTetras()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTetrasOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbHexas()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbHexasOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPyramids()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPyramidsOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPrisms()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPrismsOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPolyhedrons()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbSubMesh()
    throw (SALOME::SALOME_Exception);

  SMESH::long_array* GetElementsId()
    throw (SALOME::SALOME_Exception);

  SMESH::long_array* GetElementsByType( SMESH::ElementType theElemType )
    throw (SALOME::SALOME_Exception);
  
  SMESH::long_array* GetNodesId()
    throw (SALOME::SALOME_Exception);
  
  SMESH::ElementType GetElementType( const CORBA::Long id, const bool iselem )
    throw (SALOME::SALOME_Exception);
  
  /*!
   * Returns ID of elements for given submesh
   */
  SMESH::long_array* GetSubMeshElementsId(const CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);

  /*!
   * Returns ID of nodes for given submesh
   * If param all==true - returns all nodes, else -
   * returns only nodes on shapes.
   */
  SMESH::long_array* GetSubMeshNodesId(const CORBA::Long ShapeID, CORBA::Boolean all)
    throw (SALOME::SALOME_Exception);
  
  /*!
   * Returns type of elements for given submesh
   */
  SMESH::ElementType GetSubMeshElementType(const CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);
  
  char* Dump();
  
  // Internal methods not available through CORBA
  // They are called by corresponding interface methods
  SMESH_Hypothesis::Hypothesis_Status addHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                                    SMESH::SMESH_Hypothesis_ptr anHyp);

  SMESH_Hypothesis::Hypothesis_Status removeHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                                       SMESH::SMESH_Hypothesis_ptr anHyp);
  
  int importMEDFile( const char* theFileName, const char* theMeshName );

  SMESH::SMESH_subMesh_ptr createSubMesh( GEOM::GEOM_Object_ptr theSubShapeObject );

  void removeSubMesh(SMESH::SMESH_subMesh_ptr theSubMesh,
                     GEOM::GEOM_Object_ptr theSubShapeObject );

  SMESH::SMESH_GroupBase_ptr createGroup(SMESH::ElementType  theElemType,
                                         const char*         theName,
                                         const TopoDS_Shape& theShape = TopoDS_Shape());

  void removeGroup( const int theId );

  SMESH::SMESH_subMesh_ptr getSubMesh(int shapeID);
  // return an existing subMesh object for the shapeID. shapeID == submeshID.

  const map<int, SMESH::SMESH_GroupBase_ptr>& getGroups() { return _mapGroups; }
  // return an existing group object.

  virtual SMESH::long_array* GetIDs();

  CORBA::Long GetMeshPtr();


  /*!
   * Get XYZ coordinates of node as list of double
   * If there is not node for given ID - returns empty list
   */
  SMESH::double_array* GetNodeXYZ(const CORBA::Long id);
  
  /*!
   * For given node returns list of IDs of inverse elements
   * If there is not node for given ID - returns empty list
   */
  SMESH::long_array* GetNodeInverseElements(const CORBA::Long id);

  /*!
   * If given element is node returns IDs of shape from position
   * else - return ID of result shape after ::FindShape()
   * from SMESH_MeshEditor
   * If there is not element for given ID - returns -1
   */
  CORBA::Long GetShapeID(const CORBA::Long id);

  /*!
   * Returns number of nodes for given element
   * If there is not element for given ID - returns -1
   */
  CORBA::Long GetElemNbNodes(const CORBA::Long id);

  /*!
   * Returns ID of node by given index for given element
   * If there is not element for given ID - returns -1
   * If there is not node for given index - returns -2
   */
  CORBA::Long GetElemNode(const CORBA::Long id, const CORBA::Long index);
  
  /*!
   * Returns true if given node is medium node
   * in given quadratic element
   */
  CORBA::Boolean IsMediumNode(const CORBA::Long ide, const CORBA::Long idn);
  
  /*!
   * Returns true if given node is medium node
   * in one of quadratic elements
   */
  CORBA::Boolean IsMediumNodeOfAnyElem(const CORBA::Long idn,
                                       SMESH::ElementType theElemType);
  
  /*!
   * Returns number of edges for given element
   */
  CORBA::Long ElemNbEdges(const CORBA::Long id);
  
  /*!
   * Returns number of faces for given element
   */
  CORBA::Long ElemNbFaces(const CORBA::Long id);
  
  /*!
   * Returns true if given element is polygon
   */
  CORBA::Boolean IsPoly(const CORBA::Long id);
  
  /*!
   * Returns true if given element is quadratic
   */
  CORBA::Boolean IsQuadratic(const CORBA::Long id);
  
  /*!
   * Returns bary center for given element
   */
  SMESH::double_array* BaryCenter(const CORBA::Long id);


  map<int, SMESH_subMesh_i*> _mapSubMesh_i; //NRI
  map<int, ::SMESH_subMesh*> _mapSubMesh;   //NRI

private:
  void CreateGroupServants();

  static int myIdGenerator;
  ::SMESH_Mesh* _impl;  // :: force no namespace here
  SMESH_Gen_i* _gen_i;
  int _id;          // id given by creator (unique within the creator instance)
  int _studyId;
  map<int, SMESH::SMESH_subMesh_ptr>    _mapSubMeshIor;
  map<int, SMESH::SMESH_GroupBase_ptr>  _mapGroups;
  map<int, SMESH::SMESH_Hypothesis_ptr> _mapHypo;
};

#endif

