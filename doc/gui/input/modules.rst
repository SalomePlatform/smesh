########################
Structured documentation
########################

.. contents::

.. currentmodule:: smeshBuilder

***************
Creating meshes
***************

.. autosummary::

   smeshBuilder.Concatenate
   smeshBuilder.CopyMesh

Importing and exporting meshes
==============================

.. autosummary::

   smeshBuilder.CreateMeshesFromUNV
   smeshBuilder.CreateMeshesFromMED
   smeshBuilder.CreateMeshesFromSTL
   smeshBuilder.CreateMeshesFromCGNS
   smeshBuilder.CreateMeshesFromGMF
   Mesh.ExportMED
   Mesh.ExportUNV
   Mesh.ExportSTL
   Mesh.ExportCGNS
   Mesh.ExportDAT
   Mesh.ExportGMF


Constructing meshes
===================

.. autosummary::

   smeshBuilder.Mesh
   Mesh.SetName
   Mesh.SetMeshOrder
   Mesh.Compute
   Mesh.Evaluate
   Mesh.Clear
   Mesh.GetMesh
   Mesh.GetShape
   Mesh.HasShapeToMesh
   Mesh.GetComputeErrors
   Mesh.GetAlgoState
   Mesh.GetFailedShapes
   Mesh.GetMeshOrder

Defining Algorithms
===================

.. autosummary::

   Mesh.Segment
   Mesh.Triangle
   Mesh.Quadrangle
   Mesh.Hexahedron
   Mesh.Prism
   Mesh.BodyFitted
   Mesh.Projection1D
   Mesh.Projection2D
   Mesh.Projection1D2D
   Mesh.Projection3D
   Mesh.UseExisting1DElements
   Mesh.UseExisting2DElements
   Mesh.UseExistingSegments
   Mesh.UseExistingFaces
   Mesh.AutomaticTetrahedralization
   Mesh.AutomaticHexahedralization
   smesh_algorithm.Mesh_Algorithm

Defining hypotheses
===================

.. currentmodule:: StdMeshersBuilder

1D Meshing Hypotheses
---------------------

.. autosummary::

   StdMeshersBuilder_Segment.NumberOfSegments
   StdMeshersBuilder_Segment.LocalLength
   StdMeshersBuilder_Segment.MaxSize
   StdMeshersBuilder_Segment.Arithmetic1D
   StdMeshersBuilder_Segment.GeometricProgression
   StdMeshersBuilder_Segment.FixedPoints1D
   StdMeshersBuilder_Segment.StartEndLength
   StdMeshersBuilder_Segment.Adaptive
   StdMeshersBuilder_Segment.Deflection1D
   StdMeshersBuilder_Segment.AutomaticLength
   StdMeshersBuilder_Segment_Python.PythonSplit1D

2D Meshing Hypotheses
---------------------

.. autosummary::

   StdMeshersBuilder_Triangle_MEFISTO.MaxElementArea
   StdMeshersBuilder_Triangle_MEFISTO.LengthFromEdges
   StdMeshersBuilder_Quadrangle.QuadrangleParameters
   StdMeshersBuilder_Quadrangle.QuadranglePreference
   StdMeshersBuilder_Quadrangle.TrianglePreference
   StdMeshersBuilder_Quadrangle.Reduced
   StdMeshersBuilder_Quadrangle.TriangleVertex

Additional Hypotheses
---------------------

.. autosummary::

   smesh_algorithm.Mesh_Algorithm.ViscousLayers
   smesh_algorithm.Mesh_Algorithm.ViscousLayers2D
   StdMeshersBuilder.StdMeshersBuilder_Segment.Propagation
   StdMeshersBuilder.StdMeshersBuilder_Segment.PropagationOfDistribution
   StdMeshersBuilder.StdMeshersBuilder_Segment.QuadraticMesh

Constructing sub-meshes
=======================

.. currentmodule:: smeshBuilder

.. autosummary::

   Mesh.GetSubMesh
   Mesh.ClearSubMesh

Editing Meshes   
==============

.. autosummary::

   Mesh.AddHypothesis
   Mesh.RemoveHypothesis
   Mesh.RemoveGlobalHypotheses
   Mesh.GetHypothesisList
   Mesh.IsUsedHypothesis

*****************
Grouping elements
*****************

.. autosummary::

   Mesh.SetAutoColor
   Mesh.GetAutoColor
   Mesh.HasDuplicatedGroupNamesMED
   
Creating groups
===============

.. autosummary::

   Mesh.CreateEmptyGroup
   Mesh.Group
   Mesh.GroupOnGeom
   Mesh.GroupOnFilter
   Mesh.MakeGroupByIds
   Mesh.MakeGroup
   Mesh.MakeGroupByCriterion
   Mesh.MakeGroupByCriteria
   Mesh.MakeGroupByFilter
   Mesh.FaceGroupsSeparatedByEdges
   Mesh.CreateDimGroup
   Mesh.ConvertToStandalone
   Mesh.GetGroups
   Mesh.NbGroups
   Mesh.GetGroupNames
   Mesh.GetGroupByName


Operations on groups
====================

.. autosummary::

   Mesh.UnionGroups
   Mesh.UnionListOfGroups
   Mesh.IntersectGroups
   Mesh.IntersectListOfGroups
   Mesh.CutGroups
   Mesh.CutListOfGroups

Deleting Groups
===============

.. autosummary::

   Mesh.RemoveGroup
   Mesh.RemoveGroupWithContents

****************
Mesh Information
****************

.. autosummary::

   smeshBuilder.GetMeshInfo
   Mesh.GetEngine
   Mesh.GetGeomEngine
   Mesh.GetGeometryByMeshElement
   Mesh.MeshDimension
   Mesh.GetMeshInfo
   Mesh.NbNodes
   Mesh.NbElements
   Mesh.Nb0DElements
   Mesh.NbBalls
   Mesh.NbEdges
   Mesh.NbEdgesOfOrder
   Mesh.NbFaces
   Mesh.NbFacesOfOrder
   Mesh.NbTriangles
   Mesh.NbTrianglesOfOrder
   Mesh.NbBiQuadTriangles
   Mesh.NbQuadrangles
   Mesh.NbQuadranglesOfOrder
   Mesh.NbBiQuadQuadrangles
   Mesh.NbPolygons
   Mesh.NbVolumes
   Mesh.NbVolumesOfOrder
   Mesh.NbTetras
   Mesh.NbTetrasOfOrder
   Mesh.NbHexas
   Mesh.NbHexasOfOrder
   Mesh.NbTriQuadraticHexas
   Mesh.NbPyramids
   Mesh.NbPyramidsOfOrder
   Mesh.NbPrisms
   Mesh.NbPrismsOfOrder
   Mesh.NbHexagonalPrisms
   Mesh.NbPolyhedrons
   Mesh.NbSubMesh
   Mesh.GetNodesId
   Mesh.GetElementsId
   Mesh.GetElementsByType
   Mesh.GetElementType
   Mesh.GetElementGeomType
   Mesh.GetElementShape
   Mesh.GetSubMeshElementsId
   Mesh.GetSubMeshNodesId
   Mesh.GetSubMeshElementType
   Mesh.GetNodeXYZ
   Mesh.GetNodeInverseElements
   Mesh.GetNodePosition
   Mesh.GetElementPosition
   Mesh.GetShapeID
   Mesh.GetShapeIDForElem
   Mesh.GetElemNbNodes
   Mesh.GetElemNode
   Mesh.GetElemNodes
   Mesh.IsMediumNode
   Mesh.IsMediumNodeOfAnyElem
   Mesh.ElemNbEdges
   Mesh.ElemNbFaces
   Mesh.GetElemFaceNodes
   Mesh.GetFaceNormal
   Mesh.FindElementByNodes
   Mesh.GetElementsByNodes
   Mesh.IsPoly
   Mesh.IsQuadratic
   Mesh.GetBallDiameter
   Mesh.BaryCenter
   Mesh.FindNodeClosestTo
   Mesh.FindElementsByPoint
   Mesh.GetPointState
   Mesh.Get1DBranches
   Mesh.Dump

******************************
Quality controls and Filtering
******************************

.. autosummary::

   smeshBuilder.GetEmptyCriterion
   smeshBuilder.GetCriterion
   smeshBuilder.GetFilter
   smeshBuilder.GetFilterFromCriteria
   smeshBuilder.GetFunctor
   Mesh.GetIdsFromFilter
   Mesh.GetMaxElementLength
   Mesh.GetAspectRatio
   Mesh.GetWarping
   Mesh.GetMinimumAngle
   Mesh.GetTaper
   Mesh.GetSkew
   Mesh.GetMinMax
   Mesh.IsManifold
   Mesh.IsCoherentOrientation2D

************
Measurements
************

.. autosummary::

   smeshBuilder.MinDistance
   smeshBuilder.GetMinDistance
   smeshBuilder.BoundingBox
   smeshBuilder.GetBoundingBox
   smeshBuilder.GetLength
   smeshBuilder.GetArea
   smeshBuilder.GetVolume
   smeshBuilder.GetAngle
   Mesh.GetFreeBorders
   Mesh.MinDistance
   Mesh.GetMinDistance
   Mesh.BoundingBox
   Mesh.GetBoundingBox
   Mesh.GetFunctor
   Mesh.FunctorValue
   Mesh.GetLength
   Mesh.GetArea
   Mesh.GetVolume
   Mesh.GetAngle

****************
Modifying meshes
****************

.. autosummary::

   smeshBuilder.GetPattern
   Mesh.GetMeshEditor

Adding nodes and elements
=========================

.. autosummary::

   Mesh.AddNode
   Mesh.Add0DElement
   Mesh.Add0DElementsToAllNodes
   Mesh.AddBall
   Mesh.AddEdge
   Mesh.AddFace
   Mesh.AddPolygonalFace
   Mesh.AddQuadPolygonalFace
   Mesh.AddVolume
   Mesh.AddPolyhedralVolume
   Mesh.AddPolyhedralVolumeByFaces
   Mesh.SetNodeOnVertex
   Mesh.SetNodeOnEdge
   Mesh.SetNodeOnFace
   Mesh.SetNodeInVolume
   Mesh.SetMeshElementOnShape
   Mesh.Make2DMeshFrom3D
   Mesh.MakeBoundaryMesh
   Mesh.MakeBoundaryElements
   Mesh.Append
   Mesh.GetLastCreatedNodes
   Mesh.GetLastCreatedElems
   Mesh.ClearLastCreated

Removing nodes and elements
===========================

.. autosummary::

   Mesh.RemoveElements
   Mesh.RemoveNodes
   Mesh.RemoveNodeWithReconnection
   Mesh.RemoveOrphanNodes

Modifying nodes and elements
============================

.. autosummary::

   Mesh.MoveNode
   Mesh.MoveClosestNodeToPoint
   Mesh.MeshToPassThroughAPoint
   Mesh.ChangeElemNodes

Convert to/from Quadratic Mesh
==============================

.. autosummary::

   Mesh.ConvertToQuadratic
   Mesh.ConvertFromQuadratic

Extrusion and Revolution
========================

.. autosummary::

   Mesh.RotationSweepObjects
   Mesh.RotationSweep
   Mesh.RotationSweepObject
   Mesh.RotationSweepObject1D
   Mesh.RotationSweepObject2D
   Mesh.ExtrusionSweepObjects
   Mesh.ExtrusionSweep
   Mesh.ExtrusionByNormal
   Mesh.ExtrusionSweepObject
   Mesh.ExtrusionSweepObject1D
   Mesh.ExtrusionSweepObject2D
   Mesh.AdvancedExtrusion
   Mesh.ExtrusionAlongPathObjects
   Mesh.ExtrusionAlongPathX
   Mesh.ExtrusionAlongPath
   Mesh.ExtrusionAlongPathObject
   Mesh.ExtrusionAlongPathObject1D
   Mesh.ExtrusionAlongPathObject2D

Transforming meshes (Translation, Rotation, Symmetry, Sewing, Merging)
======================================================================

.. autosummary::

   Mesh.Mirror
   Mesh.MirrorMakeMesh
   Mesh.MirrorObject
   Mesh.MirrorObjectMakeMesh
   Mesh.Translate
   Mesh.TranslateMakeMesh
   Mesh.TranslateObject
   Mesh.TranslateObjectMakeMesh
   Mesh.Rotate
   Mesh.RotateMakeMesh
   Mesh.RotateObject
   Mesh.RotateObjectMakeMesh
   Mesh.FindCoincidentNodes
   Mesh.FindCoincidentNodesOnPart
   Mesh.MergeNodes
   Mesh.FindEqualElements
   Mesh.MergeElements
   Mesh.MergeEqualElements
   Mesh.FindFreeBorders
   Mesh.FillHole
   Mesh.FindCoincidentFreeBorders
   Mesh.SewCoincidentFreeBorders
   Mesh.SewFreeBorders
   Mesh.SewConformFreeBorders
   Mesh.SewBorderToSide
   Mesh.SewSideElements

Changing orientation of elements
================================

.. autosummary::

   Mesh.Reorient
   Mesh.ReorientObject
   Mesh.Reorient2D
   Mesh.Reorient2DByNeighbours
   Mesh.Reorient2DBy3D

Uniting triangles
=================

.. autosummary::

   Mesh.DeleteDiag
   Mesh.TriToQuad
   Mesh.TriToQuadObject

Cutting elements
================

.. autosummary::

   Mesh.InverseDiag
   Mesh.QuadToTri
   Mesh.QuadToTriObject
   Mesh.QuadTo4Tri
   Mesh.SplitQuad
   Mesh.SplitQuadObject
   Mesh.BestSplit
   Mesh.SplitVolumesIntoTetra
   Mesh.SplitBiQuadraticIntoLinear
   Mesh.SplitHexahedraIntoPrisms
   Mesh.SplitQuadsNearTriangularFacets
   Mesh.SplitHexaToTetras
   Mesh.SplitHexaToPrisms
   Mesh.AddNodeOnSegment
   Mesh.AddNodeOnFace

Smoothing
=========

.. autosummary::

   Mesh.Smooth
   Mesh.SmoothObject
   Mesh.SmoothParametric
   Mesh.SmoothParametricObject

Duplication of nodes and elements (to emulate cracks)
=====================================================

.. autosummary::

   Mesh.DoubleElements
   Mesh.DoubleNodes
   Mesh.DoubleNode
   Mesh.DoubleNodeGroup
   Mesh.DoubleNodeGroups
   Mesh.DoubleNodeElem
   Mesh.DoubleNodeElemInRegion
   Mesh.DoubleNodeElemGroup
   Mesh.DoubleNodeElemGroupInRegion
   Mesh.DoubleNodeElemGroups
   Mesh.DoubleNodeElemGroupsInRegion
   Mesh.AffectedElemGroupsInRegion
   Mesh.DoubleNodesOnGroupBoundaries
   Mesh.CreateFlatElementsOnFacesGroups
   
Renumbering nodes and elements
==============================

.. autosummary::

   Mesh.RenumberNodes
   Mesh.RenumberElements


*****************
Auxiliary methods
*****************

.. autosummary::

   GetName
   DegreesToRadians
   New
