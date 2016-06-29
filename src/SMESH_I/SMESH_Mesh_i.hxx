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
//  File   : SMESH_Mesh_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#ifndef _SMESH_MESH_I_HXX_
#define _SMESH_MESH_I_HXX_

#include "SMESH.hxx"

#include "SMESH_Hypothesis.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"
#include "SMDS_ElemIterator.hxx"

#include <SALOME_GenericObj_i.hh>
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include <map>

class SMESH_Gen_i;
class SMESH_GroupBase_i;
class SMESH_subMesh_i;
class SMESH_PreMeshInfo;
class SMESH_MeshEditor_i;
class DriverMED_W_Field;
class SMESHDS_Mesh;

class SMESH_I_EXPORT SMESH_Mesh_i:
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

  CORBA::Boolean HasShapeToMesh()
    throw (SALOME::SALOME_Exception);

  GEOM::GEOM_Object_ptr GetShapeToMesh()
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean IsLoaded()
    throw (SALOME::SALOME_Exception);

  void Load()
    throw (SALOME::SALOME_Exception);

  void Clear()
    throw (SALOME::SALOME_Exception);

  void ClearSubMesh(CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);

  SMESH::Hypothesis_Status AddHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                         SMESH::SMESH_Hypothesis_ptr anHyp,
                                         CORBA::String_out           anErrorText)
    throw (SALOME::SALOME_Exception);

  SMESH::Hypothesis_Status RemoveHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                            SMESH::SMESH_Hypothesis_ptr anHyp)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfHypothesis* GetHypothesisList(GEOM::GEOM_Object_ptr aSubShape)
    throw (SALOME::SALOME_Exception);

  SMESH::submesh_array* GetSubMeshes()
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_subMesh_ptr GetSubMesh(GEOM::GEOM_Object_ptr aSubShape, const char* theName)
    throw (SALOME::SALOME_Exception);

  void RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr CreateGroup( SMESH::ElementType theElemType, const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_GroupOnGeom_ptr CreateGroupFromGEOM(SMESH::ElementType    theElemType,
                                                   const char*           theName,
                                                   GEOM::GEOM_Object_ptr theGeomObj )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_GroupOnFilter_ptr CreateGroupFromFilter(SMESH::ElementType theElemType,
                                                       const char*        theName,
                                                       SMESH::Filter_ptr  theFilter )
    throw (SALOME::SALOME_Exception);

  void RemoveGroup( SMESH::SMESH_GroupBase_ptr theGroup )
    throw (SALOME::SALOME_Exception);

  void RemoveGroupWithContents( SMESH::SMESH_GroupBase_ptr theGroup )
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* GetGroups()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbGroups()
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr UnionGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                      SMESH::SMESH_GroupBase_ptr theGroup2,
                                      const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr UnionListOfGroups( const SMESH::ListOfGroups& theGroups,
                                            const char* theName)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr IntersectGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                          SMESH::SMESH_GroupBase_ptr theGroup2,
                                          const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr IntersectListOfGroups( const SMESH::ListOfGroups&  theGroups,
                                                const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr CutGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                    SMESH::SMESH_GroupBase_ptr theGroup2,
                                    const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr CutListOfGroups( const SMESH::ListOfGroups& theMainGroups,
                                          const SMESH::ListOfGroups& theToolGroups,
                                          const char* theName )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr CreateDimGroup( const SMESH::ListOfIDSources& theGroups,
                                         SMESH::ElementType            theElemType,
                                         const char*                   theName,
                                         SMESH::NB_COMMON_NODES_ENUM   theNbCommonNodes,
                                         CORBA::Boolean                theUnderlyingOnly )
    throw (SALOME::SALOME_Exception);


  SMESH::SMESH_Group_ptr ConvertToStandalone( SMESH::SMESH_GroupBase_ptr theGroupOn )
    throw (SALOME::SALOME_Exception);

  SMESH::log_array* GetLog(CORBA::Boolean clearAfterGet)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor_ptr GetMeshEditor() throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor_ptr GetMeshEditPreviewer() throw (SALOME::SALOME_Exception);

  CORBA::Boolean HasModificationsToDiscard() throw (SALOME::SALOME_Exception);

  void ClearLog() throw (SALOME::SALOME_Exception);

  CORBA::Long GetId() throw (SALOME::SALOME_Exception);

  CORBA::Long GetStudyId() throw (SALOME::SALOME_Exception);

  // --- C++ interface

  void SetImpl(::SMESH_Mesh* impl);
  ::SMESH_Mesh& GetImpl();         // :: force no namespace here

  SMESH_Gen_i* GetGen() { return _gen_i; }

  int ImportUNVFile( const char* theFileName )
    throw (SALOME::SALOME_Exception);

  int ImportSTLFile( const char* theFileName )
    throw (SALOME::SALOME_Exception);

  SMESH::ComputeError* ImportGMFFile( const char* theFileName,
                                      bool        theMakeRequiredGroups)
    throw (SALOME::SALOME_Exception);

  /*!
   * consult DriverMED_R_SMESHDS_Mesh::ReadStatus for returned value
   */
  SMESH::DriverMED_ReadStatus ImportMEDFile( const char* theFileName, const char* theMeshName )
    throw (SALOME::SALOME_Exception);

  SMESH::DriverMED_ReadStatus ImportCGNSFile( const char*  theFileName,
                                              const int    theMeshIndex,
                                              std::string& theMeshName)
    throw (SALOME::SALOME_Exception);

  /*!
   *  Auto color
   */
  void SetAutoColor(CORBA::Boolean theAutoColor)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean GetAutoColor()
    throw (SALOME::SALOME_Exception);

  /*! Check group names for duplications.
   *  Consider maximum group name length stored in MED file.
   */
  CORBA::Boolean HasDuplicatedGroupNamesMED();
  /*!
   * Return string representation of a MED file version comprising nbDigits
   */
  char* GetVersionString(SMESH::MED_VERSION version, CORBA::Short nbDigits);

  void ExportToMEDX( const char*        file,
                     CORBA::Boolean     auto_groups,
                     SMESH::MED_VERSION version,
                     CORBA::Boolean     overwrite,
                     CORBA::Boolean     autoDimension=true) throw (SALOME::SALOME_Exception);
  void ExportToMED ( const char*        file,
                     CORBA::Boolean     auto_groups,
                     SMESH::MED_VERSION version ) throw (SALOME::SALOME_Exception);
  void ExportMED   ( const char*        file,
                     CORBA::Boolean     auto_groups ) throw (SALOME::SALOME_Exception);

  void ExportSAUV( const char* file, CORBA::Boolean auto_groups ) throw (SALOME::SALOME_Exception);

  void ExportDAT( const char* file ) throw (SALOME::SALOME_Exception);
  void ExportUNV( const char* file ) throw (SALOME::SALOME_Exception);
  void ExportSTL( const char* file, bool isascii ) throw (SALOME::SALOME_Exception);
  void ExportCGNS(SMESH::SMESH_IDSource_ptr meshPart,
                  const char*               file,
                  CORBA::Boolean            overwrite) throw (SALOME::SALOME_Exception);
  void ExportGMF(SMESH::SMESH_IDSource_ptr meshPart,
                 const char*               file,
                 CORBA::Boolean            withRequiredGroups) throw (SALOME::SALOME_Exception);

  void ExportPartToMED(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file,
                       CORBA::Boolean            auto_groups,
                       SMESH::MED_VERSION        version,
                       CORBA::Boolean            overwrite,
                       CORBA::Boolean            autoDim,
                       const GEOM::ListOfFields& fields,
                       const char*               geomAssocFields) throw (SALOME::SALOME_Exception);
  void ExportPartToDAT(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file) throw (SALOME::SALOME_Exception);
  void ExportPartToUNV(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file) throw (SALOME::SALOME_Exception);
  void ExportPartToSTL(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file,
                       CORBA::Boolean            isascii) throw (SALOME::SALOME_Exception);

  CORBA::Double GetComputeProgress();

  CORBA::Long NbNodes()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbElements()
    throw (SALOME::SALOME_Exception);

  CORBA::Long Nb0DElements()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbBalls()
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

  CORBA::Long NbBiQuadTriangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbQuadrangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbQuadranglesOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbBiQuadQuadrangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPolygons()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPolygonsOfOrder(SMESH::ElementOrder order=SMESH::ORDER_ANY)
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

  CORBA::Long NbTriQuadraticHexas()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPyramids()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPyramidsOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPrisms()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbPrismsOfOrder(SMESH::ElementOrder order)
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbHexagonalPrisms()
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

  SMESH::ElementType GetElementType( CORBA::Long id, bool iselem )
    throw (SALOME::SALOME_Exception);

  SMESH::EntityType GetElementGeomType( CORBA::Long id )
    throw (SALOME::SALOME_Exception);

  SMESH::GeometryType GetElementShape( CORBA::Long id )
    throw (SALOME::SALOME_Exception);

  /*!
   * Returns ID of elements for given submesh
   */
  SMESH::long_array* GetSubMeshElementsId(CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);

  /*!
   * Returns ID of nodes for given submesh
   * If param all==true - returns all nodes, else -
   * returns only nodes on shapes.
   */
  SMESH::long_array* GetSubMeshNodesId(CORBA::Long ShapeID, CORBA::Boolean all)
    throw (SALOME::SALOME_Exception);

  /*!
   * Returns type of elements for given submesh
   */
  SMESH::ElementType GetSubMeshElementType(CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);

  char* Dump();

  // Create groups of elements preventing computation of a sub-shape
  SMESH::ListOfGroups* MakeGroupsOfBadInputElements( int         theSubShapeID,
                                                     const char* theGroupName)
    throw (SALOME::SALOME_Exception);

  // Internal methods not available through CORBA
  // They are called by corresponding interface methods
  SMESH_Hypothesis::Hypothesis_Status addHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                                    SMESH::SMESH_Hypothesis_ptr anHyp,
                                                    std::string*                anErrorText=0);

  SMESH_Hypothesis::Hypothesis_Status removeHypothesis(GEOM::GEOM_Object_ptr aSubShape,
                                                       SMESH::SMESH_Hypothesis_ptr anHyp);

  static SMESH::Hypothesis_Status
  ConvertHypothesisStatus (SMESH_Hypothesis::Hypothesis_Status theStatus);

  static void PrepareForWriting (const char* file, bool overwrite = true);

  //int importMEDFile( const char* theFileName, const char* theMeshName );

  SMESH::SMESH_subMesh_ptr createSubMesh( GEOM::GEOM_Object_ptr theSubShapeObject );

  bool removeSubMesh(SMESH::SMESH_subMesh_ptr theSubMesh,
                     GEOM::GEOM_Object_ptr theSubShapeObject );

  SMESH::SMESH_GroupBase_ptr createGroup(SMESH::ElementType        theElemType,
                                         const char*               theName,
                                         const TopoDS_Shape&       theShape = TopoDS_Shape(),
                                         const SMESH_PredicatePtr& thePred = SMESH_PredicatePtr());

  void removeGroup( const int theId );

  SMESH::SMESH_subMesh_ptr getSubMesh(int shapeID);
  // return an existing subMesh object for the shapeID. shapeID == submeshID.

  const std::map<int, SMESH::SMESH_GroupBase_ptr>& getGroups() { return _mapGroups; }
  // return existing group objects

  void onHypothesisModified();
  // callback from _impl to forget not loaded mesh data (issue 0021208)

  void checkMeshLoaded();

  /*!
   * \brief Update data if geometry changes
   *
   * Issue 0022501
   */
  void CheckGeomModif();
  /*!
   * \brief Update hypotheses assigned to geom groups if the latter change
   *
   * NPAL16168: "geometrical group edition from a submesh don't modifiy mesh computation"
   */
  void CheckGeomGroupModif();

  CORBA::LongLong GetMeshPtr();

  /*!
   * \brief Assure that all groups are published
   */
  void CreateGroupServants();

  /*!
   * \brief Return groups cantained in _mapGroups by their IDs
   */
  SMESH::ListOfGroups* GetGroups(const std::list<int>& groupIDs) const;

  /*!
   * Get XYZ coordinates of node as list of double
   * If there is not node for given ID - returns empty list
   */
  SMESH::double_array* GetNodeXYZ(CORBA::Long id);

  /*!
   * For given node returns list of IDs of inverse elements
   * If there is not node for given ID - returns empty list
   */
  SMESH::long_array* GetNodeInverseElements(CORBA::Long id);

  /*!
   * \brief Return position of a node on shape
   */
  SMESH::NodePosition* GetNodePosition(CORBA::Long NodeID);

  /*!
   * \brief Return position of an element on shape
   */
  SMESH::ElementPosition GetElementPosition(CORBA::Long ElemID);

  /*!
   * If given element is node returns IDs of shape from position
   * If there is not node for given ID - returns -1
   */
  CORBA::Long GetShapeID(CORBA::Long id);

  /*!
   * For given element returns ID of result shape after
   * ::FindShape() from SMESH_MeshEditor
   * If there is not element for given ID - returns -1
   */
  CORBA::Long GetShapeIDForElem(CORBA::Long id);

  /*!
   * Returns number of nodes for given element
   * If there is not element for given ID - returns -1
   */
  CORBA::Long GetElemNbNodes(CORBA::Long id);

  /*!
   * Returns IDs of nodes of given element
   */
  SMESH::long_array* GetElemNodes(CORBA::Long id);

  /*!
   * Returns ID of node by given index for given element
   * If there is not element for given ID - returns -1
   * If there is not node for given index - returns -2
   */
  CORBA::Long GetElemNode(CORBA::Long id, CORBA::Long index);

  /*!
   * Returns true if given node is medium node
   * in given quadratic element
   */
  CORBA::Boolean IsMediumNode(CORBA::Long ide, CORBA::Long idn);

  /*!
   * Returns true if given node is medium node
   * in one of quadratic elements
   */
  CORBA::Boolean IsMediumNodeOfAnyElem(CORBA::Long idn,
                                       SMESH::ElementType theElemType);

  /*!
   * Returns number of edges for given element
   */
  CORBA::Long ElemNbEdges(CORBA::Long id);

  /*!
   * Returns number of faces for given element
   */
  CORBA::Long ElemNbFaces(CORBA::Long id);
  /*!
   * Returns nodes of given face (counted from zero) for given element.
   */
  SMESH::long_array* GetElemFaceNodes(CORBA::Long elemId, CORBA::Short faceIndex);

  /*!
   * Returns three components of normal of given mesh face (or an empty array in KO case)
   */
  SMESH::double_array* GetFaceNormal(CORBA::Long faceId, CORBA::Boolean normalized);

  /*!
   * Returns an element based on all given nodes.
   */
  CORBA::Long FindElementByNodes(const SMESH::long_array& nodes);

  /*!
   * Returns true if given element is polygon
   */
  CORBA::Boolean IsPoly(CORBA::Long id);

  /*!
   * Returns true if given element is quadratic
   */
  CORBA::Boolean IsQuadratic(CORBA::Long id);

  /*!
   * Returns diameter of ball discrete element or zero in case of an invalid \a id
   */
  CORBA::Double GetBallDiameter(CORBA::Long id);

  /*!
   * Returns bary center for given element
   */
  SMESH::double_array* BaryCenter(CORBA::Long id);

  /*!
   * Returns information about imported MED file
   */
  virtual SMESH::MedFileInfo* GetMEDFileInfo();

  /*!
   * Sets list of notebook variables used for Mesh operations separated by ":" symbol
   */
  void SetParameters (const char* theParameters);

  /*!
   * Returns list of notebook variables used for Mesh operations separated by ":" symbol
   */
  char* GetParameters();

  /*!
   * Returns list of notebook variables used for last Mesh operation
   */
  SMESH::string_array* GetLastParameters();

  /*!
   * \brief Return \c true if a meshing order not yet set for a concurrent sub-mesh
   */
  CORBA::Boolean IsUnorderedSubMesh(CORBA::Long submeshID);
  /*!
   * \brief Return submesh objects list in meshing order
   */
  virtual SMESH::submesh_array_array* GetMeshOrder();
  /*!
   * \brief Set submesh object order
   */
  virtual ::CORBA::Boolean SetMeshOrder(const SMESH::submesh_array_array& theSubMeshArray);


  /*!
   * Collect statistic of mesh elements given by iterator
   */
  static void CollectMeshInfo(const SMDS_ElemIteratorPtr theItr,
                              SMESH::long_array&         theInfo);
  /*!
   * \brief Return iterator on elements of given type in given object
   */
  static SMDS_ElemIteratorPtr GetElements(SMESH::SMESH_IDSource_ptr obj,
                                          SMESH::ElementType        type);

  // =========================
  // SMESH_IDSource interface
  // =========================

  virtual SMESH::long_array* GetIDs();
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
   * Returns types of elements it contains
   */
  virtual SMESH::array_of_ElementType* GetTypes();
  /*!
   * Returns self
   */
  virtual SMESH::SMESH_Mesh_ptr GetMesh();
  /*!
   * Returns false if GetMeshInfo() returns incorrect information that may
   * happen if mesh data is not yet fully loaded from the file of study.
   */
  bool IsMeshInfoCorrect();
   /*!
    * Returns mesh unstructed grid information.
    */
   virtual SALOMEDS::TMPFile* GetVtkUgStream();

  std::map<int, SMESH_subMesh_i*> _mapSubMesh_i; //NRI
  std::map<int, ::SMESH_subMesh*> _mapSubMesh;   //NRI

private:
  std::string prepareMeshNameAndGroups( const char* file, CORBA::Boolean overwrite );

  /*!
   * Check and correct names of mesh groups
   */
  void checkGroupNames();

  /*
   * Write GEOM fields to MED file
   */
  void exportMEDFields( DriverMED_W_Field &       writer,
                        SMESHDS_Mesh*             meshDS,
                        const GEOM::ListOfFields& fields,
                        const char*               geomAssocFields);
  /*!
   * Convert submesh ids into submesh interfaces
   */
  void convertMeshOrder(const TListOfListOfInt&     theIdsOrder,
                        SMESH::submesh_array_array& theSubMeshOrder,
                        const bool                  theIsDump);

  /*!
   * \brief Finds concurrent sub-meshes
   */
  TListOfListOfInt findConcurrentSubMeshes();

private:

  static int    _idGenerator;
  ::SMESH_Mesh* _impl;        // :: force no namespace here
  SMESH_Gen_i*  _gen_i;
  int           _id;          // id given by creator (unique within the creator instance)
  int           _studyId;
  std::map<int, SMESH::SMESH_subMesh_ptr>    _mapSubMeshIor;
  std::map<int, SMESH::SMESH_GroupBase_ptr>  _mapGroups;
  std::map<int, SMESH::SMESH_Hypothesis_ptr> _mapHypo;
  SMESH_MeshEditor_i*    _editor;
  SMESH_MeshEditor_i*    _previewEditor;
  SMESH::MedFileInfo_var _medFileInfo;
  SMESH_PreMeshInfo*     _preMeshInfo; // mesh info before full loading from study file

  SMESH_PreMeshInfo* & changePreMeshInfo() { return _preMeshInfo; }
  friend class SMESH_PreMeshInfo;

private:

  // Data used to track changes of GEOM groups
  struct TGeomGroupData {
    // keep study entry but not ior because GEOM_Object actually changes if
    // number of items in a group varies (1) <-> (>1)
    std::string       _groupEntry;
    std::set<int>     _indices; // indices of group items within group's main shape
    CORBA::Object_var _smeshObject; // SMESH object depending on GEOM group
  };
  std::list<TGeomGroupData> _geomGroupData;
  int                       _mainShapeTick; // to track modifications of the meshed shape

  /*!
   * Remember GEOM group data
   */
  void addGeomGroupData(GEOM::GEOM_Object_ptr theGeomObj,
                        CORBA::Object_ptr     theSmeshObj);
  /*!
   * Remove GEOM group data relating to removed smesh object
   */
  void removeGeomGroupData(CORBA::Object_ptr theSmeshObj);
  /*!
   * Return new group contents if it has been changed and update group data
   */
  TopoDS_Shape newGroupShape( TGeomGroupData & groupData);

};

#endif
