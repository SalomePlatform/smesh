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
// File      : SMESH_MeshAlgos.hxx
// Created   : Tue Apr 30 18:00:36 2013
// Author    : Edward AGAPOV (eap)

// Initially this file held some low level algorithms extracted from SMESH_MeshEditor
// to make them accessible from Controls package, and more


#ifndef __SMESH_MeshAlgos_HXX__
#define __SMESH_MeshAlgos_HXX__

#include "SMESH_Utils.hxx"

#include "SMDSAbs_ElementType.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMESH_TypeDefs.hxx"

#include <TopAbs_State.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <vector>

class Bnd_B3d;
class gp_Ax1;
class SMDS_Mesh;
class SMDS_MeshElement;
class SMDS_MeshGroup;
class SMDS_MeshNode;

//=======================================================================
/*!
 * \brief Searcher for the node closest to a point
 */
//=======================================================================

struct SMESHUtils_EXPORT SMESH_NodeSearcher
{
  virtual const SMDS_MeshNode* FindClosestTo( const gp_Pnt& pnt ) = 0;
  virtual void MoveNode( const SMDS_MeshNode* node, const gp_Pnt& toPnt ) = 0;
  virtual int  FindNearPoint(const gp_Pnt&                        point,
                             const double                         tolerance,
                             std::vector< const SMDS_MeshNode* >& foundNodes) = 0;
  virtual ~SMESH_NodeSearcher() {}
};

//=======================================================================
/*!
 * \brief Searcher for elements
 */
//=======================================================================

struct SMESHUtils_EXPORT SMESH_ElementSearcher
{
  /*!
   * \brief Find elements of given type where the given point is IN or ON.
   *        Returns nb of found elements and elements them-selves.
   *
   * 'ALL' type means elements of any type excluding nodes and 0D elements
   */
  virtual int FindElementsByPoint(const gp_Pnt&                           point,
                                  SMDSAbs_ElementType                     type,
                                  std::vector< const SMDS_MeshElement* >& foundElems) = 0;
  /*!
   * \brief Return an element most close to the given point
   */
  virtual const SMDS_MeshElement* FindClosestTo( const gp_Pnt&       point,
                                                 SMDSAbs_ElementType type) = 0;
  /*!
   * \brief Return elements possibly intersecting the line
   */
  virtual void GetElementsNearLine( const gp_Ax1&                           line,
                                    SMDSAbs_ElementType                     type,
                                    std::vector< const SMDS_MeshElement* >& foundElems) = 0;
  /*!
   * \brief Return elements whose bounding box intersects a sphere
   */
  virtual void GetElementsInSphere( const gp_XYZ&                           center,
                                    const double                            radius,
                                    SMDSAbs_ElementType                     type,
                                    std::vector< const SMDS_MeshElement* >& foundElems) = 0;
  /*!
   * \brief Return elements whose bounding box intersects a given bounding box
   */
  virtual void GetElementsInBox( const Bnd_B3d&                          box,
                                 SMDSAbs_ElementType                     type,
                                 std::vector< const SMDS_MeshElement* >& foundElems) = 0;
  /*!
   * \brief Find out if the given point is out of closed 2D mesh.
   */
  virtual TopAbs_State GetPointState(const gp_Pnt& point) = 0;

  /*!
   * \brief Return a projection of a given point to a 2D mesh.
   *        Optionally return the closest face
   */
  virtual gp_XYZ Project(const gp_Pnt&            point,
                         SMDSAbs_ElementType      type,
                         const SMDS_MeshElement** closestFace= 0) = 0;

  virtual ~SMESH_ElementSearcher();
};

namespace SMESH_MeshAlgos
{
  /*!
   * \brief Return SMESH_NodeSearcher. The caller is responsible for deleting it
   */
  SMESHUtils_EXPORT
  SMESH_NodeSearcher* GetNodeSearcher( SMDS_Mesh& mesh );

  SMESHUtils_EXPORT
  SMESH_NodeSearcher* GetNodeSearcher( SMDS_ElemIteratorPtr elemIt );

  /*!
   * \brief Return SMESH_ElementSearcher. The caller is responsible for deleting it
   */
  SMESHUtils_EXPORT
  SMESH_ElementSearcher* GetElementSearcher( SMDS_Mesh& mesh,
                                             double     tolerance=-1.);
  SMESHUtils_EXPORT
  SMESH_ElementSearcher* GetElementSearcher( SMDS_Mesh& mesh,
                                             SMDS_ElemIteratorPtr elemIt,
                                             double     tolerance=-1. );


  /*!
   * \brief Return true if the point is IN or ON of the element
   */
  SMESHUtils_EXPORT
  bool IsOut( const SMDS_MeshElement* element, const gp_Pnt& point, double tol );

  SMESHUtils_EXPORT
  double GetDistance( const SMDS_MeshElement* elem, const gp_Pnt& point, gp_XYZ* closestPnt = 0 );

  SMESHUtils_EXPORT
  double GetDistance( const SMDS_MeshEdge* edge, const gp_Pnt& point, gp_XYZ* closestPnt = 0 );

  SMESHUtils_EXPORT
  double GetDistance( const SMDS_MeshFace* face, const gp_Pnt& point, gp_XYZ* closestPnt = 0 );

  SMESHUtils_EXPORT
  double GetDistance( const SMDS_MeshVolume* volume, const gp_Pnt& point, gp_XYZ* closestPnt = 0 );

  SMESHUtils_EXPORT
  void GetBarycentricCoords( const gp_XY& point,
                             const gp_XY& t0, const gp_XY& t1, const gp_XY& t2,
                             double &    bc0, double &    bc1);

  /*!
   * Return a face having linked nodes n1 and n2 and which is
   * - not in avoidSet,
   * - in elemSet provided that !elemSet.empty()
   * i1 and i2 optionally returns indices of n1 and n2
   */
  SMESHUtils_EXPORT
  const SMDS_MeshElement* FindFaceInSet(const SMDS_MeshNode*    n1,
                                        const SMDS_MeshNode*    n2,
                                        const TIDSortedElemSet& elemSet,
                                        const TIDSortedElemSet& avoidSet,
                                        int*                    i1=0,
                                        int*                    i2=0);
  /*!
   * \brief Calculate normal of a mesh face
   */
  SMESHUtils_EXPORT
  bool FaceNormal(const SMDS_MeshElement* F, gp_XYZ& normal, bool normalized=true);

  /*!
   * \brief Return number of nodes common to two elements
   */
  SMESHUtils_EXPORT
  int NbCommonNodes(const SMDS_MeshElement* e1,
                    const SMDS_MeshElement* e2);
  /*!
   * \brief Return nodes common to two elements
   */
  SMESHUtils_EXPORT
  std::vector< const SMDS_MeshNode*> GetCommonNodes(const SMDS_MeshElement* e1,
                                                    const SMDS_MeshElement* e2);
  /*!
   * \brief Return true if node1 encounters first in the face and node2, after.
   *        The nodes are supposed to be neighbor nodes in the face.
   */
  SMESHUtils_EXPORT
  bool IsRightOrder( const SMDS_MeshElement* face,
                     const SMDS_MeshNode*    node0,
                     const SMDS_MeshNode*    node1 );

  typedef std::vector< std::vector< const SMDS_MeshElement* > > TElemGroupVector;
  typedef std::vector< std::vector< const SMDS_MeshNode* > >    TNodeGroupVector;
  /*!
   * \brief Partition given 1D elements into groups of contiguous edges.
   *        A node where number of meeting edges != 2 is a group end.
   *        An optional startNode is used to orient groups it belongs to.
   * \return a list of edge groups and a list of corresponding node groups.
   *         If a group is closed, the first and last nodes of the group are same.
   */
  SMESHUtils_EXPORT
  void Get1DBranches( SMDS_ElemIteratorPtr edgeIt,
                      TElemGroupVector&    edgeGroups,
                      TNodeGroupVector&    nodeGroups,
                      const SMDS_MeshNode* startNode = 0 );

  /*!
   * \brief Mark elements given by SMDS_Iterator
   */
  template< class ElemIter >
  void MarkElems( ElemIter it, const bool isMarked )
  {
    while ( it->more() ) it->next()->setIsMarked( isMarked );
  }
  /*!
   * \brief Mark elements given by std iterators
   */
  template< class ElemIter >
  void MarkElems( ElemIter it, ElemIter end, const bool isMarked )
  {
    for ( ; it != end; ++it ) (*it)->setIsMarked( isMarked );
  }
  /*!
   * \brief Mark nodes of elements given by SMDS_Iterator
   */
  template< class ElemIter >
  void MarkElemNodes( ElemIter it, const bool isMarked, const bool markElem = false )
  {
    if ( markElem )
      while ( it->more() ) {
        const SMDS_MeshElement* e = it->next();
        e->setIsMarked( isMarked );
        MarkElems( e->nodesIterator(), isMarked );
      }
    else
      while ( it->more() )
        MarkElems( it->next()->nodesIterator(), isMarked );
  }
  /*!
   * \brief Mark elements given by std iterators
   */
  template< class ElemIter >
  void MarkElemNodes( ElemIter it, ElemIter end, const bool isMarked, const bool markElem = false )
  {
    if ( markElem )
      for ( ; it != end; ++it ) {
        (*it)->setIsMarked( isMarked );
        MarkElems( (*it)->nodesIterator(), isMarked );
      }
    else
      for ( ; it != end; ++it )
        MarkElems( (*it)->nodesIterator(), isMarked );
  }

  // 2 nodes + optional medium node
  struct Edge
  {
    const SMDS_MeshNode* _node1;
    const SMDS_MeshNode* _node2;
    const SMDS_MeshNode* _medium;
  };

  /*!
   * Return sharp edges of faces and non-manifold ones.
   * Optionally adds existing edges to the result. Angle is in degrees.
   */
  SMESHUtils_EXPORT
  std::vector< Edge > FindSharpEdges( SMDS_Mesh* mesh,
                                      double     angle,
                                      bool       addExisting );

  /*!
   * Distribute all faces of the mesh between groups using given edges.
   */
  SMESHUtils_EXPORT
  std::vector< std::vector< const SMDS_MeshElement* > >
  SeparateFacesByEdges( SMDS_Mesh* mesh, const std::vector< Edge >& edges );


  typedef std::vector<const SMDS_MeshNode*> TFreeBorder;
  typedef std::vector<TFreeBorder>          TFreeBorderVec;
  struct TFreeBorderPart
  {
    int _border; // border index within a TFreeBorderVec
    int _node1;  // node index within the border-th TFreeBorder
    int _node2;
    int _nodeLast;
  };
  typedef std::vector<TFreeBorderPart>  TCoincidentGroup;
  typedef std::vector<TCoincidentGroup> TCoincidentGroupVec;
  struct CoincidentFreeBorders
  {
    TFreeBorderVec      _borders;          // nodes of all free borders
    TCoincidentGroupVec _coincidentGroups; // groups of coincident parts of borders
  };

  /*!
   * Returns TFreeBorder's coincident within the given tolerance.
   * If the tolerance <= 0.0 then one tenth of an average size of elements adjacent
   * to free borders being compared is used.
   */
  SMESHUtils_EXPORT
  void FindCoincidentFreeBorders(SMDS_Mesh&              mesh,
                                 double                  tolerance,
                                 CoincidentFreeBorders & foundFreeBordes);
  // Implemented in ./SMESH_FreeBorders.cxx

  /*!
   * Returns all or only closed TFreeBorder's.
   * Optionally check if the mesh is manifold and if faces are correctly oriented.
   */
  SMESHUtils_EXPORT
  void FindFreeBorders(SMDS_Mesh&       mesh,
                       TFreeBorderVec & foundFreeBordes,
                       const bool       closedOnly,
                       bool*            isManifold = 0,
                       bool*            isGoodOri = 0);
  // Implemented in ./SMESH_FreeBorders.cxx

  /*!
   * Fill a hole defined by a TFreeBorder with 2D elements.
   */
  SMESHUtils_EXPORT
  void FillHole(const TFreeBorder &                   freeBorder,
                SMDS_Mesh&                            mesh,
                std::vector<const SMDS_MeshElement*>& newFaces);
  // Implemented in ./SMESH_FillHole.cxx

  /*!
   * \brief Find nodes whose merge makes the element invalid
   */
  SMESHUtils_EXPORT
  void DeMerge(const SMDS_MeshElement*              elem,
               std::vector< const SMDS_MeshNode* >& newNodes,
               std::vector< const SMDS_MeshNode* >& noMergeNodes);
  // Implemented in SMESH_DeMerge.cxx


  typedef std::vector< std::pair< const SMDS_MeshElement*, int > > TElemIntPairVec;
  typedef std::vector< std::pair< const SMDS_MeshNode*,    int > > TNodeIntPairVec;
  /*!
   * \brief Create an offset mesh of given faces
   *  \param [in] faceIt - the input faces
   *  \param [in] theFixIntersections - to fix self intersections of the offset mesh or not
   *  \param [out] new2OldFaces - history of faces
   *  \param [out] new2OldNodes - history of nodes
   *  \return SMDS_Mesh* - the new offset mesh, a caller should delete
   */
  SMESHUtils_EXPORT
  SMDS_Mesh* MakeOffset( SMDS_ElemIteratorPtr faceIt,
                         SMDS_Mesh&           mesh,
                         const double         offset,
                         const bool           theFixIntersections,
                         TElemIntPairVec&           new2OldFaces,
                         TNodeIntPairVec&           new2OldNodes );
  // Implemented in ./SMESH_Offset.cxx


  //=======================================================================
  /*!
   * \brief Cut faces of a triangular mesh.
   *  Usage work-flow: 1) call Cut() methods as many times as needed
   *                   2) call MakeNewFaces() to really modify the mesh faces
   */
  //=======================================================================
  // implemented in SMESH_Offset.cxx

  class SMESHUtils_EXPORT Intersector
  {
  public:
    Intersector( SMDS_Mesh* mesh, double tol, const std::vector< gp_XYZ >& normals );
    ~Intersector();

    //! Compute cut of two faces of the mesh
    void Cut( const SMDS_MeshElement* face1,
              const SMDS_MeshElement* face2,
              const int               nbCommonNodes = -1 );

    //! Store a face cut by a line given by its ends lying either on face edges or inside the face.
    //  Line ends are accompanied by indices of intersected face edges.
    //  Edge index is <0 if a line end is inside the face.
    void Cut( const SMDS_MeshElement* face,
              SMESH_NodeXYZ&          lineEnd1,
              int                     edgeIndex1,
              SMESH_NodeXYZ&          lineEnd2,
              int                     edgeIndex2 );

    //! Split all faces intersected by Cut() methods.
    //  theSign = (-1|1) is used to choose which part of a face cut by another one to remove.
    //  1 means to remove a part opposite to face normal.
    //  Optionally optimize quality of split faces by edge swapping.
    void MakeNewFaces( SMESH_MeshAlgos::TElemIntPairVec& theNew2OldFaces,
                       SMESH_MeshAlgos::TNodeIntPairVec& theNew2OldNodes,
                       const double                      theSign = 1.,
                       const bool                        theOptimize = false );

    typedef std::vector< SMESH_NodeXYZ > TFace;

    //! Cut a face by planes, whose normals point to parts to keep.
    //  Return true if the whole face is cut off
    static bool CutByPlanes(const SMDS_MeshElement*       face,
                            const std::vector< gp_Ax1 > & planes,
                            const double                  tol,
                            std::vector< TFace > &        newFaceConnectivity );

  private:
    struct Algo;
    Algo* myAlgo;
  };

  //=======================================================================
  /*!
   * \brief Divide a mesh face into triangles
   */
  //=======================================================================
  // Implemented in ./SMESH_Triangulate.cxx

  class SMESHUtils_EXPORT Triangulate
  {
  public:

    Triangulate(bool optimize=false);
    ~Triangulate();

    static int GetNbTriangles( const SMDS_MeshElement* face );

    int GetTriangles( const SMDS_MeshElement*             face,
                      std::vector< const SMDS_MeshNode*>& nodes);
  private:

    bool triangulate( std::vector< const SMDS_MeshNode*>& nodes, const size_t nbNodes );

    struct PolyVertex;
    struct Optimizer;
    struct Data;

    Data*      _data;
    Optimizer* _optimizer;
  };

  // structure used in MakePolyLine() to define a cutting plane
  struct PolySegment
  {
    // 2 points, each defined as follows:
    // ( myNode1 &&  myNode2 ) ==> point is in the middle of an edge defined by two nodes
    // ( myNode1 && !myNode2 ) ==> point is at myNode1 of a some face
    // else                    ==> point is at myXYZ
    const SMDS_MeshNode*    myNode1[2];
    const SMDS_MeshNode*    myNode2[2];
    gp_XYZ                  myXYZ  [2];

    // face on which myXYZ projects (found by MakePolyLine())
    const SMDS_MeshElement* myFace [2];

    // vector on the plane; to use a default plane set vector = (0,0,0)
    gp_Vec myVector;

    // point returning coordinates of a middle of the two points, projected to mesh
    gp_Pnt myMidProjPoint;
  };
  typedef std::vector<PolySegment> TListOfPolySegments;

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
   *  \param [in] group - an optional group where created mesh segments will
   *        be added.
   */
  // Implemented in ./SMESH_PolyLine.cxx
  SMESHUtils_EXPORT
  void MakePolyLine( SMDS_Mesh*                            mesh,
                     TListOfPolySegments&                  segments,
                     std::vector<const SMDS_MeshElement*>& newEdges,
                     std::vector<const SMDS_MeshNode*>&    newNodes,
                     SMDS_MeshGroup*                       group=0,
                     SMESH_ElementSearcher*                searcher=0);

  /*!
   * Create a slot of given width around given 1D elements lying on a triangle mesh.
   * The slot is constructed by cutting faces by cylindrical surfaces made around each segment.
   * \return Edges located at the slot boundary
   */
  // Implemented in ./SMESH_Slot.cxx
  SMESHUtils_EXPORT
  std::vector< Edge > MakeSlot( SMDS_ElemIteratorPtr             segmentIt,
                                double                           width,
                                SMDS_Mesh*                       mesh,
                                std::vector< SMDS_MeshGroup* > & groupsToUpdate);

} // namespace SMESH_MeshAlgos

#endif
