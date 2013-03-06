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

class SMESH_Mesh_i;

namespace MeshEditor_I {
  struct TPreviewMesh;
}

class SMESH_MeshEditor_i: public POA_SMESH::SMESH_MeshEditor
{
public:
  SMESH_MeshEditor_i(SMESH_Mesh_i * theMesh, bool isPreview);

  virtual ~ SMESH_MeshEditor_i();
  /*!
   * \brief Return edited mesh ID
   * \retval int - mesh ID
   */
  int GetMeshId() const { return myMesh->GetId(); }

  // --- CORBA

  /*!
   * Return data of mesh edition preview
   */
  SMESH::MeshPreviewStruct* GetPreviewData() throw (SALOME::SALOME_Exception);
  /*!
   * If during last operation of MeshEditor some nodes were
   * created this method returns list of their IDs, if new nodes
   * not created - returns an empty list
   */
  SMESH::long_array* GetLastCreatedNodes() throw (SALOME::SALOME_Exception);
  /*!
   * If during last operation of MeshEditor some elements were
   * created this method returns list of their IDs, if new elements
   * not created - returns an empty list
   */
  SMESH::long_array* GetLastCreatedElems() throw (SALOME::SALOME_Exception);
  /*!
   * \brief Returns description of an error/warning occured during the last operation
   */
  SMESH::ComputeError* GetLastError() throw (SALOME::SALOME_Exception);

  /*!
   * \brief Wrap a sequence of ids in a SMESH_IDSource
   */
  SMESH::SMESH_IDSource_ptr MakeIDSource(const SMESH::long_array& IDsOfElements,
                                         SMESH::ElementType       type);
  static bool               IsTemporaryIDSource( SMESH::SMESH_IDSource_ptr& idSource );

  CORBA::Boolean RemoveElements(const SMESH::long_array & IDsOfElements)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean RemoveNodes   (const SMESH::long_array & IDsOfNodes)
    throw (SALOME::SALOME_Exception);
  CORBA::Long    RemoveOrphanNodes()
    throw (SALOME::SALOME_Exception);

  /*!
   * Methods for creation new elements.
   * Returns ID of created element or 0 if element not created
   */
  CORBA::Long AddNode(CORBA::Double x, CORBA::Double y, CORBA::Double z)
    throw (SALOME::SALOME_Exception);
  CORBA::Long Add0DElement(CORBA::Long IDOfNode)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddBall(CORBA::Long IDOfNodem, CORBA::Double diameter)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddEdge(const SMESH::long_array & IDsOfNodes)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddFace(const SMESH::long_array & IDsOfNodes)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddPolygonalFace(const SMESH::long_array & IDsOfNodes)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddVolume(const SMESH::long_array & IDsOfNodes)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddPolyhedralVolume(const SMESH::long_array & IDsOfNodes,
                                  const SMESH::long_array & Quantities)
    throw (SALOME::SALOME_Exception);
  CORBA::Long AddPolyhedralVolumeByFaces(const SMESH::long_array & IdsOfFaces)
    throw (SALOME::SALOME_Exception);

  /*!
   * \brief Create 0D elements on all nodes of the given object except those 
   *        nodes on which a 0D element already exists.
   *  \param theObject object on whose nodes 0D elements will be created.
   *  \param theGroupName optional name of a group to add 0D elements created
   *         and/or found on nodes of \a theObject.
   *  \return an object (a new group or a temporary SMESH_IDSource) holding
   *          ids of new and/or found 0D elements.
   */
  SMESH::SMESH_IDSource_ptr Create0DElementsOnAllNodes(SMESH::SMESH_IDSource_ptr theObject,
                                                       const char*               theGroupName)
    throw (SALOME::SALOME_Exception);

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
                          CORBA::Double x, CORBA::Double y, CORBA::Double z)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean InverseDiag(CORBA::Long NodeID1, CORBA::Long NodeID2)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean DeleteDiag(CORBA::Long NodeID1, CORBA::Long NodeID2)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean Reorient(const SMESH::long_array & IDsOfElements)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean ReorientObject(SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);

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
                         const SMESH::PointStruct& thePoint) throw (SALOME::SALOME_Exception);

  // Split/Join faces
  CORBA::Boolean TriToQuad       (const SMESH::long_array &   IDsOfElements,
                                  SMESH::NumericalFunctor_ptr Criterion,
                                  CORBA::Double               MaxAngle)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  SMESH::NumericalFunctor_ptr Criterion,
                                  CORBA::Double               MaxAngle)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean QuadToTri       (const SMESH::long_array &   IDsOfElements,
                                  SMESH::NumericalFunctor_ptr Criterion)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean QuadToTriObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  SMESH::NumericalFunctor_ptr Criterion)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean SplitQuad       (const SMESH::long_array &   IDsOfElements,
                                  CORBA::Boolean              Diag13)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean SplitQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                  CORBA::Boolean              Diag13)
    throw (SALOME::SALOME_Exception);
  CORBA::Long    BestSplit       (CORBA::Long                 IDOfQuad,
                                  SMESH::NumericalFunctor_ptr Criterion)
    throw (SALOME::SALOME_Exception);
  void           SplitVolumesIntoTetra(SMESH::SMESH_IDSource_ptr elems,
                                       CORBA::Short             methodFlags)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean Smooth(const SMESH::long_array &              IDsOfElements,
                        const SMESH::long_array &              IDsOfFixedNodes,
                        CORBA::Long                            MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                              const SMESH::long_array &              IDsOfFixedNodes,
                              CORBA::Long                            MaxNbOfIterations,
                              CORBA::Double                          MaxAspectRatio,
                              SMESH::SMESH_MeshEditor::Smooth_Method Method)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean SmoothParametric(const SMESH::long_array &              IDsOfElements,
                                  const SMESH::long_array &              IDsOfFixedNodes,
                                  CORBA::Long                            MaxNbOfIterations,
                                  CORBA::Double                          MaxAspectRatio,
                                  SMESH::SMESH_MeshEditor::Smooth_Method Method)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean SmoothParametricObject(SMESH::SMESH_IDSource_ptr              theObject,
                                        const SMESH::long_array &              IDsOfFixedNodes,
                                        CORBA::Long                            MaxNbOfIterations,
                                        CORBA::Double                          MaxAspectRatio,
                                        SMESH::SMESH_MeshEditor::Smooth_Method Method)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean smooth(const SMESH::long_array &              IDsOfElements,
                        const SMESH::long_array &              IDsOfFixedNodes,
                        CORBA::Long                            MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method,
                        bool                                   IsParametric)
    throw (SALOME::SALOME_Exception);
  CORBA::Boolean smoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                              const SMESH::long_array &              IDsOfFixedNodes,
                              CORBA::Long                            MaxNbOfIterations,
                              CORBA::Double                          MaxAspectRatio,
                              SMESH::SMESH_MeshEditor::Smooth_Method Method,
                              bool                                   IsParametric)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean ConvertFromQuadratic()
    throw (SALOME::SALOME_Exception);
  void           ConvertFromQuadraticObject(SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);
  void           ConvertToQuadratic(CORBA::Boolean Force3d)
    throw (SALOME::SALOME_Exception);
  void           ConvertToQuadraticObject(CORBA::Boolean            theForce3d,
                                          SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);
  void           ConvertToBiQuadratic(CORBA::Boolean            theForce3d,
                                      SMESH::SMESH_IDSource_ptr theObject)
    throw (SALOME::SALOME_Exception);

  void RenumberNodes() throw (SALOME::SALOME_Exception);
  void RenumberElements() throw (SALOME::SALOME_Exception);

  void RotationSweep(const SMESH::long_array & IDsOfElements,
                     const SMESH::AxisStruct & Axis,
                     CORBA::Double             AngleInRadians,
                     CORBA::Long               NbOfSteps,
                     CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  void RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
                           const SMESH::AxisStruct & Axis,
                           CORBA::Double             AngleInRadians,
                           CORBA::Long               NbOfSteps,
                           CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  void RotationSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                             const SMESH::AxisStruct & Axis,
                             CORBA::Double             AngleInRadians,
                             CORBA::Long               NbOfSteps,
                             CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  void RotationSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                             const SMESH::AxisStruct & Axis,
                             CORBA::Double             AngleInRadians,
                             CORBA::Long               NbOfSteps,
                             CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);

  void ExtrusionSweep(const SMESH::long_array & IDsOfElements,
                      const SMESH::DirStruct &  StepVector,
                      CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  void ExtrusionSweep0D(const SMESH::long_array & IDsOfElements,
                      const SMESH::DirStruct &  StepVector,
                      CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);

  void ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
                            const SMESH::DirStruct &  StepVector,
                            CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);

  void ExtrusionSweepObject0D(SMESH::SMESH_IDSource_ptr theObject,
                              const SMESH::DirStruct &  StepVector,
                              CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  void ExtrusionSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                              const SMESH::DirStruct &  StepVector,
                              CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  void ExtrusionSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                              const SMESH::DirStruct &  StepVector,
                              CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  void AdvancedExtrusion(const SMESH::long_array & theIDsOfElements,
                         const SMESH::DirStruct &  theStepVector,
                         CORBA::Long               theNbOfSteps,
                         CORBA::Long               theExtrFlags,
                         CORBA::Double             theSewTolerance)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor::Extrusion_Error
  ExtrusionAlongPath(const SMESH::long_array &   IDsOfElements,
                     SMESH::SMESH_Mesh_ptr       PathMesh,
                     GEOM::GEOM_Object_ptr       PathShape,
                     CORBA::Long                 NodeStart,
                     CORBA::Boolean              HasAngles,
                     const SMESH::double_array & Angles,
                     CORBA::Boolean              HasRefPoint,
                     const SMESH::PointStruct &  RefPoint)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor::Extrusion_Error
  ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
                           SMESH::SMESH_Mesh_ptr       PathMesh,
                           GEOM::GEOM_Object_ptr       PathShape,
                           CORBA::Long                 NodeStart,
                           CORBA::Boolean              HasAngles,
                           const SMESH::double_array & Angles,
                           CORBA::Boolean              HasRefPoint,
                           const SMESH::PointStruct &  RefPoint)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_MeshEditor::Extrusion_Error
  ExtrusionAlongPathObject1D(SMESH::SMESH_IDSource_ptr   theObject,
                             SMESH::SMESH_Mesh_ptr       PathMesh,
                             GEOM::GEOM_Object_ptr       PathShape,
                             CORBA::Long                 NodeStart,
                             CORBA::Boolean              HasAngles,
                             const SMESH::double_array & Angles,
                             CORBA::Boolean              HasRefPoint,
                             const SMESH::PointStruct &  RefPoint)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_MeshEditor::Extrusion_Error
  ExtrusionAlongPathObject2D(SMESH::SMESH_IDSource_ptr   theObject,
                             SMESH::SMESH_Mesh_ptr       PathMesh,
                             GEOM::GEOM_Object_ptr       PathShape,
                             CORBA::Long                 NodeStart,
                             CORBA::Boolean              HasAngles,
                             const SMESH::double_array & Angles,
                             CORBA::Boolean              HasRefPoint,
                             const SMESH::PointStruct &  RefPoint)
    throw (SALOME::SALOME_Exception);

  SMESH::double_array* LinearAnglesVariation(SMESH::SMESH_Mesh_ptr       PathMesh,
                                             GEOM::GEOM_Object_ptr       PathShape,
                                             const SMESH::double_array & Angles);

  void Mirror(const SMESH::long_array &           IDsOfElements,
              const SMESH::AxisStruct &           Axis,
              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
              CORBA::Boolean                      Copy)
    throw (SALOME::SALOME_Exception);
  void MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
                    const SMESH::AxisStruct &           Axis,
                    SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                    CORBA::Boolean                      Copy)
    throw (SALOME::SALOME_Exception);
  void Translate(const SMESH::long_array & IDsOfElements,
                 const SMESH::DirStruct &   Vector,
                 CORBA::Boolean            Copy)
    throw (SALOME::SALOME_Exception);
  void TranslateObject(SMESH::SMESH_IDSource_ptr  theObject,
                       const SMESH::DirStruct &   Vector,
                       CORBA::Boolean             Copy)
    throw (SALOME::SALOME_Exception);
  void Rotate(const SMESH::long_array & IDsOfElements,
              const SMESH::AxisStruct &  Axis,
              CORBA::Double             Angle,
              CORBA::Boolean            Copy)
    throw (SALOME::SALOME_Exception);
  void RotateObject(SMESH::SMESH_IDSource_ptr  theObject,
                    const SMESH::AxisStruct &  Axis,
                    CORBA::Double              Angle,
                    CORBA::Boolean             Copy)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* RotationSweepMakeGroups(const SMESH::long_array& IDsOfElements,
                                               const SMESH::AxisStruct& Axix,
                                               CORBA::Double            AngleInRadians,
                                               CORBA::Long              NbOfSteps,
                                               CORBA::Double            Tolerance)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* RotationSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                     const SMESH::AxisStruct&  Axix,
                                                     CORBA::Double             AngleInRadians,
                                                     CORBA::Long               NbOfSteps,
                                                     CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* RotationSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                       const SMESH::AxisStruct&  Axix,
                                                       CORBA::Double             AngleInRadians,
                                                       CORBA::Long               NbOfSteps,
                                                       CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* RotationSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                       const SMESH::AxisStruct&  Axix,
                                                       CORBA::Double             AngleInRadians,
                                                       CORBA::Long               NbOfSteps,
                                                       CORBA::Double             Tolerance)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepMakeGroups(const SMESH::long_array& IDsOfElements,
                                                const SMESH::DirStruct&  StepVector,
                                                CORBA::Long              NbOfSteps)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepMakeGroups0D(const SMESH::long_array& IDsOfElements,
                                                const SMESH::DirStruct&  StepVector,
                                                CORBA::Long              NbOfSteps)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* AdvancedExtrusionMakeGroups(const SMESH::long_array& IDsOfElements,
                                                   const SMESH::DirStruct&  StepVector,
                                                   CORBA::Long              NbOfSteps,
                                                   CORBA::Long              ExtrFlags,
                                                   CORBA::Double            SewTolerance)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                      const SMESH::DirStruct&   StepVector,
                                                      CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepObject0DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                        const SMESH::DirStruct&   StepVector,
                                                        CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepObject1DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                        const SMESH::DirStruct&   StepVector,
                                                        CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionSweepObject2DMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                        const SMESH::DirStruct&   StepVector,
                                                        CORBA::Long               NbOfSteps)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionAlongPathMakeGroups(const SMESH::long_array&   IDsOfElements,
                                                    SMESH::SMESH_Mesh_ptr      PathMesh,
                                                    GEOM::GEOM_Object_ptr      PathShape,
                                                    CORBA::Long                NodeStart,
                                                    CORBA::Boolean             HasAngles,
                                                    const SMESH::double_array& Angles,
                                                    CORBA::Boolean             HasRefPoint,
                                                    const SMESH::PointStruct&  RefPoint,
                                                    SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionAlongPathObjectMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
                                                          SMESH::SMESH_Mesh_ptr      PathMesh,
                                                          GEOM::GEOM_Object_ptr      PathShape,
                                                          CORBA::Long                NodeStart,
                                                          CORBA::Boolean             HasAngles,
                                                          const SMESH::double_array& Angles,
                                                          CORBA::Boolean             HasRefPoint,
                                                          const SMESH::PointStruct&  RefPoint,
                                                          SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionAlongPathObject1DMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
                                                            SMESH::SMESH_Mesh_ptr      PathMesh,
                                                            GEOM::GEOM_Object_ptr      PathShape,
                                                            CORBA::Long                NodeStart,
                                                            CORBA::Boolean             HasAngles,
                                                            const SMESH::double_array& Angles,
                                                            CORBA::Boolean             HasRefPoint,
                                                            const SMESH::PointStruct&  RefPoint,
                                                            SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* ExtrusionAlongPathObject2DMakeGroups(SMESH::SMESH_IDSource_ptr  Object,
                                                            SMESH::SMESH_Mesh_ptr      PathMesh,
                                                            GEOM::GEOM_Object_ptr      PathShape,
                                                            CORBA::Long                NodeStart,
                                                            CORBA::Boolean             HasAngles,
                                                            const SMESH::double_array& Angles,
                                                            CORBA::Boolean             HasRefPoint,
                                                            const SMESH::PointStruct&  RefPoint,
                                                            SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);

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
                                              SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);
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
                                           SMESH::SMESH_MeshEditor::Extrusion_Error& Error)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* MirrorMakeGroups(const SMESH::long_array&            IDsOfElements,
                                        const SMESH::AxisStruct&            Mirror,
                                        SMESH::SMESH_MeshEditor::MirrorType MirrorType)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* MirrorObjectMakeGroups(SMESH::SMESH_IDSource_ptr           Object,
                                              const SMESH::AxisStruct&            Mirror,
                                              SMESH::SMESH_MeshEditor::MirrorType MirrorType)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* TranslateMakeGroups(const SMESH::long_array& IDsOfElements,
                                           const SMESH::DirStruct&  Vector)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* TranslateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                                 const SMESH::DirStruct&   Vector)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* RotateMakeGroups(const SMESH::long_array& IDsOfElements,
                                        const SMESH::AxisStruct& Axis,
                                        CORBA::Double            AngleInRadians)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* RotateObjectMakeGroups(SMESH::SMESH_IDSource_ptr Object,
                                              const SMESH::AxisStruct&  Axis,
                                              CORBA::Double             AngleInRadians)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Mesh_ptr MirrorMakeMesh(const SMESH::long_array&            IDsOfElements,
                                       const SMESH::AxisStruct&            Mirror,
                                       SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                       CORBA::Boolean                      CopyGroups,
                                       const char*                         MeshName)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_Mesh_ptr MirrorObjectMakeMesh(SMESH::SMESH_IDSource_ptr           Object,
                                             const SMESH::AxisStruct&            Mirror,
                                             SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                                             CORBA::Boolean                      CopyGroups,
                                             const char*                         MeshName)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_Mesh_ptr TranslateMakeMesh(const SMESH::long_array& IDsOfElements,
                                          const SMESH::DirStruct&  Vector,
                                          CORBA::Boolean           CopyGroups,
                                          const char*              MeshName)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_Mesh_ptr TranslateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                                const SMESH::DirStruct&   Vector,
                                                CORBA::Boolean            CopyGroups,
                                                const char*               MeshName)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_Mesh_ptr RotateMakeMesh(const SMESH::long_array& IDsOfElements,
                                       const SMESH::AxisStruct& Axis,
                                       CORBA::Double            AngleInRadians,
                                       CORBA::Boolean           CopyGroups,
                                       const char*              MeshName)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_Mesh_ptr RotateObjectMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                             const SMESH::AxisStruct&  Axis,
                                             CORBA::Double             AngleInRadians,
                                             CORBA::Boolean            CopyGroups,
                                             const char*               MeshName)
    throw (SALOME::SALOME_Exception);

  void Scale(SMESH::SMESH_IDSource_ptr  theObject,
             const SMESH::PointStruct&  thePoint,
             const SMESH::double_array& theScaleFact,
             CORBA::Boolean             theCopy)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* ScaleMakeGroups(SMESH::SMESH_IDSource_ptr  theObject,
                                       const SMESH::PointStruct&  thePoint,
                                       const SMESH::double_array& theScaleFact)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Mesh_ptr ScaleMakeMesh(SMESH::SMESH_IDSource_ptr Object,
                                      const SMESH::PointStruct& Point,
                                      const SMESH::double_array& theScaleFact,
                                      CORBA::Boolean            CopyGroups,
                                      const char*               MeshName)
    throw (SALOME::SALOME_Exception);

  void FindCoincidentNodes (CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes)
    throw (SALOME::SALOME_Exception);
  void FindCoincidentNodesOnPart(SMESH::SMESH_IDSource_ptr      Object,
                                 CORBA::Double                  Tolerance,
                                 SMESH::array_of_long_array_out GroupsOfNodes)
    throw (SALOME::SALOME_Exception);
  void FindCoincidentNodesOnPartBut(SMESH::SMESH_IDSource_ptr      Object,
                                    CORBA::Double                  Tolerance,
                                    SMESH::array_of_long_array_out GroupsOfNodes,
                                    const SMESH::ListOfIDSources&  ExceptSubMeshOrGroups)
    throw (SALOME::SALOME_Exception);
  void MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes)
    throw (SALOME::SALOME_Exception);
  void FindEqualElements(SMESH::SMESH_IDSource_ptr      Object,
                         SMESH::array_of_long_array_out GroupsOfElementsID)
    throw (SALOME::SALOME_Exception);
  void MergeElements(const SMESH::array_of_long_array& GroupsOfElementsID)
    throw (SALOME::SALOME_Exception);
  void MergeEqualElements()
    throw (SALOME::SALOME_Exception);
  CORBA::Long MoveClosestNodeToPoint(CORBA::Double x,
                                     CORBA::Double y,
                                     CORBA::Double z,
                                     CORBA::Long   nodeID)
    throw (SALOME::SALOME_Exception);
  /*!
   * \brief Return ID of node closest to a given point
   */
  CORBA::Long FindNodeClosestTo(CORBA::Double x,
                                CORBA::Double y,
                                CORBA::Double z)
    throw (SALOME::SALOME_Exception);
  /*!
   * Return elements of given type where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::long_array* FindElementsByPoint(CORBA::Double      x,
                                         CORBA::Double      y,
                                         CORBA::Double      z,
                                         SMESH::ElementType type)
    throw (SALOME::SALOME_Exception);
  /*!
   * Searching among the given elements, return elements of given type 
   * where the given point is IN or ON.
   * 'ALL' type means elements of any type excluding nodes
   */
  SMESH::long_array* FindAmongElementsByPoint(SMESH::SMESH_IDSource_ptr elements,
                                              CORBA::Double             x,
                                              CORBA::Double             y,
                                              CORBA::Double             z,
                                              SMESH::ElementType        type)
    throw (SALOME::SALOME_Exception);

  /*!
   * Return point state in a closed 2D mesh in terms of TopAbs_State enumeration.
   * TopAbs_UNKNOWN state means that either mesh is wrong or the analysis fails.
   */
  CORBA::Short GetPointState(CORBA::Double x, CORBA::Double y, CORBA::Double z)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor::Sew_Error
  SewFreeBorders(CORBA::Long FirstNodeID1,
                 CORBA::Long SecondNodeID1,
                 CORBA::Long LastNodeID1,
                 CORBA::Long FirstNodeID2,
                 CORBA::Long SecondNodeID2,
                 CORBA::Long LastNodeID2,
                 CORBA::Boolean CreatePolygons,
                 CORBA::Boolean CreatePolyedrs)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewConformFreeBorders(CORBA::Long FirstNodeID1,
                        CORBA::Long SecondNodeID1,
                        CORBA::Long LastNodeID1,
                        CORBA::Long FirstNodeID2,
                        CORBA::Long SecondNodeID2)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                  CORBA::Long SecondNodeIDOnFreeBorder,
                  CORBA::Long LastNodeIDOnFreeBorder,
                  CORBA::Long FirstNodeIDOnSide,
                  CORBA::Long LastNodeIDOnSide,
                  CORBA::Boolean CreatePolygons,
                  CORBA::Boolean CreatePolyedrs)
    throw (SALOME::SALOME_Exception);
  SMESH::SMESH_MeshEditor::Sew_Error
  SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                  const SMESH::long_array& IDsOfSide2Elements,
                  CORBA::Long NodeID1OfSide1ToMerge,
                  CORBA::Long NodeID1OfSide2ToMerge,
                  CORBA::Long NodeID2OfSide1ToMerge,
                  CORBA::Long NodeID2OfSide2ToMerge)
    throw (SALOME::SALOME_Exception);

  /*!
   * Set new nodes for given element.
   * If number of nodes is not corresponded to type of
   * element - returns false
   */
  CORBA::Boolean ChangeElemNodes(CORBA::Long ide, const SMESH::long_array& newIDs)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean DoubleNodes( const SMESH::long_array& theNodes,
                              const SMESH::long_array& theModifiedElems )
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean DoubleNode( CORBA::Long theNodeId,
                             const SMESH::long_array& theModifiedElems )
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean DoubleNodeGroup( SMESH::SMESH_GroupBase_ptr theNodes,
                                  SMESH::SMESH_GroupBase_ptr theModifiedElems )
    throw (SALOME::SALOME_Exception);

  /*!
   * \brief Creates a hole in a mesh by doubling the nodes of some particular elements.
   * Works as DoubleNodeGroup(), but returns a new group with newly created nodes.
   * \param theNodes - group of nodes to be doubled.
   * \param theModifiedElems - group of elements to be updated.
   * \return a new group with newly created nodes
   * \sa DoubleNodeGroup()
   */
  SMESH::SMESH_Group_ptr DoubleNodeGroupNew( SMESH::SMESH_GroupBase_ptr theNodes,
                                             SMESH::SMESH_GroupBase_ptr theModifiedElems )
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean DoubleNodeGroups( const SMESH::ListOfGroups& theNodes,
                                   const SMESH::ListOfGroups& theModifiedElems )
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Group_ptr DoubleNodeGroupsNew( const SMESH::ListOfGroups& theNodes,
                                              const SMESH::ListOfGroups& theModifiedElems )
    throw (SALOME::SALOME_Exception);

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
                                 const SMESH::long_array& theAffectedElems )
    throw (SALOME::SALOME_Exception);

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
                                         GEOM::GEOM_Object_ptr    theShape )
    throw (SALOME::SALOME_Exception);

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
                                      SMESH::SMESH_GroupBase_ptr theAffectedElems )
    throw (SALOME::SALOME_Exception);

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
                                                 SMESH::SMESH_GroupBase_ptr theAffectedElems )
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups*   DoubleNodeElemGroup2New(SMESH::SMESH_GroupBase_ptr theElems,
                                                 SMESH::SMESH_GroupBase_ptr theNodesNot,
                                                 SMESH::SMESH_GroupBase_ptr theAffectedElems,
                                                 CORBA::Boolean             theElemGroupNeeded,
                                                 CORBA::Boolean             theNodeGroupNeeded)
    throw (SALOME::SALOME_Exception);
  
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
                                              GEOM::GEOM_Object_ptr      theShape )
    throw (SALOME::SALOME_Exception);

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
                                       const SMESH::ListOfGroups& theAffectedElems )
    throw (SALOME::SALOME_Exception);

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
                                                  const SMESH::ListOfGroups& theAffectedElems )
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups*   DoubleNodeElemGroups2New(const SMESH::ListOfGroups& theElems,
                                                  const SMESH::ListOfGroups& theNodesNot,
                                                  const SMESH::ListOfGroups& theAffectedElems,
                                                  CORBA::Boolean             theElemGroupNeeded,
                                                  CORBA::Boolean             theNodeGroupNeeded)
    throw (SALOME::SALOME_Exception);

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
                                               GEOM::GEOM_Object_ptr      theShape )
    throw (SALOME::SALOME_Exception);

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
                                                   GEOM::GEOM_Object_ptr      theShape )
    throw (SALOME::SALOME_Exception);

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
  CORBA::Boolean CreateFlatElementsOnFacesGroups( const SMESH::ListOfGroups& theGroupsOfFaces )
    throw (SALOME::SALOME_Exception);

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
                      SMESH::array_of_long_array_out GroupsOfNodes)
  throw (SALOME::SALOME_Exception);

  /*!
   * \brief Generated skin mesh (containing 2D cells) from 3D mesh
   * The created 2D mesh elements based on nodes of free faces of boundary volumes
   * \return TRUE if operation has been completed successfully, FALSE otherwise
   */
  CORBA::Boolean Make2DMeshFrom3D()
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Mesh_ptr MakeBoundaryMesh(SMESH::SMESH_IDSource_ptr elements,
                                         SMESH::Bnd_Dimension      dimension,
                                         const char*               groupName,
                                         const char*               meshName,
                                         CORBA::Boolean            toCopyElements,
                                         CORBA::Boolean            toCopyMissingBondary,
                                         SMESH::SMESH_Group_out    group)
    throw (SALOME::SALOME_Exception);

  CORBA::Long MakeBoundaryElements(SMESH::Bnd_Dimension dimension,
                                   const char* groupName,
                                   const char* meshName,
                                   CORBA::Boolean toCopyAll,
                                   const SMESH::ListOfIDSources& groups,
                                   SMESH::SMESH_Mesh_out mesh,
                                   SMESH::SMESH_Group_out group)
    throw (SALOME::SALOME_Exception);

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
  SMESH::ListOfGroups* getGroups(const std::list<int>* groupIDs)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* rotationSweep(const SMESH::long_array & IDsOfElements,
                                     const SMESH::AxisStruct & Axis,
                                     CORBA::Double             AngleInRadians,
                                     CORBA::Long               NbOfSteps,
                                     CORBA::Double             Tolerance,
                                     const bool                MakeGroups,
                                     const SMDSAbs_ElementType ElementType=SMDSAbs_All)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* extrusionSweep(const SMESH::long_array & IDsOfElements,
                                      const SMESH::DirStruct &  StepVector,
                                      CORBA::Long               NbOfSteps,
                                      bool                      MakeGroups,
                                      const SMDSAbs_ElementType ElementType=SMDSAbs_All)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* advancedExtrusion(const SMESH::long_array & theIDsOfElements,
                                         const SMESH::DirStruct &  theStepVector,
                                         CORBA::Long               theNbOfSteps,
                                         CORBA::Long               theExtrFlags,
                                         CORBA::Double             theSewTolerance,
                                         const bool                MakeGroups)
    throw (SALOME::SALOME_Exception);
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
                                          const SMDSAbs_ElementType   ElementType=SMDSAbs_All)
    throw (SALOME::SALOME_Exception);
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
                                           SMESH::SMESH_MeshEditor::Extrusion_Error & theError)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* mirror(TIDSortedElemSet &                  IDsOfElements,
                              const SMESH::AxisStruct &           Axis,
                              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
                              CORBA::Boolean                      Copy,
                              bool                                MakeGroups,
                              ::SMESH_Mesh*                       TargetMesh=0)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* translate(TIDSortedElemSet        & IDsOfElements,
                                 const SMESH::DirStruct &  Vector,
                                 CORBA::Boolean            Copy,
                                 bool                      MakeGroups,
                                 ::SMESH_Mesh*             TargetMesh=0)
    throw (SALOME::SALOME_Exception);
  SMESH::ListOfGroups* rotate(TIDSortedElemSet &           IDsOfElements,
                              const SMESH::AxisStruct &  Axis,
                              CORBA::Double             Angle,
                              CORBA::Boolean            Copy,
                              bool                      MakeGroups,
                              ::SMESH_Mesh*             TargetMesh=0)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfGroups* scale(SMESH::SMESH_IDSource_ptr   theObject,
                             const SMESH::PointStruct&   thePoint,
                             const SMESH::double_array&  theScaleFact,
                             CORBA::Boolean              theCopy,
                             bool                        theMakeGroups,
                             ::SMESH_Mesh*               theTargetMesh=0)
    throw (SALOME::SALOME_Exception);

  void convertToQuadratic(CORBA::Boolean            theForce3d,
                          CORBA::Boolean            theToBiQuad,
                          SMESH::SMESH_IDSource_ptr theObject = SMESH::SMESH_IDSource::_nil())
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_Mesh_ptr makeMesh(const char* theMeshName);

  void dumpGroupsList(SMESH::TPythonDump & theDumpPython, 
                      const SMESH::ListOfGroups * theGroupList);

  string generateGroupName(const string& thePrefix);

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
  std::list< _IDSource* >      myAuxIDSources;
  void                         deleteAuxIDSources();
};

#endif
