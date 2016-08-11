// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_Operations.h
//  Author : IVAN MECHETIN, Open CASCADE S.A.S. (ivan.mechetin@opencascade.com)

#ifndef SMESHGUI_OPERATIONS_H
#define SMESHGUI_OPERATIONS_H

namespace SMESHOp {
  enum {
    // Tools --------------------------//--------------------------------
    OpDelete                 = 1000,   // DELETE
    OpSelectFiltersLibrary   = 1010,   // MENU TOOLS - SELECTION FILTERS LIBRARY
    OpReset                  = 1020,   // RESET
    OpScalarBarProperties    = 1021,   // SCALAR BAR PROPERTIES
    OpShowScalarBar          = 1022,   // SHOW SCALAR BAR
    OpSaveDistribution       = 1030,   // SAVE DISTRIBUTION
    OpShowDistribution       = 1031,   // SHOW DISTRIBUTION
#ifndef DISABLE_PLOT2DVIEWER
    OpPlotDistribution       = 1032,   // PLOT DISTRIBUTION
#endif
    OpFileInformation        = 1040,   // POPUP MENU - FILE INFORMATION
    // Import -------------------------//--------------------------------
    OpImportDAT              = 1100,   // MENU FILE - IMPORT - DAT FILE
    OpImportUNV              = 1101,   // MENU FILE - IMPORT - UNV FILE
    OpImportMED              = 1102,   // MENU FILE - IMPORT - MED FILE
    OpImportSTL              = 1103,   // MENU FILE - IMPORT - STL FILE
#ifdef WITH_CGNS
    OpImportCGNS             = 1104,   // MENU FILE - IMPORT - CGNS FILE
#endif
    OpImportSAUV             = 1105,   // MENU FILE - IMPORT - SAUV FILE
    OpImportGMF              = 1106,   // MENU FILE - IMPORT - GMF FILE
    // Export -------------------------//--------------------------------
    OpExportDAT              = 1200,   // MENU FILE  - EXPORT - DAT FILE
    OpExportMED              = 1201,   // MENU FILE  - EXPORT - MED FILE
    OpExportUNV              = 1202,   // MENU FILE  - EXPORT - UNV FILE
    OpExportSTL              = 1203,   // MENU FILE  - EXPORT - STL FILE
#ifdef WITH_CGNS
    OpExportCGNS             = 1204,   // MENU FILE  - EXPORT - CGNS FILE
#endif
    OpExportSAUV             = 1205,   // MENU FILE  - EXPORT - SAUV FILE
    OpExportGMF              = 1206,   // MENU FILE  - EXPORT - GMF FILE
    OpPopupExportDAT         = 1210,   // POPUP MENU - EXPORT - DAT FILE
    OpPopupExportMED         = 1211,   // POPUP MENU - EXPORT - MED FILE
    OpPopupExportUNV         = 1212,   // POPUP MENU - EXPORT - UNV FILE
    OpPopupExportSTL         = 1213,   // POPUP MENU - EXPORT - STL FILE
#ifdef WITH_CGNS
    OpPopupExportCGNS        = 1214,   // POPUP MENU - EXPORT - CGNS FILE
#endif
    OpPopupExportSAUV        = 1215,   // POPUP MENU - EXPORT - SAUV FILE
    OpPopupExportGMF         = 1216,   // POPUP MENU - EXPORT - GMF FILE
    // Mesh ---------------------------//--------------------------------
    OpCreateMesh             = 2030,   // MENU MESH  - CREATE MESH
    OpCreateSubMesh          = 2031,   // MENU MESH  - CREATE SUBMESH
    OpEditMeshOrSubMesh      = 2032,   // MENU MESH  - EDIT MESH/SUBMESH
    OpEditMesh               = 2033,   // POPUP  - EDIT MESH
    OpEditSubMesh            = 2034,   // POPUP  - EDIT SUBMESH
    OpBuildCompoundMesh      = 2035,   // MENU MESH  - BUILD COMPOUND
    OpCopyMesh               = 2036,   // MENU MESH  - COPY MESH
    OpCompute                = 2040,   // MENU MESH  - COMPUTE
    OpComputeSubMesh         = 2041,   // POPUP  - COMPUTE SUBMESH
    OpPreCompute             = 2042,   // MENU MESH  - PREVIEW
    OpEvaluate               = 2043,   // MENU MESH  - EVALUATE
    OpMeshOrder              = 2044,   // MENU MESH  - CHANGE SUBMESH PRIORITY
    OpCreateGroup            = 2050,   // MENU MESH  - CREATE GROUP
    OpCreateGeometryGroup    = 2051,   // MENU MESH  - CREATE GROUPS FROM GEOMETRY
    OpConstructGroup         = 2052,   // MENU MESH  - CONSTRUCT GROUP
    OpEditGroup              = 2053,   // MENU MESH  - EDIT GROUP
    OpEditGeomGroupAsGroup   = 2054,   // MENU MESH  - EDIT GROUP AS STANDALONE
    OpUnionGroups            = 2060,   // MENU MESH  - UNION GROUPS
    OpIntersectGroups        = 2061,   // MENU MESH  - INTERSECT GROUPS
    OpCutGroups              = 2062,   // MENU MESH  - CUT GROUPS
    OpGroupUnderlyingElem    = 2070,   // MENU MESH  - GROUP OF UNDERLYING ENTITIES
    OpEditGroupPopup         = 2080,   // POPUP MENU - EDIT GROUP
    OpAddElemGroupPopup      = 2081,   // POPUP MENU - ADD ELEMENTS TO GROUP
    OpRemoveElemGroupPopup   = 2082,   // POPUP MENU - REMOVE ELEMENTS FROM GROUP
    OpMeshInformation        = 2100,   // MENU MESH  - MESH INFORMATION
    OpWhatIs                 = 2101,   // MENU MESH  - MESH ELEMENT INFORMATION
    OpStdInfo                = 2102,   // MENU MESH  - MESH STANDART INFORMATION
    OpFindElementByPoint     = 2103,   // MENU MESH  - FIND ELEMENT BY POINT
    OpUpdate                 = 2200,   // POPUP MENU - UPDATE
    // Controls -----------------------//--------------------------------
    OpFreeNode               = 3000,   // MENU CONTROLS - FREE NODES
    OpEqualNode              = 3001,   // MENU CONTROLS - DOUBLE NODES
    OpNodeConnectivityNb     = 3002,   // MENU CONTROLS - NODE CONNECTIVITY NUMBER
    OpFreeEdge               = 3100,   // MENU CONTROLS - FREE EDGES
    OpFreeBorder             = 3101,   // MENU CONTROLS - FREE BORDERS
    OpLength                 = 3102,   // MENU CONTROLS - LENGTH
    OpConnection             = 3103,   // MENU CONTROLS - DOUBLE NODES
    OpEqualEdge              = 3104,   // MENU CONTROLS - BORDERS AT MULTICONNECTION
    OpFreeFace               = 3200,   // MENU CONTROLS - FREE FACES
    OpBareBorderFace         = 3201,   // MENU CONTROLS - FACES WITH BARE BORDER
    OpOverConstrainedFace    = 3202,   // MENU CONTROLS - OVERCONSTRAINED FACES
    OpLength2D               = 3203,   // MENU CONTROLS - LENGTH 2D
    OpConnection2D           = 3204,   // MENU CONTROLS - BORDERS ON MULTICONNECTION 2D
    OpArea                   = 3205,   // MENU CONTROLS - AREA
    OpTaper                  = 3206,   // MENU CONTROLS - TAPER
    OpAspectRatio            = 3207,   // MENU CONTROLS - ASPECT RATIO
    OpMinimumAngle           = 3208,   // MENU CONTROLS - MINIMUM ANGLE
    OpWarpingAngle           = 3209,   // MENU CONTROLS - WARPING ANGLE
    OpSkew                   = 3210,   // MENU CONTROLS - SKEW
    OpMaxElementLength2D     = 3211,   // MENU CONTROLS - ELEMENT DIAMETER 2D
    OpEqualFace              = 3212,   // MENU CONTROLS - DOUBLE FACES
    OpAspectRatio3D          = 3300,   // MENU CONTROLS - ASPECT RATIO 3D
    OpVolume                 = 3301,   // MENU CONTROLS - VOLUME
    OpMaxElementLength3D     = 3302,   // MENU CONTROLS - ELEMENT DIAMETER 3D
    OpBareBorderVolume       = 3303,   // MENU CONTROLS - VOLUMES WITH BARE BORDER
    OpOverConstrainedVolume  = 3304,   // MENU CONTROLS - OVERCONSTRAINED VOLUMES
    OpEqualVolume            = 3305,   // MENU CONTROLS - DOUBLE VOLUMES
    OpOverallMeshQuality     = 3400,   // MENU CONTROLS - OVERALL MESH QUALITY
    // Modification -------------------//--------------------------------
    OpNode                   = 4000,   // MENU MODIFICATION - ADD - NODE
    OpElem0D                 = 4001,   // MENU MODIFICATION - ADD - 0D ELEMENT
    OpElem0DOnElemNodes      = 4002,   // MENU MODIFICATION - ADD - 0D ELEMENTS ON ELEMENT NODES
    OpBall                   = 4003,   // MENU MODIFICATION - ADD - BALL
    OpEdge                   = 4004,   // MENU MODIFICATION - ADD - EDGE
    OpTriangle               = 4005,   // MENU MODIFICATION - ADD - TRIANGLE
    OpQuadrangle             = 4006,   // MENU MODIFICATION - ADD - QUADRANGLE
    OpPolygon                = 4007,   // MENU MODIFICATION - ADD - POLYGON
    OpTetrahedron            = 4008,   // MENU MODIFICATION - ADD - TETRAHEDRON
    OpHexahedron             = 4009,   // MENU MODIFICATION - ADD - HEXAHEDRON
    OpPentahedron            = 4010,   // MENU MODIFICATION - ADD - PENTAHEDRON
    OpPyramid                = 4011,   // MENU MODIFICATION - ADD - PYRAMID
    OpHexagonalPrism         = 4012,   // MENU MODIFICATION - ADD - HEXAGONAL PRISM
    OpPolyhedron             = 4013,   // MENU MODIFICATION - ADD - POLYHEDRON
    OpQuadraticEdge          = 4100,   // MENU MODIFICATION - ADD - QUADRATIC EDGE
    OpQuadraticTriangle      = 4101,   // MENU MODIFICATION - ADD - QUADRATIC TRIANGLE
    OpBiQuadraticTriangle    = 4102,   // MENU MODIFICATION - ADD - BIQUADRATIC TRIANGLE
    OpQuadraticQuadrangle    = 4103,   // MENU MODIFICATION - ADD - QUADRATIC QUADRANGLE
    OpBiQuadraticQuadrangle  = 4104,   // MENU MODIFICATION - ADD - BIQUADRATIC QUADRANGLE
    OpQuadraticTetrahedron   = 4105,   // MENU MODIFICATION - ADD - QUADRATIC TETRAHEDRON
    OpQuadraticPyramid       = 4106,   // MENU MODIFICATION - ADD - QUADRATIC PYRAMID
    OpQuadraticPentahedron   = 4107,   // MENU MODIFICATION - ADD - QUADRATIC PENTAHEDRON
    OpQuadraticHexahedron    = 4108,   // MENU MODIFICATION - ADD - QUADRATIC HEXAHEDRON
    OpTriQuadraticHexahedron = 4109,   // MENU MODIFICATION - ADD - TRIQUADRATIC HEXAHEDRON
    OpQuadraticPolygon       = 4110,   // MENU MODIFICATION - ADD - QUADRATIC POLYGON
    OpRemoveNodes            = 4200,   // MENU MODIFICATION - REMOVE - NODE
    OpRemoveElements         = 4201,   // MENU MODIFICATION - REMOVE - ELEMENTS
    OpRemoveOrphanNodes      = 4202,   // MENU MODIFICATION - REMOVE - ORPHAN NODES
    OpDeleteGroup            = 4210,   // MENU MODIFICATION - REMOVE - DELETE GROUPS WITH CONTENTS
    OpClearMesh              = 4220,   // MENU MODIFICATION - REMOVE - CLEAR MESH DATA
    OpRenumberingNodes       = 4300,   // MENU MODIFICATION - RENUMBERING - NODES
    OpRenumberingElements    = 4301,   // MENU MODIFICATION - RENUMBERING - ELEMENTS
    OpTranslation            = 4400,   // MENU MODIFICATION - TRANSFORMATION - TRANSLATION
    OpRotation               = 4401,   // MENU MODIFICATION - TRANSFORMATION - ROTATION
    OpSymmetry               = 4402,   // MENU MODIFICATION - TRANSFORMATION - SYMMETRY
    OpScale                  = 4403,   // MENU MODIFICATION - TRANSFORMATION - SCALE TRANSFORM
    OpSewing                 = 4404,   // MENU MODIFICATION - TRANSFORMATION - SEWING
    OpMergeNodes             = 4405,   // MENU MODIFICATION - TRANSFORMATION - MERGE NODES
    OpMergeElements          = 4406,   // MENU MODIFICATION - TRANSFORMATION - MERGE ELEMENTS
    OpDuplicateNodes         = 4407,   // MENU MODIFICATION - TRANSFORMATION - DUPLICATE NODES OR/AND ELEMENTS
    OpMoveNode               = 4500,   // MENU MODIFICATION - MOVE NODE
    OpDiagonalInversion      = 4501,   // MENU MODIFICATION - DIAGONAL INVERSION
    OpUnionOfTwoTriangle     = 4502,   // MENU MODIFICATION - UNION OF TWO TRIANGLE
    OpOrientation            = 4503,   // MENU MODIFICATION - ORIENTATION
    OpReorientFaces          = 4504,   // MENU MODIFICATION - REORIENT FACES BY VECTOR
    OpUnionOfTriangles       = 4505,   // MENU MODIFICATION - UNION OF TRIANGLES
    OpCuttingOfQuadrangles   = 4506,   // MENU MODIFICATION - CUTTING OF QUADRANGLES
    OpSplitVolumes           = 4507,   // MENU MODIFICATION - SPLIT VOLUMES
    OpSmoothing              = 4508,   // MENU MODIFICATION - SMOOTHING
    OpExtrusion              = 4509,   // MENU MODIFICATION - EXTRUSION
    OpExtrusionAlongAPath    = 4510,   // MENU MODIFICATION - EXTRUSION ALONG A PATH
    OpRevolution             = 4511,   // MENU MODIFICATION - REVOLUTION
    OpPatternMapping         = 4512,   // MENU MODIFICATION - PATTERN MAPPING
    OpConvertMeshToQuadratic = 4513,   // MENU MODIFICATION - CONVERT TO/FROM QUADRATIC
    OpCreateBoundaryElements = 4514,   // MENU MODIFICATION - CREATE BOUNDARY ELEMENTS
    OpSplitBiQuadratic       = 4515,   // MENU MODIFICATION - SPLIT BI-QUADRATIC TO LINEAR
    // Measurements -------------------//--------------------------------
    OpPropertiesLength       = 5000,   // MENU MEASUREMENTS - BASIC PROPERTIES - LENGTH
    OpPropertiesArea         = 5001,   // MENU MEASUREMENTS - BASIC PROPERTIES - AREA
    OpPropertiesVolume       = 5002,   // MENU MEASUREMENTS - BASIC PROPERTIES - VOLUME
    OpMinimumDistance        = 5003,   // MENU MEASUREMENTS - MINIMUM DISTANCE
    OpBoundingBox            = 5004,   // MENU MEASUREMENTS - BOUNDING BOX
    // Hypothesis ---------------------//--------------------------------
    OpEditHypothesis         = 6000,   // POPUP MENU - EDIT HYPOTHESIS
    OpUnassign               = 6001,   // POPUP MENU - UNASSIGN
    // Numbering ----------------------//--------------------------------
    OpNumberingNodes         = 6010,   // POPUP MENU - NUMBERING - DISPLAY NODES
    OpNumberingElements      = 6011,   // POPUP MENU - NUMBERING - DISPLAY ELEMENTS
    // AutoColor ----------------------//--------------------------------
    OpAutoColor              = 6020,   // POPUP MENU - AUTO COLOR
    OpDisableAutoColor       = 6021,   // POPUP MENU - DISABLE AUTO COLOR
    // DisplayMode --------------------//--------------------------------
    OpDMWireframe            = 6030,   // POPUP MENU - DISPLAY MODE - WIREFRAME
    OpDMShading              = 6031,   // POPUP MENU - DISPLAY MODE - SHADING
    OpDMNodes                = 6032,   // POPUP MENU - DISPLAY MODE - NODES
    OpDMShrink               = 6033,   // POPUP MENU - DISPLAY MODE - SHRINK
    // DisplayEntity ------------------//--------------------------------
    OpDE0DElements           = 6040,   // POPUP MENU - DISPLAY ENTITY - 0D ELEMENTS
    OpDEEdges                = 6041,   // POPUP MENU - DISPLAY ENTITY - EDGES
    OpDEFaces                = 6042,   // POPUP MENU - DISPLAY ENTITY - FACES
    OpDEVolumes              = 6043,   // POPUP MENU - DISPLAY ENTITY - VOLUMES
    OpDEBalls                = 6044,   // POPUP MENU - DISPLAY ENTITY - BALLS
    OpDEAllEntity            = 6045,   // POPUP MENU - DISPLAY ENTITY - ALL ENTITIES
    OpDEChoose               = 6046,   // POPUP MENU - DISPLAY ENTITY - CHOOSE ENTITIES
    // Representation -----------------//--------------------------------
    OpRepresentationLines    = 6050,   // POPUP MENU - 2D QUADRATIC - LINES
    OpRepresentationArcs     = 6051,   // POPUP MENU - 2D QUADRATIC - ARCS
    // OrientationOnFaces -------------//--------------------------------
    OpOrientationOnFaces     = 6060,   // POPUP MENU - ORIENTATION ON FACES
    // PropertiesGUI ------------------//--------------------------------
    OpProperties             = 6070,   // POPUP MENU - PROPERTIES
    // Transparency -------------------//--------------------------------
    OpTransparency           = 6080,   // POPUP MENU - TRANSPARENCY
    // Display ------------------------//--------------------------------
    OpShow                   = 6090,   // POPUP MENU - SHOW
    OpHide                   = 6091,   // POPUP MENU - HIDE
    OpShowOnly               = 6092,   // POPUP MENU - SHOW ONLY
    // Clipping -----------------------//--------------------------------
    OpClipping               = 6100,   // POPUP MENU - CLIPPING
    // SortChild ----------------------//--------------------------------
    OpSortChild              = 6110,   // POPUP MENU - SORT CHILDREN
    // Advanced -----------------------//--------------------------------
    OpAdvancedNoOp           = 10000,  // NO OPERATION (advanced operations base)
    //@@ insert new functions before this line @@ do not remove this line @@//
    OpLastOperationID        = 20000   // DO NOT USE OPERATION IDs MORE THAN 20000 !!!
  };
}

#endif // SMESHGUI_OPERATIONS_H
