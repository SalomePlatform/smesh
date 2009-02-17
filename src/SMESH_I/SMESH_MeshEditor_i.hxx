//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
#include <list>

class SMESH_MeshEditor;
class SMESH_Mesh_i;

class SMESH_MeshEditor_i: public POA_SMESH::SMESH_MeshEditor
{
 public:
  SMESH_MeshEditor_i(SMESH_Mesh_i * theMesh, bool isPreview);

  virtual ~ SMESH_MeshEditor_i();

  // --- CORBA
  CORBA::Boolean RemoveElements(const SMESH::long_array & IDsOfElements);
  CORBA::Boolean RemoveNodes(const SMESH::long_array & IDsOfNodes);

  /*!
   * Methods for creation new elements.
   * Returns ID of created element or 0 if element not created
   */
  CORBA::Long AddNode(CORBA::Double x, CORBA::Double y, CORBA::Double z);
  CORBA::Long AddEdge(const SMESH::long_array & IDsOfNodes);
  CORBA::Long AddFace(const SMESH::long_array & IDsOfNodes);
  CORBA::Long AddPolygonalFace(const SMESH::long_array & IDsOfNodes);
  CORBA::Long AddVolume(const SMESH::long_array & IDsOfNodes);
  CORBA::Long AddPolyhedralVolume(const SMESH::long_array & IDsOfNodes,
                                  const SMESH::long_array & Quantities);
  CORBA::Long AddPolyhedralVolumeByFaces(const SMESH::long_array & IdsOfFaces);

  /*!
   * \brief Bind a node to a vertex
   * \param NodeID - node ID
   * \param VertexID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetNodeOnVertex(CORBA::Long NodeID, CORBA::Long VertexID)
    throw (SALOME::SALOME_Exception);
  /*!
   * \brief Store node position on an edge
   * \param NodeID - node ID
   * \param EdgeID - edge ID available through GEOM_Object.GetSubShapeIndices()[0]
   * \param paramOnEdge - parameter on edge where the node is located
   */
  void SetNodeOnEdge(CORBA::Long NodeID, CORBA::Long EdgeID,
                     CORBA::Double paramOnEdge)
    throw (SALOME::SALOME_Exception);
  /*!
   * \brief Store node position on a face
   * \param NodeID - node ID
   * \param FaceID - face ID available through GEOM_Object.GetSubShapeIndices()[0]
   * \param u - U parameter on face where the node is located
   * \param v - V parameter on face where the node is located
   */
  void SetNodeOnFace(CORBA::Long NodeID, CORBA::Long FaceID,
                     CORBA::Double u, CORBA::Double v)
    throw (SALOME::SALOME_Exception);
  /*!
   * \brief Bind a node to a solid
   * \param NodeID - node ID
   * \param SolidID - vertex ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetNodeInVolume(CORBA::Long NodeID, CORBA::Long SolidID)
    throw (SALOME::SALOME_Exception);
  /*!
   * \brief Bind an element to a shape
   * \param ElementID - element ID
   * \param ShapeID - shape ID available through GEOM_Object.GetSubShapeIndices()[0]
   */
  void SetMeshElementOnShape(CORBA::Long ElementID, CORBA::Long ShapeID)
    throw (SALOME::SALOME_Exception);


  CORBA::Boolean MoveNode(CORBA::Long NodeID,
                          CORBA::Double x, CORBA::Double y, CORBA::Double z);

  CORBA::Boolean InverseDiag(CORBA::Long NodeID1, CORBA::Long NodeID2);
  CORBA::Boolean DeleteDiag(CORBA::Long NodeID1, CORBA::Long NodeID2);
  CORBA::Boolean Reorient(const SMESH::long_array & IDsOfElements);
  CORBA::Boolean ReorientObject(SMESH::SMESH_IDSource_ptr theObject);

  // Split/Join faces
  CORBA::Boolean TriToQuad       (const SMESH::long_array &   IDsOfElements,
                                  SMESH::NumericalFunctor_ptr Criterion,
                                  CORBA::Double               MaxAngle);
  CORBA::Boolean TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  SMESH::NumericalFunctor_ptr Criterion,
                                  CORBA::Double               MaxAngle);
  CORBA::Boolean QuadToTri       (const SMESH::long_array &   IDsOfElements,
                                  SMESH::NumericalFunctor_ptr Criterion);
  CORBA::Boolean QuadToTriObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  SMESH::NumericalFunctor_ptr Criterion);
  CORBA::Boolean SplitQuad       (const SMESH::long_array &   IDsOfElements,
                                  CORBA::Boolean              Diag13);
  CORBA::Boolean SplitQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  CORBA::Boolean              Diag13);
  CORBA::Long    BestSplit       (CORBA::Long                 IDOfQuad,
                                  SMESH::NumericalFunctor_ptr Criterion);

  CORBA::Boolean Smooth(const SMESH::long_array &              IDsOfElements,
                        const SMESH::long_array &              IDsOfFixedNodes,
                        CORBA::Long                            MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
			      const SMESH::long_array &              IDsOfFixedNodes,
			      CORBA::Long                            MaxNbOfIterations,
			      CORBA::Double                          MaxAspectRatio,
			      SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothParametric(const SMESH::long_array &              IDsOfElements,
                                  const SMESH::long_array &              IDsOfFixedNodes,
                                  CORBA::Long                            MaxNbOfIterations,
                                  CORBA::Double                          MaxAspectRatio,
                                  SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothParametricObject(SMESH::SMESH_IDSource_ptr              theObject,
                                        const SMESH::long_array &              IDsOfFixedNodes,
                                        CORBA::Long                            MaxNbOfIterations,
                                        CORBA::Double                          MaxAspectRatio,
                                        SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean smooth(const SMESH::long_array &              IDsOfElements,
                        const SMESH::long_array &              IDsOfFixedNodes,
                        CORBA::Long                            MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method,
                        bool                                   IsParametric);
  CORBA::Boolean smoothObject(SMESH::SMESH_IDSource_ptr              theObject,
			      const SMESH::long_array &              IDsOfFixedNodes,
			      CORBA::Long                            MaxNbOfIterations,
			      CORBA::Double                          MaxAspectRatio,
			      SMESH::SMESH_MeshEditor::Smooth_Method Method,
                              bool                                   IsParametric);


  void ConvertToQuadratic(CORBA::Boolean Force3d);
  CORBA::Boolean ConvertFromQuadratic();

  void RenumberNodes();
  void RenumberElements();

  void RotationSweep(const SMESH::long_array & IDsOfElements,
                     const SMESH::AxisStruct & Axis,
                     CORBA::Double             AngleInRadians,
                     CORBA::Long               NbOfSteps,
                     CORBA::Double             Tolerance);
  void RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
			   const SMESH::AxisStruct & Axis,
			   CORBA::Double             AngleInRadians,
			   CORBA::Long               NbOfSteps,
			   CORBA::Double             Tolerance);
  void RotationSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
			     const SMESH::AxisStruct & Axis,
			     CORBA::Double             AngleInRadians,
			     CORBA::Long               NbOfSteps,
			     CORBA::Double             Tolerance);
  void RotationSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
			     const SMESH::AxisStruct & Axis,
			     CORBA::Double             AngleInRadians,
			     CORBA::Long               NbOfSteps,
			     CORBA::Double             Tolerance);

  void ExtrusionSweep(const SMESH::long_array & IDsOfElements,
                      const SMESH::DirStruct &  StepVector,
                      CORBA::Long               NbOfSteps);
  void ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
			    const SMESH::DirStruct &  StepVector,
			    CORBA::Long               NbOfSteps);
  void ExtrusionSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                              const SMESH::DirStruct &  StepVector,
                              CORBA::Long               NbOfSteps);
  void ExtrusionSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                              const SMESH::DirStruct &  StepVector,
                              CORBA::Long               NbOfSteps);
  void AdvancedExtrusion(const SMESH::long_array & theIDsOfElements,
			 const SMESH::DirStruct &  theStepVector,
			 CORBA::Long               theNbOfSteps,
			 CORBA::Long               theExtrFlags,
			 CORBA::Double             theSewTolerance);

  SMESH::SMESH_MeshEditor::Extrusion_Error
    ExtrusionAlongPath(const SMESH::long_array &   IDsOfElements,
                       SMESH::SMESH_Mesh_ptr       PathMesh,
                       GEOM::GEOM_Object_ptr       PathShape,
                       CORBA::Long                 NodeStart,
                       CORBA::Boolean              HasAngles,
                       const SMESH::double_array & Angles,
                       CORBA::Boolean              HasRefPoint,
                       const SMESH::PointStruct &  RefPoint);

  SMESH::SMESH_MeshEditor::Extrusion_Error
    ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
                             SMESH::SMESH_Mesh_ptr       PathMesh,
                             GEOM::GEOM_Object_ptr       PathShape,
                             CORBA::Long                 NodeStart,
                             CORBA::Boolean              HasAngles,
                             const SMESH::double_array & Angles,
                             CORBA::Boolean              HasRefPoint,
                             const SMESH::PointStruct &  RefPoint);
  SMESH::SMESH_MeshEditor::Extrusion_Error
    ExtrusionAlongPathObject1D(SMESH::SMESH_IDSource_ptr   theObject,
			       SMESH::SMESH_Mesh_ptr       PathMesh,
			       GEOM::GEOM_Object_ptr       PathShape,
			       CORBA::Long                 NodeStart,
			       CORBA::Boolean              HasAngles,
			       const SMESH::double_array & Angles,
			       CORBA::Boolean              HasRefPoint,
			       const SMESH::PointStruct &  RefPoint);
  SMESH::SMESH_MeshEditor::Extrusion_Error
    ExtrusionAlongPathObject2D(SMESH::SMESH_IDSource_ptr   theObject,
			       SMESH::SMESH_Mesh_ptr       PathMesh,
			       GEOM::GEOM_Object_ptr       PathShape,
			       CORBA::Long                 NodeStart,
			       CORBA::Boolean              HasAngles,
			       const SMESH::double_array & Angles,
			       CORBA::Boolean              HasRefPoint,
			       const SMESH::PointStruct &  RefPoint);

  SMESH::double_array* LinearAnglesVariation(SMESH::SMESH_Mesh_ptr       PathMesh,
                                             GEOM::GEOM_Object_ptr       PathShape,
                                             const SMESH::double_array & Angles);

  void Mirror(const SMESH::long_array &           IDsOfElements,
              const SMESH::AxisStruct &           Axis,
              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
              CORBA::Boolean                      Copy);
  void MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
		    const SMESH::AxisStruct &           Axis,
		    SMESH::SMESH_MeshEditor::MirrorType MirrorType,
		    CORBA::Boolean                      Copy);
  void Translate(const SMESH::long_array & IDsOfElements,
                 const SMESH::DirStruct &   Vector,
                 CORBA::Boolean            Copy);
  void TranslateObject(SMESH::SMESH_IDSource_ptr  theObject,
		       const SMESH::DirStruct &   Vector,
		       CORBA::Boolean             Copy);
  void Rotate(const SMESH::long_array & IDsOfElements,
              const SMESH::AxisStruct &  Axis,
              CORBA::Double             Angle,
              CORBA::Boolean            Copy);
  void RotateObject(SMESH::SMESH_IDSource_ptr  theObject,
		    const SMESH::AxisStruct &  Axis,
		    CORBA::Double              Angle,
		    CORBA::Boolean             Copy);

  SMESH::ListOfGroups* RotationSweepMakeGroups(const SMESH::long_array& IDsOfElements,
                                               const SMESH::AxisStruct& Axix,
                                               CORBA::Double            AngleInRadians,
                                               CORBA::Long              NbOfSteps,
                                               CORBA::Double            Tolerance);
  SMESH::ListOfGroups* RotationSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                     const SMESH::AxisStruct&  Axix,
                                                     CORBA::Double             AngleInRadians,
                                                     CORBA::Long               NbOfSteps,
                                                     CORBA::Double             Tolerance);
  SMESH::ListOfGroups* RotationSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
						       const SMESH::AxisStruct&  Axix,
						       CORBA::Double             AngleInRadians,
						       CORBA::Long               NbOfSteps,
						       CORBA::Double             Tolerance);
  SMESH::ListOfGroups* RotationSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
						       const SMESH::AxisStruct&  Axix,
						       CORBA::Double             AngleInRadians,
						       CORBA::Long               NbOfSteps,
						       CORBA::Double             Tolerance);
  SMESH::ListOfGroups* ExtrusionSweepMakeGroups(const SMESH::long_array& IDsOfElements,
                                                const SMESH::DirStruct&  StepVector,
                                                CORBA::Long              NbOfSteps);
  SMESH::ListOfGroups* AdvancedExtrusionMakeGroups(const SMESH::long_array& IDsOfElements,
                                                   const SMESH::DirStruct&  StepVector,
                                                   CORBA::Long              NbOfSteps,
                                                   CORBA::Long              ExtrFlags,
                                                   CORBA::Double            SewTolerance);
  SMESH::ListOfGroups* ExtrusionSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                      const SMESH::DirStruct&   StepVector,
                                                      CORBA::Long               NbOfSteps);
  SMESH::ListOfGroups* ExtrusionSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                        const SMESH::DirStruct&   StepVector,
                                                        CORBA::Long               NbOfSteps);
  SMESH::ListOfGroups* ExtrusionSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                        const SMESH::DirStruct&   StepVector,
                                                        CORBA::Long               NbOfSteps);
  SMESH::ListOfGroups* ExtrusionAlongPathMakeGroups(const SMESH::long_array&   IDsOfElements,
                                                    SMESH::SMESH_Mesh_ptr      PathMesh,
                                                    GEOM::GEOM_Object_ptr      PathShape,
                                                    CORBA::Long                NodeStart,
                                                    CORBA::Boolean             HasAngles,
                                                    const SMESH::double_array& Angles,
                                                    CORBA::Boolean             HasRefPoint,
                                                    const SMESH::PointStruct&  RefPoint,
                                                    SMESH::SMESH_MeshEditor::Extrusion_Error& Error);
  SMESH::ListOfGroups* ExtrusionAlongPathObjectMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
                                                          SMESH::SMESH_Mesh_ptr      PathMesh,
                                                          GEOM::GEOM_Object_ptr      PathShape,
                                                          CORBA::Long                NodeStart,
                                                          CORBA::Boolean             HasAngles,
                                                          const SMESH::double_array& Angles,
                                                          CORBA::Boolean             HasRefPoint,
                                                          const SMESH::PointStruct&  RefPoint,
                                                          SMESH::SMESH_MeshEditor::Extrusion_Error& Error);
  SMESH::ListOfGroups* ExtrusionAlongPathObject1DMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
							    SMESH::SMESH_Mesh_ptr      PathMesh,
							    GEOM::GEOM_Object_ptr      PathShape,
							    CORBA::Long                NodeStart,
							    CORBA::Boolean             HasAngles,
							    const SMESH::double_array& Angles,
							    CORBA::Boolean             HasRefPoint,
							    const SMESH::PointStruct&  RefPoint,
							    SMESH::SMESH_MeshEditor::Extrusion_Error& Error);
  SMESH::ListOfGroups* ExtrusionAlongPathObject2DMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
							    SMESH::SMESH_Mesh_ptr      PathMesh,
							    GEOM::GEOM_Object_ptr      PathShape,
							    CORBA::Long                NodeStart,
							    CORBA::Boolean             HasAngles,
							    const SMESH::double_array& Angles,
							    CORBA::Boolean             HasRefPoint,
							    const SMESH::PointStruct&  RefPoint,
							    SMESH::SMESH_MeshEditor::Extrusion_Error& Error);
  SMESH::ListOfGroups* MirrorMakeGroups(const SMESH::long_array&            IDsOfElements,
                                        const SMESH::AxisStruct&            Mirror,
                                        SMESH::SMESH_MeshEditor::MirrorType MirrorType);
  SMESH::ListOfGroups* MirrorObjectMakeGroups(SMESH::SMESH_IDSource_ptr           Object,
                                              const SMESH::AxisStruct&            Mirror,
                                              SMESH::SMESH_MeshEditor::MirrorType MirrorType);
  SMESH::ListOfGroups* TranslateMakeGroups(const SMESH::long_array& IDsOfElements,
                                           const SMESH::DirStruct&  Vector);
  SMESH::ListOfGroups* TranslateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                 const SMESH::DirStruct&   Vector);
  SMESH::ListOfGroups* RotateMakeGroups(const SMESH::long_array& IDsOfElements,
                                        const SMESH::AxisStruct& Axis,
                                        CORBA::Double            AngleInRadians);
  SMESH::ListOfGroups* RotateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                              const SMESH::AxisStruct&  Axis,
                                              CORBA::Double             AngleInRadians);

  SMESH::SMESH_Mesh_ptr MirrorMakeMesh(const SMESH::long_array&            IDsOfElements,
                                       const SMESH::AxisStruct&            Mirror,
                                       SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                       CORBA::Boolean                      CopyGroups,
                                       const char*                         MeshName);
  SMESH::SMESH_Mesh_ptr MirrorObjectMakeMesh(SMESH::SMESH_IDSource_ptr           Object,
                                             const SMESH::AxisStruct&            Mirror,
                                             SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                             CORBA::Boolean                      CopyGroups,
                                             const char*                         MeshName);
  SMESH::SMESH_Mesh_ptr TranslateMakeMesh(const SMESH::long_array& IDsOfElements,
                                          const SMESH::DirStruct&  Vector,
                                          CORBA::Boolean           CopyGroups,
                                          const char*              MeshName);
  SMESH::SMESH_Mesh_ptr TranslateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                                const SMESH::DirStruct&   Vector,
                                                CORBA::Boolean            CopyGroups,
                                                const char*               MeshName);
  SMESH::SMESH_Mesh_ptr RotateMakeMesh(const SMESH::long_array& IDsOfElements,
                                       const SMESH::AxisStruct& Axis,
                                       CORBA::Double            AngleInRadians,
                                       CORBA::Boolean           CopyGroups,
                                       const char*              MeshName);
  SMESH::SMESH_Mesh_ptr RotateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                             const SMESH::AxisStruct&  Axis,
                                             CORBA::Double             AngleInRadians,
                                             CORBA::Boolean            CopyGroups,
                                             const char*               MeshName);

  void FindCoincidentNodes (CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes);
  void FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      Object,
                                 CORBA::Double                  Tolerance,
                                 SMESH::array_of_long_array_out GroupsOfNodes);
  void MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes);
  void FindEqualElements(SMESH::SMESH_IDSource_ptr      Object,
                         SMESH::array_of_long_array_out GroupsOfElementsID);
  void MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID);
  void MergeEqualElements();
  CORBA::Long MoveClosestNodeToPoint(CORBA::Double x,
                                     CORBA::Double y,
                                     CORBA::Double z,
                                     CORBA::Long   nodeID);



  SMESH::SMESH_MeshEditor::Sew_Error
  SewFreeBorders(CORBA::Long FirstNodeID1,
                 CORBA::Long SecondNodeID1,
                 CORBA::Long LastNodeID1,
                 CORBA::Long FirstNodeID2,
                 CORBA::Long SecondNodeID2,
                 CORBA::Long LastNodeID2,
                 CORBA::Boolean CreatePolygons,
                 CORBA::Boolean CreatePolyedrs);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewConformFreeBorders(CORBA::Long FirstNodeID1,
                        CORBA::Long SecondNodeID1,
                        CORBA::Long LastNodeID1,
                        CORBA::Long FirstNodeID2,
                        CORBA::Long SecondNodeID2);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                  CORBA::Long SecondNodeIDOnFreeBorder,
                  CORBA::Long LastNodeIDOnFreeBorder,
                  CORBA::Long FirstNodeIDOnSide,
                  CORBA::Long LastNodeIDOnSide,
                  CORBA::Boolean CreatePolygons,
                  CORBA::Boolean CreatePolyedrs);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                  const SMESH::long_array& IDsOfSide2Elements,
                  CORBA::Long NodeID1OfSide1ToMerge,
                  CORBA::Long NodeID1OfSide2ToMerge,
                  CORBA::Long NodeID2OfSide1ToMerge,
                  CORBA::Long NodeID2OfSide2ToMerge);

  /*!
   * Set new nodes for given element.
   * If number of nodes is not corresponded to type of
   * element - returns false
   */
  CORBA::Boolean ChangeElemNodes(CORBA::Long ide, const SMESH::long_array& newIDs);
  
  /*!
   * Return data of mesh edition preview
   */
  SMESH::MeshPreviewStruct* GetPreviewData();

  /*!
   * If during last operation of MeshEditor some nodes were
   * created this method returns list of it's IDs, if new nodes
   * not creared - returns empty list
   */
  SMESH::long_array* GetLastCreatedNodes();

  /*!
   * If during last operation of MeshEditor some elements were
   * created this method returns list of it's IDs, if new elements
   * not creared - returns empty list
   */
  SMESH::long_array* GetLastCreatedElems();

  /*!
   * \brief Return edited mesh ID
    * \retval int - mesh ID
   */
  int GetMeshId() const { return myMesh->GetId(); }
  
  CORBA::Boolean DoubleNodes( const SMESH::long_array& theNodes,
                              const SMESH::long_array& theModifiedElems );

  CORBA::Boolean DoubleNode( CORBA::Long theNodeId,
                             const SMESH::long_array& theModifiedElems );

  CORBA::Boolean DoubleNodeGroup( SMESH::SMESH_GroupBase_ptr theNodes,
                                  SMESH::SMESH_GroupBase_ptr theModifiedElems );

  CORBA::Boolean DoubleNodeGroups( const SMESH::ListOfGroups& theNodes,
                                   const SMESH::ListOfGroups& theModifiedElems);

private: //!< private methods

  SMESHDS_Mesh * GetMeshDS() { return myMesh->GetMeshDS(); }

  /*!
   * \brief Update myLastCreated* or myPreviewData
    * \param anEditor - it contains edition results
   */
  void storeResult(::SMESH_MeshEditor& anEditor);
  /*!
   * \brief Clear myLastCreated* or myPreviewData
   */
  void initData();

  /*!
   * \brief Return groups by their IDs
   */
  SMESH::ListOfGroups* getGroups(const std::list<int>* groupIDs);

  SMESH::ListOfGroups* rotationSweep(const SMESH::long_array & IDsOfElements,
                                     const SMESH::AxisStruct & Axis,
                                     CORBA::Double             AngleInRadians,
                                     CORBA::Long               NbOfSteps,
                                     CORBA::Double             Tolerance,
                                     const bool                MakeGroups,
				     const SMDSAbs_ElementType ElementType=SMDSAbs_All);
  SMESH::ListOfGroups* extrusionSweep(const SMESH::long_array & IDsOfElements,
                                      const SMESH::DirStruct &  StepVector,
                                      CORBA::Long               NbOfSteps,
                                      const bool                MakeGroups,
                                      const SMDSAbs_ElementType ElementType=SMDSAbs_All);
  SMESH::ListOfGroups* advancedExtrusion(const SMESH::long_array & theIDsOfElements,
                                         const SMESH::DirStruct &  theStepVector,
                                         CORBA::Long               theNbOfSteps,
                                         CORBA::Long               theExtrFlags,
                                         CORBA::Double             theSewTolerance,
                                         const bool                MakeGroups);
  SMESH::ListOfGroups* extrusionAlongPath(const SMESH::long_array &   IDsOfElements,
                                          SMESH::SMESH_Mesh_ptr       PathMesh,
                                          GEOM::GEOM_Object_ptr       PathShape,
                                          CORBA::Long                 NodeStart,
                                          CORBA::Boolean              HasAngles,
                                          const SMESH::double_array & Angles,
                                          CORBA::Boolean              HasRefPoint,
                                          const SMESH::PointStruct &  RefPoint,
                                          const bool                  MakeGroups,
                                          SMESH::SMESH_MeshEditor::Extrusion_Error & Error,
					  const SMDSAbs_ElementType   ElementType=SMDSAbs_All);
  SMESH::ListOfGroups* mirror(const SMESH::long_array &           IDsOfElements,
                              const SMESH::AxisStruct &           Axis,
                              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                              CORBA::Boolean                      Copy,
                              const bool                          MakeGroups,
                              ::SMESH_Mesh*                       TargetMesh=0);
  SMESH::ListOfGroups* translate(const SMESH::long_array & IDsOfElements,
                                 const SMESH::DirStruct &  Vector,
                                 CORBA::Boolean            Copy,
                                 const bool                MakeGroups,
                                 ::SMESH_Mesh*             TargetMesh=0);
  SMESH::ListOfGroups* rotate(const SMESH::long_array & IDsOfElements,
                              const SMESH::AxisStruct &  Axis,
                              CORBA::Double             Angle,
                              CORBA::Boolean            Copy,
                              const bool                MakeGroups,
                              ::SMESH_Mesh*             TargetMesh=0);

  SMESH::SMESH_Mesh_ptr makeMesh(const char* theMeshName);
  
  void DumpGroupsList(SMESH::TPythonDump & theDumpPython, 
                      const SMESH::ListOfGroups * theGroupList);

private: //!< fields

  SMESH_Mesh_i*         myMesh_i;
  SMESH_Mesh *          myMesh;

  SMESH::long_array_var myLastCreatedElems;
  SMESH::long_array_var myLastCreatedNodes;

  SMESH::MeshPreviewStruct_var myPreviewData;
  bool                         myPreviewMode;
};

#endif
