

/*----------------------------------------------------------*/
/*                                                                                                                      */
/*                                              LIBMESH V 5.46                                          */
/*                                                                                                                      */
/*----------------------------------------------------------*/
/*                                                                                                                      */
/*      Description:            handle .meshb file format I/O           */
/*      Author:                         Loic MARECHAL                                           */
/*      Creation date:          feb 16 2007                                                     */
/*      Last modification:      dec 09 2011                                                     */
/*                                                                                                                      */
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Defines                                                                                                      */
/*----------------------------------------------------------*/

#ifdef WIN32
 #if defined MESHDriverGMF_EXPORTS || defined MeshDriverGMF_EXPORTS
  #define MESHDriverGMF_EXPORT __declspec( dllexport )
 #else
  #define MESHDriverGMF_EXPORT __declspec( dllimport )
 #endif
#else
 #define MESHDriverGMF_EXPORT
#endif

#define GmfStrSiz 1024
#define GmfMaxTyp 1000
#define GmfMaxKwd 81
#define GmfMshVer 1
#define GmfRead 1
#define GmfWrite 2
#define GmfSca 1
#define GmfVec 2
#define GmfSymMat 3
#define GmfMat 4
#define GmfFloat 1
#define GmfDouble 2

/* see MeshGems/Docs/meshgems_formats_description.pdf */
enum GmfKwdCod
{
        GmfReserved1, \
        GmfVersionFormatted, \
        GmfReserved2, \
        GmfDimension, \
        GmfVertices, \
        GmfEdges, \
        GmfTriangles, \
        GmfQuadrilaterals, \
        GmfTetrahedra, \
        GmfPrisms, \
        GmfHexahedra, \
        GmfIterationsAll, \
        GmfTimesAll, \
        GmfCorners, \
        GmfRidges, \
        GmfRequiredVertices, \
        GmfRequiredEdges, \
        GmfRequiredTriangles, \
        GmfRequiredQuadrilaterals, \
        GmfTangentAtEdgeVertices, \
        GmfNormalAtVertices, \
        GmfNormalAtTriangleVertices, \
        GmfNormalAtQuadrilateralVertices, \
        GmfAngleOfCornerBound, \
        GmfTrianglesP2, \
        GmfEdgesP2, \
        GmfSolAtPyramids, \
        GmfQuadrilateralsQ2, \
        GmfISolAtPyramids, \
        GmfSubDomainFromGeom, \
        GmfTetrahedraP2, \
        GmfFault_NearTri, \
        GmfFault_Inter, \
        GmfHexahedraQ2, \
        GmfExtraVerticesAtEdges, \
        GmfExtraVerticesAtTriangles, \
        GmfExtraVerticesAtQuadrilaterals, \
        GmfExtraVerticesAtTetrahedra, \
        GmfExtraVerticesAtPrisms, \
        GmfExtraVerticesAtHexahedra, \
        GmfVerticesOnGeometricVertices, \
        GmfVerticesOnGeometricEdges, \
        GmfVerticesOnGeometricTriangles, \
        GmfVerticesOnGeometricQuadrilaterals, \
        GmfEdgesOnGeometricEdges, \
        GmfFault_FreeEdge, \
        GmfPolyhedra, \
        GmfPolygons, \
        GmfFault_Overlap, \
        GmfPyramids, \
        GmfBoundingBox, \
        GmfBody, \
        GmfPrivateTable, \
        GmfFault_BadShape, \
        GmfEnd, \
        GmfTrianglesOnGeometricTriangles, \
        GmfTrianglesOnGeometricQuadrilaterals, \
        GmfQuadrilateralsOnGeometricTriangles, \
        GmfQuadrilateralsOnGeometricQuadrilaterals, \
        GmfTangents, \
        GmfNormals, \
        GmfTangentAtVertices, \
        GmfSolAtVertices, \
        GmfSolAtEdges, \
        GmfSolAtTriangles, \
        GmfSolAtQuadrilaterals, \
        GmfSolAtTetrahedra, \
        GmfSolAtPrisms, \
        GmfSolAtHexahedra, \
        GmfDSolAtVertices, \
        GmfISolAtVertices, \
        GmfISolAtEdges, \
        GmfISolAtTriangles, \
        GmfISolAtQuadrilaterals, \
        GmfISolAtTetrahedra, \
        GmfISolAtPrisms, \
        GmfISolAtHexahedra, \
        GmfIterations, \
        GmfTime, \
        GmfFault_SmallTri, \
        GmfCoarseHexahedra, \
        GmfFault_MultipleEdge
};


/*----------------------------------------------------------*/
/* External procedures                                                                          */
/*----------------------------------------------------------*/

MESHDriverGMF_EXPORT extern int GmfOpenMesh(const char *, int, ...);
MESHDriverGMF_EXPORT extern int GmfCloseMesh(int);
MESHDriverGMF_EXPORT extern int GmfStatKwd(int, int, ...);
MESHDriverGMF_EXPORT extern int GmfGotoKwd(int, int);
MESHDriverGMF_EXPORT extern int GmfSetKwd(int, int, ...);
MESHDriverGMF_EXPORT extern void GmfGetLin(int, int, ...);
MESHDriverGMF_EXPORT extern void GmfSetLin(int, int, ...);


/*----------------------------------------------------------*/
/* Fortran 77 API                                                                                       */
/*----------------------------------------------------------*/

#if defined(F77_NO_UNDER_SCORE)
#define call(x) x
#else
#define call(x) x ## _
#endif


/*----------------------------------------------------------*/
/* Transmesh private API                                                                        */
/*----------------------------------------------------------*/

#ifdef TRANSMESH

MESHDriverGMF_EXPORT extern char *GmfKwdFmt[ GmfMaxKwd + 1 ][4];
MESHDriverGMF_EXPORT extern int GmfCpyLin(int, int, int);

#endif
