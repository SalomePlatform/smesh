.. _modules_page:

#######
Modules
#######

********************************
Auxiliary methods and structures
********************************

Functions:

:meth:`def ParseParameters <smeshBuilder.ParseParameters>`

:meth:`def ParseAngles <smeshBuilder.ParseAngles>`

:meth:`def __initPointStruct <smeshBuilder.__initPointStruct>`

:meth:`def __initAxisStruct <smeshBuilder.__initAxisStruct>`

:meth:`def IsEqual <smeshBuilder.IsEqual>`

:meth:`def GetName <smeshBuilder.GetName>`

:meth:`def TreatHypoStatus <smeshBuilder.TreatHypoStatus>`

:meth:`def AssureGeomPublished <smeshBuilder.AssureGeomPublished>`

:meth:`def FirstVertexOnCurve <smeshBuilder.FirstVertexOnCurve>`

:meth:`def DumpPython <smeshBuilder.smeshBuilder.DumpPython>`

:meth:`def SetDumpPythonHistorical <smeshBuilder.smeshBuilder.SetDumpPythonHistorical>`

:meth:`def init_smesh <smeshBuilder.smeshBuilder.init_smesh>`

:meth:`def EnumToLong <smeshBuilder.smeshBuilder.EnumToLong>`

:meth:`def ColorToString <smeshBuilder.smeshBuilder.ColorToString>`

:meth:`def GetPointStruct <smeshBuilder.smeshBuilder.GetPointStruct>`

:meth:`def GetDirStruct <smeshBuilder.smeshBuilder.GetDirStruct>`

:meth:`def MakeDirStruct <smeshBuilder.smeshBuilder.MakeDirStruct>`

:meth:`def GetAxisStruct <smeshBuilder.smeshBuilder.GetAxisStruct>`

:meth:`def SetName <smeshBuilder.smeshBuilder.SetName>`

:meth:`def SetEmbeddedMode <smeshBuilder.smeshBuilder.SetEmbeddedMode>`

:meth:`def IsEmbeddedMode <smeshBuilder.smeshBuilder.IsEmbeddedMode>`

:meth:`def SetCurrentStudy <smeshBuilder.smeshBuilder.SetCurrentStudy>`

:meth:`def GetCurrentStudy <smeshBuilder.smeshBuilder.GetCurrentStudy>`

:meth:`def GetSubShapesId <smeshBuilder.smeshBuilder.GetSubShapesId>`

:meth:`def SetBoundaryBoxSegmentation <smeshBuilder.smeshBuilder.SetBoundaryBoxSegmentation>`

:meth:`def GetSubShapeName <smeshBuilder.Mesh.GetSubShapeName>`

:meth:`def GetLog <smeshBuilder.Mesh.GetLog>`

:meth:`def ClearLog <smeshBuilder.Mesh.ClearLog>`

:meth:`def GetId <smeshBuilder.Mesh.GetId>`

:meth:`def GetStudyId <smeshBuilder.Mesh.GetStudyId>`

:meth:`def GetIDSource <smeshBuilder.Mesh.GetIDSource>`



***************
Creating Meshes
***************

Functions:

:meth:`def Concatenate <smeshBuilder.smeshBuilder.Concatenate>`

:meth:`def CopyMesh <smeshBuilder.smeshBuilder.CopyMesh>`

Importing and exporting meshes
##############################

Functions:

:meth:`def CreateMeshesFromUNV <smeshBuilder.smeshBuilder.CreateMeshesFromUNV>`

:meth:`def CreateMeshesFromMED <smeshBuilder.smeshBuilder.CreateMeshesFromMED>`

:meth:`def CreateMeshesFromSAUV <smeshBuilder.smeshBuilder.CreateMeshesFromSAUV>`

:meth:`def CreateMeshesFromSTL <smeshBuilder.smeshBuilder.CreateMeshesFromSTL>`

:meth:`def CreateMeshesFromCGNS <smeshBuilder.smeshBuilder.CreateMeshesFromCGNS>`

:meth:`def CreateMeshesFromGMF <smeshBuilder.smeshBuilder.CreateMeshesFromGMF>`

:meth:`def ExportMED <smeshBuilder.Mesh.ExportMED>`

:meth:`def ExportSAUV <smeshBuilder.Mesh.ExportSAUV>`

:meth:`def ExportDAT <smeshBuilder.Mesh.ExportDAT>`

:meth:`def ExportUNV <smeshBuilder.Mesh.ExportUNV>`

:meth:`def ExportSTL <smeshBuilder.Mesh.ExportSTL>`

:meth:`def ExportCGNS <smeshBuilder.Mesh.ExportCGNS>`

:meth:`def ExportGMF <smeshBuilder.Mesh.ExportGMF>`

:meth:`def ExportToMED <smeshBuilder.Mesh.ExportToMED>`


Constructing meshes
###################

Functions:

:meth:`def Mesh <smeshBuilder.smeshBuilder.Mesh>`

:meth:`def __init__ <smeshBuilder.Mesh.__init__>`

:meth:`def SetMesh <smeshBuilder.Mesh.SetMesh>`

:meth:`def GetMesh <smeshBuilder.Mesh.GetMesh>`

:meth:`def SetName <smeshBuilder.Mesh.SetName>`

:meth:`def GetShape <smeshBuilder.Mesh.GetShape>`

:meth:`def SetShape <smeshBuilder.Mesh.SetShape>`

:meth:`def IsReadyToCompute <smeshBuilder.Mesh.IsReadyToCompute>`

:meth:`def GetAlgoState <smeshBuilder.Mesh.GetAlgoState>`

:meth:`def Evaluate <smeshBuilder.Mesh.Evaluate>`

:meth:`def Compute <smeshBuilder.Mesh.Compute>`

:meth:`def GetComputeErrors <smeshBuilder.Mesh.GetComputeErrors>`

:meth:`def GetFailedShapes <smeshBuilder.Mesh.GetFailedShapes>`

:meth:`def GetMeshOrder <smeshBuilder.Mesh.GetMeshOrder>`

:meth:`def SetMeshOrder <smeshBuilder.Mesh.SetMeshOrder>`

:meth:`def Clear <smeshBuilder.Mesh.Clear>`


Defining Algorithms
###################

Basic meshing algorithms
========================

Data Structures:

:class:`class StdMeshersBuilder_Segment <StdMeshersBuilder.StdMeshersBuilder_Segment>`

:class:`class StdMeshersBuilder_Segment_Python <StdMeshersBuilder.StdMeshersBuilder_Segment_Python>`

:class:`class StdMeshersBuilder_Triangle_MEFISTO <StdMeshersBuilder.StdMeshersBuilder_Triangle_MEFISTO>`

:class:`class StdMeshersBuilder_Quadrangle <StdMeshersBuilder.StdMeshersBuilder_Quadrangle>`

:class:`class StdMeshersBuilder_Hexahedron <StdMeshersBuilder.StdMeshersBuilder_Hexahedron>`

:class:`class StdMeshersBuilder_UseExistingElements_1D <StdMeshersBuilder.StdMeshersBuilder_UseExistingElements_1D>`

:class:`class StdMeshersBuilder_UseExistingElements_1D2D <StdMeshersBuilder.StdMeshersBuilder_UseExistingElements_1D2D>`

:class:`class StdMeshersBuilder_Cartesian_3D <StdMeshersBuilder.StdMeshersBuilder_Cartesian_3D>`

:class:`class StdMeshersBuilder_UseExisting_1D <StdMeshersBuilder.StdMeshersBuilder_UseExisting_1D>`

:class:`class StdMeshersBuilder_UseExisting_2D <StdMeshersBuilder.StdMeshersBuilder_UseExisting_2D>`

Functions:

:meth:`def AutomaticTetrahedralization <smeshBuilder.Mesh.AutomaticTetrahedralization>`

:meth:`def AutomaticHexahedralization <smeshBuilder.Mesh.AutomaticHexahedralization>`




Projection algorithms
=====================

Data Structures:

:class:`class StdMeshersBuilder_Projection1D <StdMeshersBuilder.StdMeshersBuilder_Projection1D>`

:class:`class StdMeshersBuilder_Projection2D <StdMeshersBuilder.StdMeshersBuilder_Projection2D>`

:class:`class StdMeshersBuilder_Projection1D2D <StdMeshersBuilder.StdMeshersBuilder_Projection1D2D>`

:class:`class StdMeshersBuilder_Projection3D <StdMeshersBuilder.StdMeshersBuilder_Projection3D>`


Segments around vertex
======================

Functions:

:meth:`def LengthNearVertex <StdMeshersBuilder.StdMeshersBuilder_Segment.LengthNearVertex>`

3D extrusion meshing algorithms
===============================

Data Structures:

:class:`class StdMeshersBuilder_Prism3D <StdMeshersBuilder.StdMeshersBuilder_Prism3D>`

:class:`class StdMeshersBuilder_RadialPrism3D <StdMeshersBuilder.StdMeshersBuilder_RadialPrism3D>`


Mesh_Algorithm
==============

:class:`class Mesh_Algorithm <smesh_algorithm.Mesh_Algorithm>`


Defining hypotheses
###################

1D Meshing Hypotheses
=====================

Functions:

:meth:`def ReversedEdgeIndices <smesh_algorithm.Mesh_Algorithm.ReversedEdgeIndices>`

:meth:`def LocalLength <StdMeshersBuilder.StdMeshersBuilder_Segment.LocalLength>`

:meth:`def MaxSize <StdMeshersBuilder.StdMeshersBuilder_Segment.MaxSize>`

:meth:`def NumberOfSegments <StdMeshersBuilder.StdMeshersBuilder_Segment.NumberOfSegments>`

:meth:`def Adaptive <StdMeshersBuilder.StdMeshersBuilder_Segment.Adaptive>`

:meth:`def Arithmetic1D <StdMeshersBuilder.StdMeshersBuilder_Segment.Arithmetic1D>`

:meth:`def GeometricProgression <StdMeshersBuilder.StdMeshersBuilder_Segment.GeometricProgression>`

:meth:`def FixedPoints1D <StdMeshersBuilder.StdMeshersBuilder_Segment.FixedPoints1D>`

:meth:`def StartEndLength <StdMeshersBuilder.StdMeshersBuilder_Segment.StartEndLength>`

:meth:`def Deflection1D <StdMeshersBuilder.StdMeshersBuilder_Segment.Deflection1D>`

:meth:`def AutomaticLength <StdMeshersBuilder.StdMeshersBuilder_Segment.AutomaticLength>`

:meth:`def PythonSplit1D <StdMeshersBuilder.StdMeshersBuilder_Segment_Python.PythonSplit1D>`


2D Meshing Hypotheses
=====================

Functions:

:meth:`def MaxElementArea <StdMeshersBuilder.StdMeshersBuilder_Triangle_MEFISTO.MaxElementArea>`

:meth:`def LengthFromEdges <StdMeshersBuilder.StdMeshersBuilder_Triangle_MEFISTO.LengthFromEdges>`


Quadrangle Hypotheses
=====================

Functions:

:meth:`def QuadrangleParameters <StdMeshersBuilder.StdMeshersBuilder_Quadrangle.QuadrangleParameters>`

:meth:`def QuadranglePreference <StdMeshersBuilder.StdMeshersBuilder_Quadrangle.QuadranglePreference>`

:meth:`def TrianglePreference <StdMeshersBuilder.StdMeshersBuilder_Quadrangle.TrianglePreference>`

:meth:`def Reduced <StdMeshersBuilder.StdMeshersBuilder_Quadrangle.Reduced>`

:meth:`def TriangleVertex <StdMeshersBuilder.StdMeshersBuilder_Quadrangle.TriangleVertex>`



Additional Hypotheses
=====================

Functions:

:meth:`def ViscousLayers <smesh_algorithm.Mesh_Algorithm.ViscousLayers>`

:meth:`def ViscousLayers2D <smesh_algorithm.Mesh_Algorithm.ViscousLayers2D>`

:meth:`def Propagation <StdMeshersBuilder.StdMeshersBuilder_Segment.Propagation>`

:meth:`def PropagationOfDistribution <StdMeshersBuilder.StdMeshersBuilder_Segment.PropagationOfDistribution>`

:meth:`def QuadraticMesh <StdMeshersBuilder.StdMeshersBuilder_Segment.QuadraticMesh>`


Constructing sub-meshes
#######################

Functions:

:meth:`def GetSubMesh <smeshBuilder.Mesh.GetSubMesh>`

:meth:`def ClearSubMesh <smeshBuilder.Mesh.ClearSubMesh>`

:meth:`def Compute <smeshBuilder.Mesh.Compute>`



Editing meshes
##############

Functions:

:meth:`def AddHypothesis <smeshBuilder.Mesh.AddHypothesis>`

:meth:`def IsUsedHypothesis <smeshBuilder.Mesh.IsUsedHypothesis>`

:meth:`def RemoveHypothesis <smeshBuilder.Mesh.RemoveHypothesis>`

:meth:`def GetHypothesisList <smeshBuilder.Mesh.GetHypothesisList>`

:meth:`def RemoveGlobalHypotheses <smeshBuilder.Mesh.RemoveGlobalHypotheses>`


****************
Mesh Information
****************

Functions:

:meth:`def GetMeshInfo <smeshBuilder.smeshBuilder.GetMeshInfo>`

:meth:`def GetGeometryByMeshElement <smeshBuilder.Mesh.GetGeometryByMeshElement>`

:meth:`def MeshDimension <smeshBuilder.Mesh.MeshDimension>`

:meth:`def GetMeshInfo <smeshBuilder.Mesh.GetMeshInfo>`

:meth:`def NbNodes <smeshBuilder.Mesh.NbNodes>`

:meth:`def NbElements <smeshBuilder.Mesh.NbElements>`

:meth:`def Nb0DElements <smeshBuilder.Mesh.Nb0DElements>`

:meth:`def NbBalls <smeshBuilder.Mesh.NbBalls>`

:meth:`def NbEdges <smeshBuilder.Mesh.NbEdges>`

:meth:`def NbEdgesOfOrder <smeshBuilder.Mesh.NbEdgesOfOrder>`

:meth:`def NbFaces <smeshBuilder.Mesh.NbFaces>`

:meth:`def NbFacesOfOrder <smeshBuilder.Mesh.NbFacesOfOrder>`

:meth:`def NbTriangles <smeshBuilder.Mesh.NbTriangles>`

:meth:`def NbTrianglesOfOrder <smeshBuilder.Mesh.NbTrianglesOfOrder>`

:meth:`def NbBiQuadTriangles <smeshBuilder.Mesh.NbBiQuadTriangles>`

:meth:`def NbQuadrangles <smeshBuilder.Mesh.NbQuadrangles>`

:meth:`def NbQuadranglesOfOrder <smeshBuilder.Mesh.NbQuadranglesOfOrder>`

:meth:`def NbBiQuadQuadrangles <smeshBuilder.Mesh.NbBiQuadQuadrangles>`

:meth:`def NbPolygons <smeshBuilder.Mesh.NbPolygons>`

:meth:`def NbVolumes <smeshBuilder.Mesh.NbVolumes>`

:meth:`def NbVolumesOfOrder <smeshBuilder.Mesh.NbVolumesOfOrder>`

:meth:`def NbTetras <smeshBuilder.Mesh.NbTetras>`

:meth:`def NbTetrasOfOrder <smeshBuilder.Mesh.NbTetrasOfOrder>`

:meth:`def NbHexas <smeshBuilder.Mesh.NbHexas>`

:meth:`def NbHexasOfOrder <smeshBuilder.Mesh.NbHexasOfOrder>`

:meth:`def NbTriQuadraticHexas <smeshBuilder.Mesh.NbTriQuadraticHexas>`

:meth:`def NbPyramids <smeshBuilder.Mesh.NbPyramids>`

:meth:`def NbPyramidsOfOrder <smeshBuilder.Mesh.NbPyramidsOfOrder>`

:meth:`def NbPrisms <smeshBuilder.Mesh.NbPrisms>`

:meth:`def NbPrismsOfOrder <smeshBuilder.Mesh.NbPrismsOfOrder>`

:meth:`def NbHexagonalPrisms <smeshBuilder.Mesh.NbHexagonalPrisms>`

:meth:`def NbPolyhedrons <smeshBuilder.Mesh.NbPolyhedrons>`

:meth:`def NbSubMesh <smeshBuilder.Mesh.NbSubMesh>`

:meth:`def GetElementsId <smeshBuilder.Mesh.GetElementsId>`

:meth:`def GetElementsByType <smeshBuilder.Mesh.GetElementsByType>`

:meth:`def GetNodesId <smeshBuilder.Mesh.GetNodesId>`

:meth:`def GetElementType <smeshBuilder.Mesh.GetElementType>`

:meth:`def GetElementGeomType <smeshBuilder.Mesh.GetElementGeomType>`

:meth:`def GetElementShape <smeshBuilder.Mesh.GetElementShape>`

:meth:`def GetSubMeshElementsId <smeshBuilder.Mesh.GetSubMeshElementsId>`

:meth:`def GetSubMeshNodesId <smeshBuilder.Mesh.GetSubMeshNodesId>`

:meth:`def GetSubMeshElementType <smeshBuilder.Mesh.GetSubMeshElementType>`

:meth:`def Dump <smeshBuilder.Mesh.Dump>`

:meth:`def GetNodeXYZ <smeshBuilder.Mesh.GetNodeXYZ>`

:meth:`def GetNodeInverseElements <smeshBuilder.Mesh.GetNodeInverseElements>`

:meth:`def GetNodePosition <smeshBuilder.Mesh.GetNodePosition>`

:meth:`def GetElementPosition <smeshBuilder.Mesh.GetElementPosition>`

:meth:`def GetShapeID <smeshBuilder.Mesh.GetShapeID>`

:meth:`def GetShapeIDForElem <smeshBuilder.Mesh.GetShapeIDForElem>`

:meth:`def GetElemNbNodes <smeshBuilder.Mesh.GetElemNbNodes>`

:meth:`def GetElemNode <smeshBuilder.Mesh.GetElemNode>`

:meth:`def GetElemNodes <smeshBuilder.Mesh.GetElemNodes>`

:meth:`def IsMediumNode <smeshBuilder.Mesh.IsMediumNode>`

:meth:`def IsMediumNodeOfAnyElem <smeshBuilder.Mesh.IsMediumNodeOfAnyElem>`

:meth:`def ElemNbEdges <smeshBuilder.Mesh.ElemNbEdges>`

:meth:`def ElemNbFaces <smeshBuilder.Mesh.ElemNbFaces>`

:meth:`def GetElemFaceNodes <smeshBuilder.Mesh.GetElemFaceNodes>`

:meth:`def GetFaceNormal <smeshBuilder.Mesh.GetFaceNormal>`

:meth:`def FindElementByNodes <smeshBuilder.Mesh.FindElementByNodes>`

:meth:`def GetElementsByNodes <smeshBuilder.Mesh.GetElementsByNodes>`

:meth:`def IsPoly <smeshBuilder.Mesh.IsPoly>`

:meth:`def IsQuadratic <smeshBuilder.Mesh.IsQuadratic>`

:meth:`def GetBallDiameter <smeshBuilder.Mesh.GetBallDiameter>`

:meth:`def BaryCenter <smeshBuilder.Mesh.BaryCenter>`

:meth:`def FindNodeClosestTo <smeshBuilder.Mesh.FindNodeClosestTo>`

:meth:`def FindElementsByPoint <smeshBuilder.Mesh.FindElementsByPoint>`

:meth:`def GetPointState <smeshBuilder.Mesh.GetPointState>`


******************************
Quality controls and Filtering
******************************

Functions:

:meth:`def GetEmptyCriterion <smeshBuilder.smeshBuilder.GetEmptyCriterion>`

:meth:`def GetCriterion <smeshBuilder.smeshBuilder.GetCriterion>`

:meth:`def GetFilter <smeshBuilder.smeshBuilder.GetFilter>`

:meth:`def GetFilterFromCriteria <smeshBuilder.smeshBuilder.GetFilterFromCriteria>`

:meth:`def GetFunctor <smeshBuilder.smeshBuilder.GetFunctor>`

:meth:`def GetIdsFromFilter <smeshBuilder.Mesh.GetIdsFromFilter>`

:meth:`def IsManifold <smeshBuilder.Mesh.IsManifold>`

:meth:`def IsCoherentOrientation2D <smeshBuilder.Mesh.IsCoherentOrientation2D>`



*****************
Grouping elements
*****************

Functions:

:meth:`def SetAutoColor <smeshBuilder.Mesh.SetAutoColor>`

:meth:`def GetAutoColor <smeshBuilder.Mesh.GetAutoColor>`

:meth:`def HasDuplicatedGroupNamesMED <smeshBuilder.Mesh.HasDuplicatedGroupNamesMED>`


Creating groups
###############

Functions:

:meth:`def CreateEmptyGroup <smeshBuilder.Mesh.CreateEmptyGroup>`

:meth:`def Group <smeshBuilder.Mesh.Group>`

:meth:`def GroupOnGeom <smeshBuilder.Mesh.GroupOnGeom>`

:meth:`def GroupOnFilter <smeshBuilder.Mesh.GroupOnFilter>`

:meth:`def MakeGroupByIds <smeshBuilder.Mesh.MakeGroupByIds>`

:meth:`def MakeGroup <smeshBuilder.Mesh.MakeGroup>`

:meth:`def MakeGroupByCriterion <smeshBuilder.Mesh.MakeGroupByCriterion>`

:meth:`def MakeGroupByCriteria <smeshBuilder.Mesh.MakeGroupByCriteria>`

:meth:`def MakeGroupByFilter <smeshBuilder.Mesh.MakeGroupByFilter>`

:meth:`def GetGroups <smeshBuilder.Mesh.GetGroups>`

:meth:`def NbGroups <smeshBuilder.Mesh.NbGroups>`

:meth:`def GetGroupNames <smeshBuilder.Mesh.GetGroupNames>`

:meth:`def GetGroupByName <smeshBuilder.Mesh.GetGroupByName>`


Using operations on groups
##########################

Functions:

:meth:`def UnionGroups <smeshBuilder.Mesh.UnionGroups>`

:meth:`def UnionListOfGroups <smeshBuilder.Mesh.UnionListOfGroups>`

:meth:`def IntersectGroups <smeshBuilder.Mesh.IntersectGroups>`

:meth:`def IntersectListOfGroups <smeshBuilder.Mesh.IntersectListOfGroups>`

:meth:`def CutGroups <smeshBuilder.Mesh.CutGroups>`

:meth:`def CutListOfGroups <smeshBuilder.Mesh.CutListOfGroups>`

:meth:`def CreateDimGroup <smeshBuilder.Mesh.CreateDimGroup>`

:meth:`def ConvertToStandalone <smeshBuilder.Mesh.ConvertToStandalone>`


Deleting groups
###############

Functions:

:meth:`def RemoveGroup <smeshBuilder.Mesh.RemoveGroup>`

:meth:`def RemoveGroupWithContents <smeshBuilder.Mesh.RemoveGroupWithContents>`


****************
Modifying meshes
****************

Functions:

:meth:`def GetPattern <smeshBuilder.smeshBuilder.GetPattern>`

:meth:`def GetMeshEditor <smeshBuilder.Mesh.GetMeshEditor>`


Adding nodes and elements
#########################

Functions:

:meth:`def AddNode <smeshBuilder.Mesh.AddNode>`

:meth:`def Add0DElement <smeshBuilder.Mesh.Add0DElement>`

:meth:`def Add0DElementsToAllNodes <smeshBuilder.Mesh.Add0DElementsToAllNodes>`

:meth:`def AddBall <smeshBuilder.Mesh.AddBall>`

:meth:`def AddEdge <smeshBuilder.Mesh.AddEdge>`

:meth:`def AddFace <smeshBuilder.Mesh.AddFace>`

:meth:`def AddPolygonalFace <smeshBuilder.Mesh.AddPolygonalFace>`

:meth:`def AddQuadPolygonalFace <smeshBuilder.Mesh.AddQuadPolygonalFace>`

:meth:`def AddVolume <smeshBuilder.Mesh.AddVolume>`

:meth:`def AddPolyhedralVolume <smeshBuilder.Mesh.AddPolyhedralVolume>`

:meth:`def AddPolyhedralVolumeByFaces <smeshBuilder.Mesh.AddPolyhedralVolumeByFaces>`

:meth:`def SetNodeOnVertex <smeshBuilder.Mesh.SetNodeOnVertex>`

:meth:`def SetNodeOnEdge <smeshBuilder.Mesh.SetNodeOnEdge>`

:meth:`def SetNodeOnFace <smeshBuilder.Mesh.SetNodeOnFace>`

:meth:`def SetNodeInVolume <smeshBuilder.Mesh.SetNodeInVolume>`

:meth:`def SetMeshElementOnShape <smeshBuilder.Mesh.SetMeshElementOnShape>`

:meth:`def Make2DMeshFrom3D <smeshBuilder.Mesh.Make2DMeshFrom3D>`

:meth:`def MakeBoundaryMesh <smeshBuilder.Mesh.MakeBoundaryMesh>`

:meth:`def MakeBoundaryElements <smeshBuilder.Mesh.MakeBoundaryElements>`

:meth:`def GetLastCreatedNodes <smeshBuilder.Mesh.GetLastCreatedNodes>`

:meth:`def GetLastCreatedElems <smeshBuilder.Mesh.GetLastCreatedElems>`

:meth:`def ClearLastCreated <smeshBuilder.Mesh.ClearLastCreated>`


Removing nodes and elements
###########################

Functions:

:meth:`def RemoveElements <smeshBuilder.Mesh.RemoveElements>`

:meth:`def RemoveNodes <smeshBuilder.Mesh.RemoveNodes>`

:meth:`def RemoveOrphanNodes <smeshBuilder.Mesh.RemoveOrphanNodes>`


Modifying nodes and elements
############################

functions:

:meth:`def MoveNode <smeshBuilder.Mesh.MoveNode>`

:meth:`def MoveClosestNodeToPoint <smeshBuilder.Mesh.MoveClosestNodeToPoint>`

:meth:`def MeshToPassThroughAPoint <smeshBuilder.Mesh.MeshToPassThroughAPoint>`

:meth:`def ChangeElemNodes <smeshBuilder.Mesh.ChangeElemNodes>`


Renumbering nodes and elements
##############################

Functions:

:meth:`def RenumberNodes <smeshBuilder.Mesh.RenumberNodes>`

:meth:`def RenumberElements <smeshBuilder.Mesh.RenumberElements>`


Transforming meshes (Translation, Rotation, Symmetry, Sewing, Merging)
######################################################################

Functions:

:meth:`def Mirror <smeshBuilder.Mesh.Mirror>`

:meth:`def MirrorMakeMesh <smeshBuilder.Mesh.MirrorMakeMesh>`

:meth:`def MirrorObject <smeshBuilder.Mesh.MirrorObject>`

:meth:`def MirrorObjectMakeMesh <smeshBuilder.Mesh.MirrorObjectMakeMesh>`

:meth:`def Translate <smeshBuilder.Mesh.Translate>`

:meth:`def TranslateMakeMesh <smeshBuilder.Mesh.TranslateMakeMesh>`

:meth:`def TranslateObject <smeshBuilder.Mesh.TranslateObject>`

:meth:`def TranslateObjectMakeMesh <smeshBuilder.Mesh.TranslateObjectMakeMesh>`

:meth:`def Rotate <smeshBuilder.Mesh.Rotate>`

:meth:`def RotateMakeMesh <smeshBuilder.Mesh.RotateMakeMesh>`

:meth:`def RotateObject <smeshBuilder.Mesh.RotateObject>`

:meth:`def RotateObjectMakeMesh <smeshBuilder.Mesh.RotateObjectMakeMesh>`

:meth:`def FindCoincidentNodes <smeshBuilder.Mesh.FindCoincidentNodes>`

:meth:`def FindCoincidentNodesOnPart <smeshBuilder.Mesh.FindCoincidentNodesOnPart>`

:meth:`def MergeNodes <smeshBuilder.Mesh.MergeNodes>`

:meth:`def FindEqualElements <smeshBuilder.Mesh.FindEqualElements>`

:meth:`def MergeElements <smeshBuilder.Mesh.MergeElements>`

:meth:`def MergeEqualElements <smeshBuilder.Mesh.MergeEqualElements>`

:meth:`def FindFreeBorders <smeshBuilder.Mesh.FindFreeBorders>`

:meth:`def FillHole <smeshBuilder.Mesh.FillHole>`

:meth:`def FindCoincidentFreeBorders <smeshBuilder.Mesh.FindCoincidentFreeBorders>`

:meth:`def SewCoincidentFreeBorders <smeshBuilder.Mesh.SewCoincidentFreeBorders>`

:meth:`def SewFreeBorders <smeshBuilder.Mesh.SewFreeBorders>`

:meth:`def SewConformFreeBorders <smeshBuilder.Mesh.SewConformFreeBorders>`

:meth:`def SewBorderToSide <smeshBuilder.Mesh.SewBorderToSide>`

:meth:`def SewSideElements <smeshBuilder.Mesh.SewSideElements>`



Uniting triangles
#################

Functions:

:meth:`def DeleteDiag <smeshBuilder.Mesh.DeleteDiag>`

:meth:`def TriToQuad <smeshBuilder.Mesh.TriToQuad>`

:meth:`def TriToQuadObject <smeshBuilder.Mesh.TriToQuadObject>`



Cutting elements
################

Functions:

:meth:`def InverseDiag <smeshBuilder.Mesh.InverseDiag>`

:meth:`def QuadToTri <smeshBuilder.Mesh.QuadToTri>`

:meth:`def QuadToTriObject <smeshBuilder.Mesh.QuadToTriObject>`

:meth:`def QuadTo4Tri <smeshBuilder.Mesh.QuadTo4Tri>`

:meth:`def SplitQuad <smeshBuilder.Mesh.SplitQuad>`

:meth:`def SplitQuadObject <smeshBuilder.Mesh.SplitQuadObject>`

:meth:`def BestSplit <smeshBuilder.Mesh.BestSplit>`

:meth:`def SplitVolumesIntoTetra <smeshBuilder.Mesh.SplitVolumesIntoTetra>`

:meth:`def SplitBiQuadraticIntoLinear <smeshBuilder.Mesh.SplitBiQuadraticIntoLinear>`

:meth:`def SplitHexahedraIntoPrisms <smeshBuilder.Mesh.SplitHexahedraIntoPrisms>`

:meth:`def SplitQuadsNearTriangularFacets <smeshBuilder.Mesh.SplitQuadsNearTriangularFacets>`

:meth:`def SplitHexaToTetras <smeshBuilder.Mesh.SplitHexaToTetras>`

:meth:`def SplitHexaToPrisms <smeshBuilder.Mesh.SplitHexaToPrisms>`



Changing orientation of elements
################################

Functions:

:meth:`def Reorient <smeshBuilder.Mesh.Reorient>`

:meth:`def ReorientObject <smeshBuilder.Mesh.ReorientObject>`

:meth:`def Reorient2D <smeshBuilder.Mesh.Reorient2D>`

:meth:`def Reorient2DBy3D <smeshBuilder.Mesh.Reorient2DBy3D>`



Smoothing
#########

Functions:

:meth:`def Smooth <smeshBuilder.Mesh.Smooth>`

:meth:`def SmoothObject <smeshBuilder.Mesh.SmoothObject>`

:meth:`def SmoothParametric <smeshBuilder.Mesh.SmoothParametric>`

:meth:`def SmoothParametricObject <smeshBuilder.Mesh.SmoothParametricObject>`



Extrusion and Revolution
########################

Functions:

:meth:`def RotationSweepObjects <smeshBuilder.Mesh.RotationSweepObjects>`

:meth:`def RotationSweep <smeshBuilder.Mesh.RotationSweep>`

:meth:`def RotationSweepObject <smeshBuilder.Mesh.RotationSweepObject>`

:meth:`def RotationSweepObject1D <smeshBuilder.Mesh.RotationSweepObject1D>`

:meth:`def RotationSweepObject2D <smeshBuilder.Mesh.RotationSweepObject2D>`

:meth:`def ExtrusionSweepObjects <smeshBuilder.Mesh.ExtrusionSweepObjects>`

:meth:`def ExtrusionSweep <smeshBuilder.Mesh.ExtrusionSweep>`

:meth:`def ExtrusionByNormal <smeshBuilder.Mesh.ExtrusionByNormal>`

:meth:`def ExtrusionSweepObject <smeshBuilder.Mesh.ExtrusionSweepObject>`

:meth:`def ExtrusionSweepObject1D <smeshBuilder.Mesh.ExtrusionSweepObject1D>`

:meth:`def ExtrusionSweepObject2D <smeshBuilder.Mesh.ExtrusionSweepObject2D>`

:meth:`def AdvancedExtrusion <smeshBuilder.Mesh.AdvancedExtrusion>`

:meth:`def ExtrusionAlongPathObjects <smeshBuilder.Mesh.ExtrusionAlongPathObjects>`

:meth:`def ExtrusionAlongPathX <smeshBuilder.Mesh.ExtrusionAlongPathX>`

:meth:`def ExtrusionAlongPath <smeshBuilder.Mesh.ExtrusionAlongPath>`

:meth:`def ExtrusionAlongPathObject <smeshBuilder.Mesh.ExtrusionAlongPathObject>`

:meth:`def ExtrusionAlongPathObject1D <smeshBuilder.Mesh.ExtrusionAlongPathObject1D>`

:meth:`def ExtrusionAlongPathObject2D <smeshBuilder.Mesh.ExtrusionAlongPathObject2D>`



Convert to/from Quadratic Mesh
##############################

Functions:

:meth:`def ConvertToQuadratic <smeshBuilder.Mesh.ConvertToQuadratic>`

:meth:`def ConvertFromQuadratic <smeshBuilder.Mesh.ConvertFromQuadratic>`



Duplication of nodes and elements (to emulate cracks)
#####################################################

Functions:

:meth:`def DoubleElements <smeshBuilder.Mesh.DoubleElements>`

:meth:`def DoubleNodes <smeshBuilder.Mesh.DoubleNodes>`

:meth:`def DoubleNode <smeshBuilder.Mesh.DoubleNode>`

:meth:`def DoubleNodeGroup <smeshBuilder.Mesh.DoubleNodeGroup>`

:meth:`def DoubleNodeGroups <smeshBuilder.Mesh.DoubleNodeGroups>`

:meth:`def DoubleNodeElem <smeshBuilder.Mesh.DoubleNodeElem>`

:meth:`def DoubleNodeElemInRegion <smeshBuilder.Mesh.DoubleNodeElemInRegion>`

:meth:`def DoubleNodeElemGroup <smeshBuilder.Mesh.DoubleNodeElemGroup>`

:meth:`def DoubleNodeElemGroupInRegion <smeshBuilder.Mesh.DoubleNodeElemGroupInRegion>`

:meth:`def DoubleNodeElemGroups <smeshBuilder.Mesh.DoubleNodeElemGroups>`

:meth:`def DoubleNodeElemGroupsInRegion <smeshBuilder.Mesh.DoubleNodeElemGroupsInRegion>`

:meth:`def AffectedElemGroupsInRegion <smeshBuilder.Mesh.AffectedElemGroupsInRegion>`

:meth:`def DoubleNodesOnGroupBoundaries <smeshBuilder.Mesh.DoubleNodesOnGroupBoundaries>`

:meth:`def CreateFlatElementsOnFacesGroups <smeshBuilder.Mesh.CreateFlatElementsOnFacesGroups>`



************
Measurements
************

Functions:


:meth:`def MinDistance <smeshBuilder.smeshBuilder.MinDistance>`

:meth:`def GetMinDistance <smeshBuilder.smeshBuilder.GetMinDistance>`

:meth:`def BoundingBox <smeshBuilder.smeshBuilder.BoundingBox>`

:meth:`def GetBoundingBox <smeshBuilder.smeshBuilder.GetBoundingBox>`

:meth:`def GetLength <smeshBuilder.smeshBuilder.GetLength>`

:meth:`def GetArea <smeshBuilder.smeshBuilder.GetArea>`

:meth:`def GetVolume <smeshBuilder.smeshBuilder.GetVolume>`

:meth:`def GetFreeBorders <smeshBuilder.Mesh.GetFreeBorders>`

:meth:`def MinDistance <smeshBuilder.Mesh.MinDistance>`

:meth:`def GetMinDistance <smeshBuilder.Mesh.GetMinDistance>`

:meth:`def BoundingBox <smeshBuilder.Mesh.BoundingBox>`

:meth:`def GetBoundingBox <smeshBuilder.Mesh.GetBoundingBox>`

:meth:`def GetFunctor <smeshBuilder.Mesh.GetFunctor>`

:meth:`def FunctorValue <smeshBuilder.Mesh.FunctorValue>`

:meth:`def GetLength <smeshBuilder.Mesh.GetLength>`

:meth:`def GetArea <smeshBuilder.Mesh.GetArea>`

:meth:`def GetVolume <smeshBuilder.Mesh.GetVolume>`

:meth:`def GetMaxElementLength <smeshBuilder.Mesh.GetMaxElementLength>`

:meth:`def GetAspectRatio <smeshBuilder.Mesh.GetAspectRatio>`

:meth:`def GetWarping <smeshBuilder.Mesh.GetWarping>`

:meth:`def GetMinimumAngle <smeshBuilder.Mesh.GetMinimumAngle>`

:meth:`def GetTaper <smeshBuilder.Mesh.GetTaper>`

:meth:`def GetSkew <smeshBuilder.Mesh.GetSkew>`

:meth:`def GetMinMax <smeshBuilder.Mesh.GetMinMax>`


*******************************
Accessing SMESH object in study
*******************************

Data Structures:

:class:`SMeshStudyTools <smeshstudytools.SMeshStudyTools>`




