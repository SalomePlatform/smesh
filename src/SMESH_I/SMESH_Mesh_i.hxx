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
                SMESH_Gen_i*            myGen_i );

  virtual ~SMESH_Mesh_i();

  // --- CORBA

  void SetShape( GEOM::GEOM_Object_ptr theShapeObject );

  CORBA::Boolean HasShapeToMesh();

  GEOM::GEOM_Object_ptr GetShapeToMesh();

  virtual void ReplaceShape(GEOM::GEOM_Object_ptr theNewGeom);

  CORBA::Boolean IsLoaded();

  void Load();

  void Clear();

  void ClearSubMesh(CORBA::Long ShapeID);

  SMESH::Hypothesis_Status AddHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                         SMESH::SMESH_Hypothesis_ptr anHyp,
                                         CORBA::String_out           anErrorText);

  SMESH::Hypothesis_Status RemoveHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                            SMESH::SMESH_Hypothesis_ptr anHyp);

  SMESH::ListOfHypothesis* GetHypothesisList(GEOM::GEOM_Object_ptr aSubShape);

  SMESH::submesh_array* GetSubMeshes();

  SMESH::SMESH_subMesh_ptr GetSubMesh(GEOM::GEOM_Object_ptr aSubShape, const char* theName);

  void RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh );

  SMESH::SMESH_Group_ptr CreateGroup( SMESH::ElementType theElemType, const char* theName );

  SMESH::SMESH_GroupOnGeom_ptr CreateGroupFromGEOM(SMESH::ElementType    theElemType,
                                                   const char*           theName,
                                                   GEOM::GEOM_Object_ptr theGeomObj );

  SMESH::SMESH_GroupOnFilter_ptr CreateGroupFromFilter(SMESH::ElementType theElemType,
                                                       const char*        theName,
                                                       SMESH::Filter_ptr  theFilter );

  void RemoveGroup( SMESH::SMESH_GroupBase_ptr theGroup );

  void RemoveGroupWithContents( SMESH::SMESH_GroupBase_ptr theGroup );

  SMESH::ListOfGroups* GetGroups();

  CORBA::Long NbGroups();

  SMESH::SMESH_Group_ptr UnionGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                      SMESH::SMESH_GroupBase_ptr theGroup2,
                                      const char* theName );

  SMESH::SMESH_Group_ptr UnionListOfGroups( const SMESH::ListOfGroups& theGroups,
                                            const char* theName);

  SMESH::SMESH_Group_ptr IntersectGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                          SMESH::SMESH_GroupBase_ptr theGroup2,
                                          const char* theName );

  SMESH::SMESH_Group_ptr IntersectListOfGroups( const SMESH::ListOfGroups&  theGroups,
                                                const char* theName );

  SMESH::SMESH_Group_ptr CutGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                    SMESH::SMESH_GroupBase_ptr theGroup2,
                                    const char* theName );

  SMESH::SMESH_Group_ptr CutListOfGroups( const SMESH::ListOfGroups& theMainGroups,
                                          const SMESH::ListOfGroups& theToolGroups,
                                          const char* theName );

  SMESH::SMESH_Group_ptr CreateDimGroup( const SMESH::ListOfIDSources& theGroups,
                                         SMESH::ElementType            theElemType,
                                         const char*                   theName,
                                         SMESH::NB_COMMON_NODES_ENUM   theNbCommonNodes,
                                         CORBA::Boolean                theUnderlyingOnly );

  SMESH::ListOfGroups* FaceGroupsSeparatedByEdges( CORBA::Double  theSharpAngle,
                                                   CORBA::Boolean theCreateEdges,
                                                   CORBA::Boolean theUseExistingEdges );

  SMESH::SMESH_Group_ptr ConvertToStandalone( SMESH::SMESH_GroupBase_ptr theGroupOn );

  SMESH::log_array* GetLog(CORBA::Boolean clearAfterGet);

  SMESH::SMESH_MeshEditor_ptr GetMeshEditor();

  SMESH::SMESH_MeshEditor_ptr GetMeshEditPreviewer();

  CORBA::Boolean HasModificationsToDiscard();

  void ClearLog();

  CORBA::Long GetId();

  // --- C++ interface

  void SetImpl(::SMESH_Mesh* impl);
  ::SMESH_Mesh& GetImpl();         // :: force no namespace here

  SMESH_Gen_i* GetGen() { return _gen_i; }

  int ImportUNVFile( const char* theFileName );

  int ImportSTLFile( const char* theFileName );

  SMESH::ComputeError* ImportGMFFile( const char* theFileName,
                                      bool        theMakeRequiredGroups);

  /*!
   * consult DriverMED_R_SMESHDS_Mesh::ReadStatus for returned value
   */
  SMESH::DriverMED_ReadStatus ImportMEDFile( const char* theFileName, const char* theMeshName );

  SMESH::DriverMED_ReadStatus ImportCGNSFile( const char*  theFileName,
                                              const int    theMeshIndex,
                                              std::string& theMeshName);

  /*!
   *  Auto color
   */
  void SetAutoColor(CORBA::Boolean theAutoColor);

  CORBA::Boolean GetAutoColor();

  /*! Check group names for duplications.
   *  Consider maximum group name length stored in MED file.
   */
  CORBA::Boolean HasDuplicatedGroupNamesMED();

  /*!
   * Return string representation of a MED file version comprising nbDigits
   */
  char* GetVersionString(CORBA::Long minor, CORBA::Short nbDigits);

  /*!
   *  Return the list of med versions compatibles for write/append,
   *  encoded in 10*major+minor (for instance, code for med 3.2.1 is 32)
   */
  SMESH::long_array* GetMEDVersionsCompatibleForAppend();

  void ExportMED( const char*        file,
                  CORBA::Boolean     auto_groups,
                  CORBA::Long        version,
                  CORBA::Boolean     overwrite,
                  CORBA::Boolean     autoDimension = true);

  void ExportSAUV( const char* file, CORBA::Boolean auto_groups );

  void ExportDAT( const char* file );
  void ExportUNV( const char* file );
  void ExportSTL( const char* file, bool isascii );
  void ExportCGNS(SMESH::SMESH_IDSource_ptr meshPart,
                  const char*               file,
                  CORBA::Boolean            overwrite,
                  CORBA::Boolean            groupElemsByType);
  void ExportGMF(SMESH::SMESH_IDSource_ptr meshPart,
                 const char*               file,
                 CORBA::Boolean            withRequiredGroups);

  void ExportPartToMED(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file,
                       CORBA::Boolean            auto_groups,
                       CORBA::Long               version,
                       CORBA::Boolean            overwrite,
                       CORBA::Boolean            autoDim,
                       const GEOM::ListOfFields& fields,
                       const char*               geomAssocFields,
                       CORBA::Double             ZTolerance);
  void ExportPartToDAT(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file);
  void ExportPartToUNV(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file);
  void ExportPartToSTL(SMESH::SMESH_IDSource_ptr meshPart,
                       const char*               file,
                       CORBA::Boolean            isascii);

  CORBA::Double GetComputeProgress();

  SMESH::smIdType NbNodes();

  SMESH::smIdType NbElements();

  SMESH::smIdType Nb0DElements();

  SMESH::smIdType NbBalls();

  SMESH::smIdType NbEdges();

  SMESH::smIdType NbEdgesOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbFaces();

  SMESH::smIdType NbFacesOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbTriangles();

  SMESH::smIdType NbTrianglesOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbBiQuadTriangles();

  SMESH::smIdType NbQuadrangles();

  SMESH::smIdType NbQuadranglesOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbBiQuadQuadrangles();

  SMESH::smIdType NbPolygons();

  SMESH::smIdType NbPolygonsOfOrder(SMESH::ElementOrder order = SMESH::ORDER_ANY);

  SMESH::smIdType NbVolumes();

  SMESH::smIdType NbVolumesOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbTetras();

  SMESH::smIdType NbTetrasOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbHexas();

  SMESH::smIdType NbHexasOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbTriQuadraticHexas();

  SMESH::smIdType NbPyramids();

  SMESH::smIdType NbPyramidsOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbPrisms();

  SMESH::smIdType NbPrismsOfOrder(SMESH::ElementOrder order);

  SMESH::smIdType NbHexagonalPrisms();

  SMESH::smIdType NbPolyhedrons();

  SMESH::smIdType NbSubMesh();

  SMESH::smIdType_array* GetElementsId();

  SMESH::smIdType_array* GetElementsByType( SMESH::ElementType theElemType );

  SMESH::smIdType_array* GetNodesId();

  SMESH::ElementType GetElementType( SMESH::smIdType id, bool iselem );

  SMESH::EntityType GetElementGeomType( SMESH::smIdType id );

  SMESH::GeometryType GetElementShape( SMESH::smIdType id );

  /*!
   * Returns ID of elements for given submesh
   */
  SMESH::smIdType_array* GetSubMeshElementsId(CORBA::Long ShapeID);

  /*!
   * Returns ID of nodes for given submesh
   * If param all==true - returns all nodes, else -
   * returns only nodes on shapes.
   */
  SMESH::smIdType_array* GetSubMeshNodesId(CORBA::Long ShapeID, CORBA::Boolean all);

  /*!
   * Returns type of elements for given submesh
   */
  SMESH::ElementType GetSubMeshElementType(CORBA::Long ShapeID);

  char* Dump();

  // Create groups of elements preventing computation of a sub-shape
  SMESH::ListOfGroups* MakeGroupsOfBadInputElements( int         theSubShapeID,
                                                     const char* theGroupName);

  // ===================================================
  // Internal methods not available through CORBA
  // They are called by corresponding interface methods
  // ===================================================

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
                                         const int                 theID = -1,
                                         const TopoDS_Shape&       theShape = TopoDS_Shape(),
                                         const SMESH_PredicatePtr& thePred = SMESH_PredicatePtr());

  void removeGroup( const int theId );

  SMESH::SMESH_subMesh_ptr getSubMesh(int shapeID);
  // return an existing subMesh object for the shapeID. shapeID == submeshID.

  const std::map<int, SMESH::SMESH_GroupBase_ptr>& getGroups() { return _mapGroups; }
  // return existing group objects

  void onHypothesisModified(int theHypID, bool updateIcons);
  // callback from _impl to forget not loaded mesh data (issue 0021208)

  /*!
   * \brief Update data if geometry changes
   *
   * Issue 0022501
   */
  void CheckGeomModif( bool isBreakLink = false );
  /*!
   * \brief Update hypotheses assigned to geom groups if the latter change
   *
   * NPAL16168: "geometrical group edition from a submesh don't modify mesh computation"
   */
  void CheckGeomGroupModif();

  CORBA::LongLong GetMeshPtr();

  /*!
   * \brief Assure that all groups are published
   */
  void CreateGroupServants();

  /*!
   * \brief Return true if all sub-meshes are computed OK - to update an icon
   */
  bool IsComputedOK();


  // ====================================
  // SMESH_Mesh interface (continuation)
  // ====================================

  /*!
   * \brief Return groups cantained in _mapGroups by their IDs
   */
  SMESH::ListOfGroups* GetGroups(const std::list<int>& groupIDs) const;

  /*!
   * Get XYZ coordinates of node as list of double
   * If there is not node for given ID - returns empty list
   */
  SMESH::double_array* GetNodeXYZ(SMESH::smIdType id);

  /*!
   * For given node returns list of IDs of inverse elements
   * If there is not node for given ID - returns empty list
   */
  SMESH::smIdType_array* GetNodeInverseElements(SMESH::smIdType    id,
                                            SMESH::ElementType elemType);

  /*!
   * \brief Return position of a node on shape
   */
  SMESH::NodePosition* GetNodePosition(SMESH::smIdType NodeID);

  /*!
   * \brief Return position of an element on shape
   */
  SMESH::ElementPosition GetElementPosition(SMESH::smIdType ElemID);

  /*!
   * If given element is node returns IDs of shape from position
   * If there is not node for given ID - returns -1
   */
  CORBA::Long GetShapeID(SMESH::smIdType id);

  /*!
   * For given element returns ID of result shape after
   * ::FindShape() from SMESH_MeshEditor
   * If there is not element for given ID - returns -1
   */
  CORBA::Long GetShapeIDForElem(SMESH::smIdType id);

  /*!
   * Returns number of nodes for given element
   * If there is not element for given ID - returns -1
   */
  CORBA::Short GetElemNbNodes(SMESH::smIdType id);

  /*!
   * Returns IDs of nodes of given element
   */
  SMESH::smIdType_array* GetElemNodes(SMESH::smIdType id);

  /*!
   * Returns ID of node by given index for given element
   * If there is not element for given ID - returns -1
   * If there is not node for given index - returns -2
   */
  SMESH::smIdType GetElemNode(SMESH::smIdType id, CORBA::Short index);

  /*!
   * Returns true if given node is medium node
   * in given quadratic element
   */
  CORBA::Boolean IsMediumNode(SMESH::smIdType ide, SMESH::smIdType idn);

  /*!
   * Returns true if given node is medium node
   * in one of quadratic elements
   */
  CORBA::Boolean IsMediumNodeOfAnyElem(SMESH::smIdType    idn,
                                       SMESH::ElementType elemType);

  /*!
   * Returns number of edges for given element
   */
  CORBA::Long ElemNbEdges(SMESH::smIdType id);

  /*!
   * Returns number of faces for given element
   */
  CORBA::Long ElemNbFaces(SMESH::smIdType id);
  /*!
   * Returns nodes of given face (counted from zero) for given element.
   */
  SMESH::smIdType_array* GetElemFaceNodes(SMESH::smIdType elemId, CORBA::Short faceIndex);

  /*!
   * Returns three components of normal of given mesh face (or an empty array in KO case)
   */
  SMESH::double_array* GetFaceNormal(CORBA::Long faceId, CORBA::Boolean normalized);

  /*!
   * Returns an element based on all given nodes.
   */
  SMESH::smIdType FindElementByNodes(const SMESH::smIdType_array& nodes);

  /*!
   * Return elements including all given nodes.
   */
  SMESH::smIdType_array* GetElementsByNodes(const SMESH::smIdType_array& nodes,
                                        SMESH::ElementType       elemType);

  /*!
   * Returns true if given element is polygon
   */
  CORBA::Boolean IsPoly(SMESH::smIdType id);

  /*!
   * Returns true if given element is quadratic
   */
  CORBA::Boolean IsQuadratic(SMESH::smIdType id);

  /*!
   * Returns diameter of ball discrete element or zero in case of an invalid \a id
   */
  CORBA::Double GetBallDiameter(SMESH::smIdType id);

  /*!
   * Returns bary center for given element
   */
  SMESH::double_array* BaryCenter(SMESH::smIdType id);

  /*!
   * Returns information about imported MED file
   */
  virtual SMESH::MedFileInfo* GetMEDFileInfo();

  /*!
   * Persistence of file info
   */
  std::string FileInfoToString();
  void FileInfoFromString(const std::string& info);

  /*!
   * Persistence of geometry tick
   */
  int& MainShapeTick() { return _mainShapeTick; }
  

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
                              SMESH::smIdType_array&     theInfo);
  /*!
   * \brief Return iterator on elements of given type in given object
   */
  static SMDS_ElemIteratorPtr GetElements(SMESH::SMESH_IDSource_ptr obj,
                                          SMESH::ElementType        type);

  // =========================
  // SMESH_IDSource interface
  // =========================

  virtual SMESH::smIdType_array* GetIDs();
  /*!
   * Returns number of mesh elements of each \a EntityType
   * Result array of number of elements per \a EntityType
   * Inherited from SMESH_IDSource
   */
  virtual SMESH::smIdType_array* GetMeshInfo();
  /*!
   * Returns number of mesh elements of each \a ElementType
   */
  virtual SMESH::smIdType_array* GetNbElementsByType();
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
  int           _nbInvalidHypos;
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
  TopoDS_Shape newGroupShape( TGeomGroupData & groupData, int how );

};

#endif
