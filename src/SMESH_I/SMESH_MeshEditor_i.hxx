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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : SMESH_MeshEditor_i.hxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//
#ifndef _SMESH_MESHEDITOR_I_HXX_
#define _SMESH_MESHEDIOTR_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include "SMESH_Mesh.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_MeshEditor.hxx"
#include <list>
#include <string>

class SMESH_Mesh_i;

namespace MeshEditor_I {
  struct TPreviewMesh;
  struct ExtrusionParams;
}

class SMESH_I_EXPORT SMESH_MeshEditor_i: public POA_SMESH::SMESH_MeshEditor
{
public:
  SMESH_MeshEditor_i(SMESH_Mesh_i * theMesh, bool isPreview);

  virtual ~ SMESH_MeshEditor_i();
  /*!
   * \brief Return edited mesh ID
   * \retval int - mesh ID
   */
  int GetMeshId() const { return myMesh->GetId(); }

  SMESH::SMESH_Mesh_ptr GetMesh();

  // --- CORBA

  /*!
   * Return data of mesh edition preview
   */
  SMESH::MeshPreviewStruct* GetPreviewData();
  /*!
   * If during last operation of MeshEditor some nodes were
   * created this method returns list of their IDs, if new nodes
   * not created - returns an empty list
   */
  SMESH::smIdType_array* GetLastCreatedNodes();
  /*!
   * If during last operation of MeshEditor some elements were
   * created this method returns list of their IDs, if new elements
   * not created - returns an empty list
   */
  SMESH::smIdType_array* GetLastCreatedElems();
  /*!
   * \brief Clears sequences of last created elements and nodes 
   */
  void ClearLastCreated();
  /*!
   * \brief Returns description of an error/warning occurred during the last operation
   */
  SMESH::ComputeError* GetLastError();

  /*!
   * \brief Wrap a sequence of ids in a SMESH_IDSource
   */
  SMESH::SMESH_IDSource_ptr MakeIDSource(const SMESH::smIdType_array& IDsOfElements,
                                         SMESH::ElementType           type);
  static bool               IsTemporaryIDSource( SMESH::SMESH_IDSource_ptr& idSource );
  static SMESH::smIdType*       GetTemporaryIDs( SMESH::SMESH_IDSource_ptr& idSource, SMESH::smIdType& nbIds );

  /*!
   * \brief Generates the unique group name
   */
  std::string GenerateGroupName(const std::string& thePrefix);

  CORBA::Boolean RemoveElements(const SMESH::smIdType_array & IDsOfElements);
  CORBA::Boolean RemoveNodes   (const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType    RemoveOrphanNodes();

  /*!
   * Methods for creation new elements.
   * Returns ID of created element or 0 if element not created
   */
  SMESH::smIdType AddNode(CORBA::Double x, CORBA::Double y, CORBA::Double z);
  SMESH::smIdType Add0DElement(SMESH::smIdType IDOfNode, CORBA::Boolean DuplicateElements);
  SMESH::smIdType AddBall(SMESH::smIdType IDOfNodem, CORBA::Double diameter);
  SMESH::smIdType AddEdge(const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType AddFace(const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType AddPolygonalFace(const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType AddQuadPolygonalFace(const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType AddVolume(const SMESH::smIdType_array & IDsOfNodes);
  SMESH::smIdType AddPolyhedralVolume(const SMESH::smIdType_array & IDsOfNodes,
                                  const SMESH::long_array & Quantities);
  SMESH::smIdType AddPolyhedralVolumeByFaces(const SMESH::smIdType_array & IdsOfFaces);

  /*!
   * \brief Create 0D elements on all nodes of the given object except those 
   *        nodes on which a 0D element already exists.
   *  \param theObject object on whose nodes 0D elements will be created.
   *  \param theGroupName optional name of a group to add 0D elements created
   *         and/or found on nodes of \a theObject.
   *  \param theDuplicateElements to add one more 0D element to a node or not
   *  \return an object (a new group or a temporary SMESH_IDSource) holding
   *          ids of new and/or found 0D elements.
   */
  SMESH::SMESH_IDSource_ptr Create0DElementsOnAllNodes(SMESH::SMESH_IDSource_ptr theObject,
                                                       const char*               theGroupName,
                                                       CORBA::Boolean            theDuplicateElements);

  /*!
   * \brief Bind a node to a vertex
   * \param NodeID - node ID
   * \param VertexID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetNodeOnVertex(SMESH::smIdType NodeID, CORBA::Long VertexID);
  /*!
   * \brief Store node position on an edge
   * \param NodeID - node ID
   * \param EdgeID - edge ID available through GEOM_Object.GetSubShapeIndices()[0]
   * \param paramOnEdge - parameter on edge where the node is located
   */
  void SetNodeOnEdge(SMESH::smIdType NodeID, CORBA::Long EdgeID,
                     CORBA::Double paramOnEdge);
  /*!
   * \brief Store node position on a face
   * \param NodeID - node ID
   * \param FaceID - face ID available through GEOM_Object.GetSubShapeIndices()[0]
   * \param u - U parameter on face where the node is located
   * \param v - V parameter on face where the node is located
   */
  void SetNodeOnFace(SMESH::smIdType NodeID, CORBA::Long FaceID,
                     CORBA::Double u, CORBA::Double v);
  /*!
   * \brief Bind a node to a solid
   * \param NodeID - node ID
   * \param SolidID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetNodeInVolume(SMESH::smIdType NodeID, CORBA::Long SolidID);
  /*!
   * \brief Bind an element to a shape
   * \param ElementID - element ID
   * \param ShapeID - shape ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetMeshElementOnShape(SMESH::smIdType ElementID, CORBA::Long ShapeID);


  CORBA::Boolean MoveNode(SMESH::smIdType NodeID,
                          CORBA::Double x, CORBA::Double y, CORBA::Double z);

  CORBA::Boolean InverseDiag(SMESH::smIdType NodeID1, SMESH::smIdType NodeID2);
  CORBA::Boolean DeleteDiag(SMESH::smIdType NodeID1, SMESH::smIdType NodeID2);
  CORBA::Boolean Reorient(const SMESH::smIdType_array & IDsOfElements);
  CORBA::Boolean ReorientObject(SMESH::SMESH_IDSource_ptr theObject);

  /*!
   * \brief Reorient faces contained in \a the2Dgroup.
   * \param the2Dgroup - the mesh or its part to reorient
   * \param theDirection - desired direction of normal of \a theFace
   * \param theFace - ID of face whose orientation is checked.
   *        It can be < 1 then \a thePoint is used to find a face.
   * \param thePoint - is used to find a face if \a theFace < 1.
   * \return number of reoriented elements.
   */
  CORBA::Long Reorient2D(SMESH::SMESH_IDSource_ptr the2Dgroup,
                         const SMESH::DirStruct&   theDirection,
                         CORBA::Long               theFace,
                         const SMESH::PointStruct& thePoint);
  /*!
   * \brief Reorient faces basing on orientation of adjacent volumes.
   * \param faces - a list of objects containing face to reorient
   * \param volumes - an object containing volumes.
   * \param outsideNormal - to orient faces to have their normal
   *        pointing either \a outside or \a inside the adjacent volumes.
   * \return number of reoriented faces.
   */
  CORBA::Long Reorient2DBy3D(const SMESH::ListOfIDSources & faces,
                             SMESH::SMESH_IDSource_ptr      volumes,
                             CORBA::Boolean                 outsideNormal);

  // Split/Join
  CORBA::Boolean TriToQuad       (const SMESH::smIdType_array & IDsOfElements,
                                  SMESH::NumericalFunctor_ptr   Criterion,
                                  CORBA::Double                 MaxAngle);
  CORBA::Boolean TriToQuadObject (SMESH::SMESH_IDSource_ptr     theObject,
                                  SMESH::NumericalFunctor_ptr   Criterion,
                                  CORBA::Double                 MaxAngle);
  CORBA::Boolean QuadToTri       (const SMESH::smIdType_array & IDsOfElements,
                                  SMESH::NumericalFunctor_ptr   Criterion);
  CORBA::Boolean QuadToTriObject (SMESH::SMESH_IDSource_ptr     theObject,
                                  SMESH::NumericalFunctor_ptr   Criterion);
  void           QuadTo4Tri      (SMESH::SMESH_IDSource_ptr     theObject);
  CORBA::Boolean SplitQuad       (const SMESH::smIdType_array & IDsOfElements,
                                  CORBA::Boolean                Diag13);
  CORBA::Boolean SplitQuadObject (SMESH::SMESH_IDSource_ptr     theObject,
                                  CORBA::Boolean                Diag13);
  CORBA::Long    BestSplit       (CORBA::Long                   IDOfQuad,
                                  SMESH::NumericalFunctor_ptr   Criterion);
  void           SplitVolumesIntoTetra(SMESH::SMESH_IDSource_ptr elems,
                                       CORBA::Short             methodFlags);
  void           SplitHexahedraIntoPrisms(SMESH::SMESH_IDSource_ptr  elems,
                                          const SMESH::PointStruct & startHexPoint,
                                          const SMESH::DirStruct&    facetToSplitNormal,
                                          CORBA::Short               methodFlags,
                                          CORBA::Boolean             allDomains);
  void           SplitBiQuadraticIntoLinear(const SMESH::ListOfIDSources& elems);

  CORBA::Boolean Smooth(const SMESH::smIdType_array &          IDsOfElements,
                        const SMESH::smIdType_array &          IDsOfFixedNodes,
                        CORBA::Short                           MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                              const SMESH::smIdType_array &          IDsOfFixedNodes,
                              CORBA::Short                           MaxNbOfIterations,
                              CORBA::Double                          MaxAspectRatio,
                              SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothParametric(const SMESH::smIdType_array &          IDsOfElements,
                                  const SMESH::smIdType_array &          IDsOfFixedNodes,
                                  CORBA::Short                           MaxNbOfIterations,
                                  CORBA::Double                          MaxAspectRatio,
                                  SMESH::SMESH_MeshEditor::Smooth_Method Method) ;
  CORBA::Boolean SmoothParametricObject(SMESH::SMESH_IDSource_ptr              theObject,
                                        const SMESH::smIdType_array &          IDsOfFixedNodes,
                                        CORBA::Short                           MaxNbOfIterations,
                                        CORBA::Double                          MaxAspectRatio,
                                        SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean smooth(const SMESH::smIdType_array &          IDsOfElements,
                        const SMESH::smIdType_array &          IDsOfFixedNodes,
                        CORBA::Short                           MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method,
                        bool                                   IsParametric);
  CORBA::Boolean smoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                              const SMESH::smIdType_array &          IDsOfFixedNodes,
                              CORBA::Short                           MaxNbOfIterations,
                              CORBA::Double                          MaxAspectRatio,
                              SMESH::SMESH_MeshEditor::Smooth_Method Method,
                              bool                                   IsParametric);

  CORBA::Boolean ConvertFromQuadratic();
  void           ConvertFromQuadraticObject(SMESH::SMESH_IDSource_ptr theObject);
  void           ConvertToQuadratic(CORBA::Boolean Force3d);
  void           ConvertToQuadraticObject(CORBA::Boolean            theForce3d,
                                          SMESH::SMESH_IDSource_ptr theObject);
  void           ConvertToBiQuadratic(CORBA::Boolean            theForce3d,
                                      SMESH::SMESH_IDSource_ptr theObject);

  void RenumberNodes();
  void RenumberElements();

  SMESH::ListOfGroups* RotationSweepObjects(const SMESH::ListOfIDSources & Nodes,
                                            const SMESH::ListOfIDSources & Edges,
                                            const SMESH::ListOfIDSources & Faces,
                                            const SMESH::AxisStruct &      Axis,
                                            CORBA::Double                  AngleInRadians,
                                            CORBA::Long                    NbOfSteps,
                                            CORBA::Double                  Tolerance,
                                            CORBA::Boolean                 ToMakeGroups);

  SMESH::ListOfGroups* ExtrusionSweepObjects(const SMESH::ListOfIDSources & Nodes,
                                             const SMESH::ListOfIDSources & Edges,
                                             const SMESH::ListOfIDSources & Faces,
                                             const SMESH::DirStruct &       StepVector,
                                             CORBA::Long                    NbOfSteps,
                                             CORBA::Boolean                 ToMakeGroups,
                                             const SMESH::double_array &    ScaleFactors,
                                             CORBA::Boolean                 ScalesVariation,
                                             const SMESH::double_array &    BasePoint,
                                             const SMESH::double_array &    Angles,
                                             CORBA::Boolean                 AnglesVariation);

  SMESH::ListOfGroups* ExtrusionByNormal(const SMESH::ListOfIDSources& objects,
                                         CORBA::Double                 stepSize,
                                         CORBA::Long                   nbOfSteps,
                                         CORBA::Boolean                byAverageNormal,
                                         CORBA::Boolean                useInputElemsOnly,
                                         CORBA::Boolean                makeGroups,
                                         CORBA::Short                  dim);
  SMESH::ListOfGroups*  AdvancedExtrusion(const SMESH::smIdType_array & theIDsOfElements,
                                          const SMESH::DirStruct &      theStepVector,
                                          CORBA::Long                   theNbOfSteps,
                                          CORBA::Long                   theExtrFlags,
                                          CORBA::Double                 theSewTolerance,
                                          CORBA::Boolean                theMakeGroups);

  SMESH::ListOfGroups*
    ExtrusionAlongPathObjects(const SMESH::ListOfIDSources & Nodes,
                              const SMESH::ListOfIDSources & Edges,
                              const SMESH::ListOfIDSources & Faces,
                              SMESH::SMESH_IDSource_ptr      PathMesh,
                              GEOM::GEOM_Object_ptr          PathShape,
                              SMESH::smIdType                NodeStart,
                              CORBA::Boolean                 HasAngles,
                              const SMESH::double_array &    Angles,
                              CORBA::Boolean                 AnglesVariation,
                              CORBA::Boolean                 HasRefPoint,
                              const SMESH::PointStruct &     RefPoint,
                              bool                           MakeGroups,
                              const SMESH::double_array &    ScaleFactors,
                              CORBA::Boolean                 ScalesVariation,
                              SMESH::SMESH_MeshEditor::Extrusion_Error& Error);

  SMESH::double_array* LinearAnglesVariation(SMESH::SMESH_Mesh_ptr       PathMesh,
                                               GEOM::GEOM_Object_ptr       PathShape,
                                               const SMESH::double_array & Angles);

  void Mirror(const SMESH::smIdType_array &       IDsOfElements,
              const SMESH::AxisStruct &           Axis,
              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
              CORBA::Boolean                      Copy);
  void MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
                    const SMESH::AxisStruct &           Axis,
                    SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                    CORBA::Boolean                      Copy);
  void Translate(const SMESH::smIdType_array & IDsOfElements,
                 const SMESH::DirStruct &      Vector,
                 CORBA::Boolean                Copy);
  void TranslateObject(SMESH::SMESH_IDSource_ptr  theObject,
                       const SMESH::DirStruct &   Vector,
                       CORBA::Boolean             Copy);
  void Rotate(const SMESH::smIdType_array & IDsOfElements,
              const SMESH::AxisStruct &     Axis,
              CORBA::Double                 Angle,
              CORBA::Boolean                Copy);
  void RotateObject(SMESH::SMESH_IDSource_ptr  theObject,
                    const SMESH::AxisStruct &  Axis,
                    CORBA::Double              Angle,
                    CORBA::Boolean             Copy);

  SMESH::ListOfGroups* MirrorMakeGroups(const SMESH::smIdType_array&        IDsOfElements,
                                        const SMESH::AxisStruct&            Mirror,
                                        SMESH::SMESH_MeshEditor::MirrorType MirrorType);
  SMESH::ListOfGroups* MirrorObjectMakeGroups(SMESH::SMESH_IDSource_ptr           Object,
                                              const SMESH::AxisStruct&            Mirror,
                                              SMESH::SMESH_MeshEditor::MirrorType MirrorType);
  SMESH::ListOfGroups* TranslateMakeGroups(const SMESH::smIdType_array& IDsOfElements,
                                           const SMESH::DirStruct&      Vector);
  SMESH::ListOfGroups* TranslateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                 const SMESH::DirStruct&   Vector);
  SMESH::ListOfGroups* RotateMakeGroups(const SMESH::smIdType_array& IDsOfElements,
                                        const SMESH::AxisStruct&     Axis,
                                        CORBA::Double                AngleInRadians);
  SMESH::ListOfGroups* RotateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                              const SMESH::AxisStruct&  Axis,
                                              CORBA::Double             AngleInRadians);

  SMESH::SMESH_Mesh_ptr MirrorMakeMesh(const SMESH::smIdType_array&        IDsOfElements,
                                       const SMESH::AxisStruct&            Mirror,
                                       SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                       CORBA::Boolean                      CopyGroups,
                                       const char*                         MeshName);
  SMESH::SMESH_Mesh_ptr MirrorObjectMakeMesh(SMESH::SMESH_IDSource_ptr           Object,
                                             const SMESH::AxisStruct&            Mirror,
                                             SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                             CORBA::Boolean                      CopyGroups,
                                             const char*                         MeshName);
  SMESH::SMESH_Mesh_ptr TranslateMakeMesh(const SMESH::smIdType_array& IDsOfElements,
                                          const SMESH::DirStruct&      Vector,
                                          CORBA::Boolean               CopyGroups,
                                          const char*                  MeshName);
  SMESH::SMESH_Mesh_ptr TranslateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                                const SMESH::DirStruct&   Vector,
                                                CORBA::Boolean            CopyGroups,
                                                const char*               MeshName);
  SMESH::SMESH_Mesh_ptr RotateMakeMesh(const SMESH::smIdType_array& IDsOfElements,
                                       const SMESH::AxisStruct&     Axis,
                                       CORBA::Double                AngleInRadians,
                                       CORBA::Boolean               CopyGroups,
                                       const char*                  MeshName);
  SMESH::SMESH_Mesh_ptr RotateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                             const SMESH::AxisStruct&  Axis,
                                             CORBA::Double             AngleInRadians,
                                             CORBA::Boolean            CopyGroups,
                                             const char*               MeshName);

  void Scale(SMESH::SMESH_IDSource_ptr  theObject,
             const SMESH::PointStruct&  thePoint,
             const SMESH::double_array& theScaleFact,
             CORBA::Boolean             theCopy);

  SMESH::ListOfGroups* ScaleMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                       const SMESH::PointStruct&  thePoint,
                                       const SMESH::double_array& theScaleFact);

  SMESH::SMESH_Mesh_ptr ScaleMakeMesh(SMESH::SMESH_IDSource_ptr  Object,
                                      const SMESH::PointStruct&  Point,
                                      const SMESH::double_array& theScaleFact,
                                      CORBA::Boolean             CopyGroups,
                                      const char*                MeshName);

  SMESH::SMESH_Mesh_ptr Offset( SMESH::SMESH_IDSource_ptr theObject,
                                CORBA::Double             Value,
                                CORBA::Boolean            CopyGroups,
                                CORBA::Boolean            CopyElements,
                                const char*               MeshName,
                                SMESH::ListOfGroups_out   Groups);

  void FindCoincidentNodes (CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes,
                            CORBA::Boolean                 SeparateCornersAndMedium);
  void FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      Object,
                                 CORBA::Double                  Tolerance,
                                 SMESH::array_of_long_array_out GroupsOfNodes,
                                 CORBA::Boolean                 SeparateCornersAndMedium);
  void FindCoincidentNodesOnPartBut(const SMESH::ListOfIDSources&  Objects,
                                    CORBA::Double                  Tolerance,
                                    SMESH::array_of_long_array_out GroupsOfNodes,
                                    const SMESH::ListOfIDSources&  ExceptSubMeshOrGroups,
                                    CORBA::Boolean                 SeparateCornersAndMedium);
  void MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes,
                   const SMESH::ListOfIDSources&     NodesToKeep,
                   CORBA::Boolean                    AvoidMakingHoles );
  void FindEqualElements(const SMESH::ListOfIDSources&  Objects,
                         const SMESH::ListOfIDSources&  ExceptSubMeshOrGroups,
                         SMESH::array_of_long_array_out GroupsOfElementsID);
  void MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID,
                     const SMESH::ListOfIDSources&     ElementsToKeep);
  void MergeEqualElements();
  SMESH::smIdType MoveClosestNodeToPoint(CORBA::Double   x,
                                         CORBA::Double   y,
                                         CORBA::Double   z,
                                         SMESH::smIdType nodeID);
  /*!
   * \brief Return ID of node closest to a given point
   */
  SMESH::smIdType FindNodeClosestTo(CORBA::Double x,
                                CORBA::Double y,
                                CORBA::Double z);
  /*!
   * Return elements of given type where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::smIdType_array* FindElementsByPoint(CORBA::Double      x,
                                         CORBA::Double      y,
                                         CORBA::Double      z,
                                         SMESH::ElementType type);
  /*!
   * Searching among the given elements, return elements of given type
   * where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::smIdType_array* FindAmongElementsByPoint(SMESH::SMESH_IDSource_ptr elements,
                                              CORBA::Double             x,
                                              CORBA::Double             y,
                                              CORBA::Double             z,
                                              SMESH::ElementType        type);

  /*!
   * Project a point to a mesh object.
   * Return ID of an element of given type where the given point is projected
   * and coordinates of the projection point.
   * In the case if nothing found, return -1 and []
   */
  SMESH::smIdType ProjectPoint(CORBA::Double             x,
                           CORBA::Double             y,
                           CORBA::Double             z,
                           SMESH::ElementType        type,
                           SMESH::SMESH_IDSource_ptr meshObject,
                           SMESH::double_array_out   projecton);

  /*!
   * Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
   * TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
   */
  CORBA::Short GetPointState(CORBA::Double x, CORBA::Double y, CORBA::Double z);

  /*!
   * Check if a 2D mesh is manifold
   */
  CORBA::Boolean IsManifold();

  /*!
   * Check if orientation of 2D elements is coherent
   */
  CORBA::Boolean IsCoherentOrientation2D();

  /*!
   * Partition given 1D elements into groups of contiguous edges.
   * A node where number of meeting edges != 2 is a group end.
   * An optional startNode is used to orient groups it belongs to.
   * \return a list of edge groups and a list of corresponding node groups.
   *         If a group is closed, the first and last nodes of the group are same.
   */
  SMESH::array_of_long_array* Get1DBranches( SMESH::SMESH_IDSource_ptr      edges,
                                             SMESH::smIdType                startNode,
                                             SMESH::array_of_long_array_out nodeGroups);

  /*!
   * Return sharp edges of faces and non-manifold ones. Optionally adds existing edges.
   */
  SMESH::ListOfEdges* FindSharpEdges(CORBA::Double angle, CORBA::Boolean addExisting);

  /*!
   * Returns all or only closed FreeBorder's.
   */
  SMESH::ListOfFreeBorders* FindFreeBorders(CORBA::Boolean closedOnly);

  /*!
   * Fill with 2D elements a hole defined by a FreeBorder.
   * Optionally add new faces to a given group, which is returned
   */
  SMESH::SMESH_Group_ptr FillHole(const SMESH::FreeBorder& hole,
                                  const char*              groupName);

  SMESH::CoincidentFreeBorders* FindCoincidentFreeBorders(CORBA::Double tolerance);
  CORBA::Short SewCoincidentFreeBorders(const SMESH::CoincidentFreeBorders& freeBorders,
                                        CORBA::Boolean                      createPolygons,
                                        CORBA::Boolean                      createPolyedrs);

  SMESH::SMESH_MeshEditor::Sew_Error
    SewFreeBorders(SMESH::smIdType FirstNodeID1,
                   SMESH::smIdType SecondNodeID1,
                   SMESH::smIdType LastNodeID1,
                   SMESH::smIdType FirstNodeID2,
                   SMESH::smIdType SecondNodeID2,
                   SMESH::smIdType LastNodeID2,
                   CORBA::Boolean  CreatePolygons,
                   CORBA::Boolean  CreatePolyedrs);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewConformFreeBorders(SMESH::smIdType FirstNodeID1,
                          SMESH::smIdType SecondNodeID1,
                          SMESH::smIdType LastNodeID1,
                          SMESH::smIdType FirstNodeID2,
                          SMESH::smIdType SecondNodeID2);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewBorderToSide(SMESH::smIdType FirstNodeIDOnFreeBorder,
                    SMESH::smIdType SecondNodeIDOnFreeBorder,
                    SMESH::smIdType LastNodeIDOnFreeBorder,
                    SMESH::smIdType FirstNodeIDOnSide,
                    SMESH::smIdType LastNodeIDOnSide,
                    CORBA::Boolean CreatePolygons,
                    CORBA::Boolean CreatePolyedrs);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewSideElements(const SMESH::smIdType_array& IDsOfSide1Elements,
                    const SMESH::smIdType_array& IDsOfSide2Elements,
                    SMESH::smIdType NodeID1OfSide1ToMerge,
                    SMESH::smIdType NodeID1OfSide2ToMerge,
                    SMESH::smIdType NodeID2OfSide1ToMerge,
                    SMESH::smIdType NodeID2OfSide2ToMerge);

  /*!
   * Set new nodes for given element.
   * If number of nodes is not corresponded to type of
   * element - returns false
   */
  CORBA::Boolean ChangeElemNodes(SMESH::smIdType ide, const SMESH::smIdType_array& newIDs);

  SMESH::SMESH_Group_ptr DoubleElements(SMESH::SMESH_IDSource_ptr theElements,
                                        const char*               theGroupName);

  CORBA::Boolean DoubleNodes( const SMESH::smIdType_array& theNodes,
                              const SMESH::smIdType_array& theModifiedElems );

  CORBA::Boolean DoubleNode( SMESH::smIdType theNodeId,
                             const SMESH::smIdType_array& theModifiedElems );

  CORBA::Boolean DoubleNodeGroup( SMESH::SMESH_GroupBase_ptr theNodes,
                                  SMESH::SMESH_GroupBase_ptr theModifiedElems );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
   * Works as DoubleNodeGroup(), but returns a new group with newly created nodes.
   * \param theNodes - group of nodes to be doubled.
   * \param theModifiedElems - group of elements to be updated.
   * \return a new group with newly created nodes
   * \sa DoubleNodeGroup()
   */
  SMESH::SMESH_Group_ptr DoubleNodeGroupNew( SMESH::SMESH_GroupBase_ptr theNodes,
                                             SMESH::SMESH_GroupBase_ptr theModifiedElems );

  CORBA::Boolean DoubleNodeGroups( const SMESH::ListOfGroups& theNodes,
                                   const SMESH::ListOfGroups& theModifiedElems );

  SMESH::SMESH_Group_ptr DoubleNodeGroupsNew( const SMESH::ListOfGroups& theNodes,
                                              const SMESH::ListOfGroups& theModifiedElems );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * \param theElems - the list of elements (edges or faces) to be replicated
   *       The nodes for duplication could be found from these elements
   * \param theNodesNot - list of nodes to NOT replicate
   * \param theAffectedElems - the list of elements (cells and edges) to which the 
   *       replicated nodes should be associated to.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodeGroup(), DoubleNodeGroups()
   */
  CORBA::Boolean DoubleNodeElem( const SMESH::smIdType_array& theElems, 
                                 const SMESH::smIdType_array& theNodesNot,
                                 const SMESH::smIdType_array& theAffectedElems );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * \param theElems - the list of elements (edges or faces) to be replicated
   *        The nodes for duplication could be found from these elements
   * \param theNodesNot - list of nodes to NOT replicate
   * \param theShape - shape to detect affected elements (element which geometric center
   *        located on or inside shape).
   *        The replicated nodes should be associated to affected elements.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodeGroupInRegion(), DoubleNodeGroupsInRegion()
   */
  CORBA::Boolean DoubleNodeElemInRegion( const SMESH::smIdType_array& theElems, 
                                         const SMESH::smIdType_array& theNodesNot,
                                         GEOM::GEOM_Object_ptr        theShape );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * \param theElems - group of of elements (edges or faces) to be replicated
   * \param theNodesNot - group of nodes not to replicated
   * \param theAffectedElems - group of elements to which the replicated nodes
   *        should be associated to.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodes(), DoubleNodeGroups(), DoubleNodeElemGroupNew()
   */
  CORBA::Boolean DoubleNodeElemGroup( SMESH::SMESH_GroupBase_ptr theElems,
                                      SMESH::SMESH_GroupBase_ptr theNodesNot,
                                      SMESH::SMESH_GroupBase_ptr theAffectedElems );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * Works as DoubleNodeElemGroup(), but returns a new group with newly created elements.
   * \param theElems - group of of elements (edges or faces) to be replicated
   * \param theNodesNot - group of nodes not to replicated
   * \param theAffectedElems - group of elements to which the replicated nodes
   *        should be associated to.
   * \return a new group with newly created elements
   * \sa DoubleNodeElemGroup()
   */
  SMESH::SMESH_Group_ptr DoubleNodeElemGroupNew( SMESH::SMESH_GroupBase_ptr theElems,
                                                 SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                 SMESH::SMESH_GroupBase_ptr theAffectedElems );

  SMESH::ListOfGroups*   DoubleNodeElemGroup2New(SMESH::SMESH_GroupBase_ptr theElems,
                                                 SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                 SMESH::SMESH_GroupBase_ptr theAffectedElems,
                                                 CORBA::Boolean             theElemGroupNeeded,
                                                 CORBA::Boolean             theNodeGroupNeeded);
  
  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * \param theElems - group of of elements (edges or faces) to be replicated
   * \param theNodesNot - group of nodes not to replicated
   * \param theShape - shape to detect affected elements (element which geometric center
   *        located on or inside shape).
   *        The replicated nodes should be associated to affected elements.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodesInRegion(), DoubleNodeGroupsInRegion()
   */
  CORBA::Boolean DoubleNodeElemGroupInRegion( SMESH::SMESH_GroupBase_ptr theElems,
                                              SMESH::SMESH_GroupBase_ptr theNodesNot,
                                              GEOM::GEOM_Object_ptr      theShape );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * This method provided for convenience works as DoubleNodes() described above.
   * \param theElems - list of groups of elements (edges or faces) to be replicated
   * \param theNodesNot - list of groups of nodes not to replicated
   * \param theAffectedElems - group of elements to which the replicated nodes
   *        should be associated to.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodeGroup(), DoubleNodes(), DoubleNodeElemGroupsNew()
   */
  CORBA::Boolean DoubleNodeElemGroups( const SMESH::ListOfGroups& theElems,
                                       const SMESH::ListOfGroups& theNodesNot,
                                       const SMESH::ListOfGroups& theAffectedElems );

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * Works as DoubleNodeElemGroups(), but returns a new group with newly created elements.
   * \param theElems - list of groups of elements (edges or faces) to be replicated
   * \param theNodesNot - list of groups of nodes not to replicated
   * \param theAffectedElems - group of elements to which the replicated nodes
   *        should be associated to.
   * \return a new group with newly created elements
   * \sa DoubleNodeElemGroups()
   */
  SMESH::SMESH_Group_ptr DoubleNodeElemGroupsNew( const SMESH::ListOfGroups& theElems,
                                                  const SMESH::ListOfGroups& theNodesNot,
                                                  const SMESH::ListOfGroups& theAffectedElems );

  SMESH::ListOfGroups*   DoubleNodeElemGroups2New(const SMESH::ListOfGroups& theElems,
                                                  const SMESH::ListOfGroups& theNodesNot,
                                                  const SMESH::ListOfGroups& theAffectedElems,
                                                  CORBA::Boolean             theElemGroupNeeded,
                                                  CORBA::Boolean             theNodeGroupNeeded);

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements
   * This method provided for convenience works as DoubleNodes() described above.
   * \param theElems - list of groups of elements (edges or faces) to be replicated
   * \param theNodesNot - list of groups of nodes not to replicated
   * \param theShape - shape to detect affected elements (element which geometric center
   *        located on or inside shape).
   *        The replicated nodes should be associated to affected elements.
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   * \sa DoubleNodeGroupInRegion(), DoubleNodesInRegion()
   */
  CORBA::Boolean DoubleNodeElemGroupsInRegion( const SMESH::ListOfGroups& theElems,
                                               const SMESH::ListOfGroups& theNodesNot,
                                               GEOM::GEOM_Object_ptr      theShape );

  /*!
   * \brief Identify the elements that will be affected by node duplication (actual duplication is not performed.
   * This method is the first step of DoubleNodeElemGroupsInRegion.
   * \param theElems - list of groups of elements (edges or faces) to be replicated
   * \param theNodesNot - list of groups of nodes not to replicated
   * \param theShape - shape to detect affected elements (element which geometric center
   *        located on or inside shape).
   *        The replicated nodes should be associated to affected elements.
   * \return groups of affected elements
   * \sa DoubleNodeElemGroupsInRegion()
   */
  SMESH::ListOfGroups* AffectedElemGroupsInRegion( const SMESH::ListOfGroups& theElems,
                                                   const SMESH::ListOfGroups& theNodesNot,
                                                   GEOM::GEOM_Object_ptr      theShape );

  /*!
   * \brief Double nodes on shared faces between groups of volumes and create flat elements on demand.
   * The list of groups must describe a partition of the mesh volumes.
   * The nodes of the internal faces at the boundaries of the groups are doubled.
   * In option, the internal faces are replaced by flat elements.
   * Triangles are transformed in prisms, and quadrangles in hexahedrons.
   * \param theDomains - list of groups of volumes
   * \param createJointElems - if TRUE, create the elements
   * \param onAllBoundaries - if TRUE, the nodes and elements are also create on
   *        the boundary between \a theDomains and the rest mesh
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   */
  CORBA::Boolean DoubleNodesOnGroupBoundaries( const SMESH::ListOfGroups& theDomains,
                                               CORBA::Boolean             createJointElems,
                                               CORBA::Boolean             onAllBoundaries );
  /*!
   * \brief Double nodes on some external faces and create flat elements.
   * Flat elements are mainly used by some types of mechanic calculations.
   *
   * Each group of the list must be constituted of faces.
   * Triangles are transformed in prisms, and quadrangles in hexahedrons.
   * @param theGroupsOfFaces - list of groups of faces
   * @return TRUE if operation has been completed successfully, FALSE otherwise
   */
  CORBA::Boolean CreateFlatElementsOnFacesGroups( const SMESH::ListOfGroups& theGroupsOfFaces );

  /*!
   *  \brief identify all the elements around a geom shape, get the faces delimiting the hole
   *  Build groups of volume to remove, groups of faces to replace on the skin of the object,
   *  groups of faces to remove insidethe object, (idem edges).
   *  Build ordered list of nodes at the border of each group of faces to replace (to be used to build a geom subshape)
   */
  void CreateHoleSkin(CORBA::Double radius,
                      GEOM::GEOM_Object_ptr theShape,
                      const char* groupName,
                      const SMESH::double_array& theNodesCoords,
                      SMESH::array_of_long_array_out GroupsOfNodes);

  /*!
   * \brief Generated skin mesh (containing 2D cells) from 3D mesh
   * The created 2D mesh elements based on nodes of free faces of boundary volumes
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   */
  CORBA::Boolean Make2DMeshFrom3D();

  SMESH::SMESH_Mesh_ptr MakeBoundaryMesh(SMESH::SMESH_IDSource_ptr elements,
                                         SMESH::Bnd_Dimension      dimension,
                                         const char*               groupName,
                                         const char*               meshName,
                                         CORBA::Boolean            toCopyElements,
                                         CORBA::Boolean            toCopyMissingBondary,
                                         SMESH::SMESH_Group_out    group);

  CORBA::Long MakeBoundaryElements(SMESH::Bnd_Dimension dimension,
                                   const char* groupName,
                                   const char* meshName,
                                   CORBA::Boolean toCopyAll,
                                   const SMESH::ListOfIDSources& groups,
                                   SMESH::SMESH_Mesh_out mesh,
                                   SMESH::SMESH_Group_out group);

  /*!
   * \brief Create a polyline consisting of 1D mesh elements each lying on a 2D element of
   *        the initial mesh. Positions of new nodes are found by cutting the mesh by the
   *        plane passing through pairs of points specified by each PolySegment structure.
   *        If there are several paths connecting a pair of points, the shortest path is
   *        selected by the module. Position of the cutting plane is defined by the two
   *        points and an optional vector lying on the plane specified by a PolySegment.
   *        By default the vector is defined by Mesh module as following. A middle point
   *        of the two given points is computed. The middle point is projected to the mesh.
   *        The vector goes from the middle point to the projection point. In case of planar
   *        mesh, the vector is normal to the mesh.
   *  \param [inout] segments - PolySegment's defining positions of cutting planes.
   *        Return the used vector and position of the middle point.
   *  \param [in] groupName - optional name of a group where created mesh segments will
   *        be added.
   */
  void MakePolyLine(SMESH::ListOfPolySegments& segments,
                    const char*                groupName);

  /*!
   * \brief Create a slot of given width around given 1D elements lying on a triangle mesh.
   *        The slot is constructed by cutting faces by cylindrical surfaces made
   *        around each segment. Segments are expected to be created by MakePolyLine().
   * \return Edges located at the slot boundary
   */
  SMESH::ListOfEdges* MakeSlot(SMESH::SMESH_GroupBase_ptr segments,
                               CORBA::Double              width);


 private: //!< private methods

  ::SMESH_MeshEditor& getEditor();

  SMESHDS_Mesh * getMeshDS() { return myMesh->GetMeshDS(); }

  MeshEditor_I::TPreviewMesh * getPreviewMesh( SMDSAbs_ElementType previewType = SMDSAbs_All );

  void declareMeshModified( bool isReComputeSafe );

  /*!
   * \brief Clear myLastCreated* or myPreviewData
   */
  void initData(bool deleteSearchers=true);

  /*!
   * \brief Return groups by their IDs
   */
  SMESH::ListOfGroups* getGroups(const std::list<int>* groupIDs);

  SMESH::ListOfGroups* mirror(TIDSortedElemSet &                  IDsOfElements,
                              const SMESH::AxisStruct &           Axis,
                              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                              CORBA::Boolean                      Copy,
                              bool                                MakeGroups,
                              ::SMESH_Mesh*                       TargetMesh=0);
  SMESH::ListOfGroups* translate(TIDSortedElemSet        & IDsOfElements,
                                 const SMESH::DirStruct &  Vector,
                                 CORBA::Boolean            Copy,
                                 bool                      MakeGroups,
                                 ::SMESH_Mesh*             TargetMesh=0);
  SMESH::ListOfGroups* rotate(TIDSortedElemSet &           IDsOfElements,
                              const SMESH::AxisStruct &  Axis,
                              CORBA::Double             Angle,
                              CORBA::Boolean            Copy,
                              bool                      MakeGroups,
                              ::SMESH_Mesh*             TargetMesh=0);

  SMESH::ListOfGroups* scale(SMESH::SMESH_IDSource_ptr   theObject,
                             const SMESH::PointStruct&   thePoint,
                             const SMESH::double_array&  theScaleFact,
                             CORBA::Boolean              theCopy,
                             bool                        theMakeGroups,
                             ::SMESH_Mesh*               theTargetMesh=0);

  void convertToQuadratic(CORBA::Boolean            theForce3d,
                          CORBA::Boolean            theToBiQuad,
                          SMESH::SMESH_IDSource_ptr theObject = SMESH::SMESH_IDSource::_nil());

  SMESH::SMESH_Mesh_ptr makeMesh(const char* theMeshName);

  void dumpGroupsList(SMESH::TPythonDump &        theDumpPython,
                      const SMESH::ListOfGroups * theGroupList);

  void prepareIdSource(SMESH::SMESH_IDSource_ptr theObject);


  enum IDSource_Error { IDSource_OK, IDSource_INVALID, IDSource_EMPTY };

  bool idSourceToSet(SMESH::SMESH_IDSource_ptr  theIDSource,
                     const SMESHDS_Mesh*        theMeshDS,
                     TIDSortedElemSet&          theElemSet,
                     const SMDSAbs_ElementType  theType,
                     const bool                 emptyIfIsMesh = false,
                     IDSource_Error*            error = 0);

  void findCoincidentNodes( TIDSortedNodeSet &             Nodes,
                            CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes,
                            CORBA::Boolean                 SeparateCornersAndMedium);



 private: //!< fields

  SMESH_Mesh_i*                myMesh_i;
  SMESH_Mesh *                 myMesh;
  ::SMESH_MeshEditor           myEditor;

  bool                         myIsPreviewMode;
  MeshEditor_I::TPreviewMesh * myPreviewMesh;
  ::SMESH_MeshEditor *         myPreviewEditor;
  SMESH::MeshPreviewStruct_var myPreviewData;

  // temporary IDSources
  struct _IDSource;
};

#endif
