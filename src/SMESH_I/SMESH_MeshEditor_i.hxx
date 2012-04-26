// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
#include "SMESH_MeshEditor.hxx"
#include <list>

class SMESH_MeshEditor;
class SMESH_Mesh_i;

class SMESH_MeshEditor_i: public POA_SMESH::SMESH_MeshEditor
{
public:
  SMESH_MeshEditor_i(SMESH_Mesh_i * theMesh, bool isPreview);

  virtual ~ SMESH_MeshEditor_i();

  // --- CORBA

  /*!
   * \brief Wrap a sequence of ids in a SMESH_IDSource
   */
  SMESH::SMESH_IDSource_ptr MakeIDSource(const SMESH::long_array& IDsOfElements,
                                         SMESH::ElementType       type);
  CORBA::Boolean RemoveElements(const SMESH::long_array & IDsOfElements);
  CORBA::Boolean RemoveNodes(const SMESH::long_array & IDsOfNodes);
  CORBA::Long    RemoveOrphanNodes();

  /*!
   * Methods for creation new elements.
   * Returns ID of created element or 0 if element not created
   */
  CORBA::Long AddNode(CORBA::Double x, CORBA::Double y, CORBA::Double z);
  CORBA::Long Add0DElement(CORBA::Long IDOfNode);
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
  void SplitVolumesIntoTetra     (SMESH::SMESH_IDSource_ptr elems,
                                  CORBA::Short methodFlags) throw (SALOME::SALOME_Exception);

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
  void ConvertToQuadraticObject(CORBA::Boolean            theForce3d,
                                SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);
  void ConvertFromQuadraticObject(SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);

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
  void ExtrusionSweep0D(const SMESH::long_array & IDsOfElements,
                      const SMESH::DirStruct &  StepVector,
                      CORBA::Long               NbOfSteps);

  void ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
                            const SMESH::DirStruct &  StepVector,
                            CORBA::Long               NbOfSteps);

  void ExtrusionSweepObject0D(SMESH::SMESH_IDSource_ptr theObject,
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
  SMESH::ListOfGroups* ExtrusionSweepMakeGroups0D(const SMESH::long_array& IDsOfElements,
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
  SMESH::ListOfGroups* ExtrusionSweepObject0DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
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

  // skl 04.06.2009 
  SMESH::ListOfGroups* ExtrusionAlongPathObjX(SMESH::SMESH_IDSource_ptr  Object,
                                              SMESH::SMESH_IDSource_ptr  Path,
                                              CORBA::Long                NodeStart,
                                              CORBA::Boolean             HasAngles,
                                              const SMESH::double_array& Angles,
                                              CORBA::Boolean             LinearVariation,
                                              CORBA::Boolean             HasRefPoint,
                                              const SMESH::PointStruct&  RefPoint,
                                              CORBA::Boolean             MakeGroups,
                                              SMESH::ElementType         ElemType,
                                              SMESH::SMESH_MeshEditor::Extrusion_Error& Error);
  SMESH::ListOfGroups* ExtrusionAlongPathX(const SMESH::long_array&   IDsOfElements,
                                           SMESH::SMESH_IDSource_ptr  Path,
                                           CORBA::Long                NodeStart,
                                           CORBA::Boolean             HasAngles,
                                           const SMESH::double_array& Angles,
                                           CORBA::Boolean             LinearVariation,
                                           CORBA::Boolean             HasRefPoint,
                                           const SMESH::PointStruct&  RefPoint,
                                           CORBA::Boolean             MakeGroups,
                                           SMESH::ElementType         ElemType,
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

  void Scale(SMESH::SMESH_IDSource_ptr  theObject,
             const SMESH::PointStruct&  thePoint,
             const SMESH::double_array& theScaleFact,
             CORBA::Boolean             theCopy);

  SMESH::ListOfGroups* ScaleMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                       const SMESH::PointStruct&  thePoint,
                                       const SMESH::double_array& theScaleFact);

  SMESH::SMESH_Mesh_ptr ScaleMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                      const SMESH::PointStruct& Point,
                                      const SMESH::double_array& theScaleFact,
                                      CORBA::Boolean            CopyGroups,
                                      const char*               MeshName);

  void FindCoincidentNodes (CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes);
  void FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      Object,
                                 CORBA::Double                  Tolerance,
                                 SMESH::array_of_long_array_out GroupsOfNodes);
  void FindCoincidentNodesOnPartBut(SMESH::SMESH_IDSource_ptr      Object,
                                    CORBA::Double                  Tolerance,
                                    SMESH::array_of_long_array_out GroupsOfNodes,
                                    const SMESH::ListOfIDSources&  ExceptSubMeshOrGroups);
  void MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes);
  void FindEqualElements(SMESH::SMESH_IDSource_ptr      Object,
                         SMESH::array_of_long_array_out GroupsOfElementsID);
  void MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID);
  void MergeEqualElements();
  CORBA::Long MoveClosestNodeToPoint(CORBA::Double x,
                                     CORBA::Double y,
                                     CORBA::Double z,
                                     CORBA::Long   nodeID);
  /*!
   * \brief Return ID of node closest to a given point
   */
  CORBA::Long FindNodeClosestTo(CORBA::Double x,
                                CORBA::Double y,
                                CORBA::Double z);
  /*!
   * Return elements of given type where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::long_array* FindElementsByPoint(CORBA::Double      x,
                                         CORBA::Double      y,
                                         CORBA::Double      z,
                                         SMESH::ElementType type);
  /*!
   * Searching among the given elements, return elements of given type 
   * where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::long_array* FindAmongElementsByPoint(SMESH::SMESH_IDSource_ptr elements,
                                              CORBA::Double             x,
                                              CORBA::Double             y,
                                              CORBA::Double             z,
                                              SMESH::ElementType        type);

  /*!
   * Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
   * TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
   */
  CORBA::Short GetPointState(CORBA::Double x, CORBA::Double y, CORBA::Double z);

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
  CORBA::Boolean DoubleNodeElem( const SMESH::long_array& theElems, 
                                 const SMESH::long_array& theNodesNot,
                                 const SMESH::long_array& theAffectedElems );

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
  CORBA::Boolean DoubleNodeElemInRegion( const SMESH::long_array& theElems, 
                                         const SMESH::long_array& theNodesNot,
                                         GEOM::GEOM_Object_ptr    theShape );

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
   * \brief Double nodes on shared faces between groups of volumes and create flat elements on demand.
   * The list of groups must describe a partition of the mesh volumes.
   * The nodes of the internal faces at the boundaries of the groups are doubled.
   * In option, the internal faces are replaced by flat elements.
   * Triangles are transformed in prisms, and quadrangles in hexahedrons.
   * @param theDomains - list of groups of volumes
   * @param createJointElems - if TRUE, create the elements
   * @return TRUE if operation has been completed successfully, FALSE otherwise
   */
  CORBA::Boolean DoubleNodesOnGroupBoundaries( const SMESH::ListOfGroups& theDomains,
                                               CORBA::Boolean createJointElems )
    throw (SALOME::SALOME_Exception);
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
                                   SMESH::SMESH_Group_out group)
    throw (SALOME::SALOME_Exception);

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
  void initData(bool deleteSearchers=true);

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
                                      bool                      MakeGroups,
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
  SMESH::ListOfGroups* extrusionAlongPathX(const SMESH::long_array &  IDsOfElements,
                                           SMESH::SMESH_IDSource_ptr  Path,
                                           CORBA::Long                NodeStart,
                                           CORBA::Boolean             HasAngles,
                                           const SMESH::double_array& Angles,
                                           CORBA::Boolean             LinearVariation,
                                           CORBA::Boolean             HasRefPoint,
                                           const SMESH::PointStruct&  RefPoint,
                                           bool                       MakeGroups,
                                           const SMDSAbs_ElementType  ElementType,
                                           SMESH::SMESH_MeshEditor::Extrusion_Error & theError);
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

  SMESH::SMESH_Mesh_ptr makeMesh(const char* theMeshName);

  void DumpGroupsList(SMESH::TPythonDump & theDumpPython, 
                      const SMESH::ListOfGroups * theGroupList);

  string generateGroupName(const string& thePrefix);

private: //!< fields

  SMESH_Mesh_i*         myMesh_i;
  SMESH_Mesh *          myMesh;

  SMESH::long_array_var myLastCreatedElems;
  SMESH::long_array_var myLastCreatedNodes;

  SMESH::MeshPreviewStruct_var myPreviewData;
  bool                         myPreviewMode;
};

#endif
