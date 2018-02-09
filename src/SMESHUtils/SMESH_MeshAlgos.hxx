// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// This file holds some low level algorithms extracted from SMESH_MeshEditor
// to make them accessible from Controls package, and more


#ifndef __SMESH_MeshAlgos_HXX__
#define __SMESH_MeshAlgos_HXX__

#include "SMESH_Utils.hxx"

#include "SMDSAbs_ElementType.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMESH_TypeDefs.hxx"

#include <TopAbs_State.hxx>
#include <vector>

class gp_Pnt;
class gp_Ax1;
class Bnd_B3d;
class SMDS_MeshNode;
class SMDS_MeshElement;
class SMDS_Mesh;

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


  typedef std::vector< std::pair< const SMDS_MeshElement*, const SMDS_MeshElement* > > TEPairVec;
  typedef std::vector< std::pair< const SMDS_MeshNode*, const SMDS_MeshNode* > >       TNPairVec;
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
                         TEPairVec&           new2OldFaces,
                         TNPairVec&           new2OldNodes );
  // Implemented in ./SMESH_Offset.cxx


  /*!
   * \brief Divide a mesh face into triangles
   */
  // Implemented in ./SMESH_Triangulate.cxx

  class SMESHUtils_EXPORT Triangulate
  {
  public:

    static int GetNbTriangles( const SMDS_MeshElement* face );

    int GetTriangles( const SMDS_MeshElement*             face,
                      std::vector< const SMDS_MeshNode*>& nodes);
  private:

    bool triangulate( std::vector< const SMDS_MeshNode*>& nodes, const size_t nbNodes );

    /*!
     * \brief Vertex of a polygon. Together with 2 neighbor Vertices represents a triangle
     */
    struct PolyVertex
    {
      SMESH_NodeXYZ _nxyz;
      gp_XY         _xy;
      PolyVertex*   _prev;
      PolyVertex*   _next;

      void   SetNodeAndNext( const SMDS_MeshNode* n, PolyVertex& v );
      void   GetTriaNodes( const SMDS_MeshNode** nodes) const;
      double TriaArea() const;
      bool   IsInsideTria( const PolyVertex* v );
      PolyVertex* Delete();
    };
    std::vector< PolyVertex > _pv;
  };


} // namespace SMESH_MeshAlgos

#endif
