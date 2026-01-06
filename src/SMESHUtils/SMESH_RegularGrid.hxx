// Copyright (C) 2016-2026  CEA, EDF
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : SMESH_RegularGrid.hxx
// Created   : Sun March 24 09:58 2024
// Author    : Cesar Conopoima (cce)


#ifndef __SMESH_RegularGrid_HXX__
#define __SMESH_RegularGrid_HXX__

//OCC
#include <Precision.hxx>
#include <NCollection_Array1.hxx>
#include <gp_Pnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_DataMapOfShapeReal.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>

//STD
#include <functional>
#include <memory>
#include <vector>
#include <map>

#include "SMESH_Utils.hxx"
#include "SMDS_MeshNode.hxx"

class Adaptor3d_Surface;
class Adaptor2d_Curve2d;
class Adaptor3d_Curve;
class gp_Pnt;
class gp_XYZ;

template <typename T>
std::vector<T> operator+(std::vector<T> const &x, std::vector<T> const &y)
{
    std::vector<T> vec;
    vec.reserve(x.size() + y.size());
    vec.insert(vec.end(), x.begin(), x.end());
    vec.insert(vec.end(), y.begin(), y.end());
    return vec;
}

namespace SMESHUtils
{
  /*!
   * \brief Define a regular grid of nx,ny,nz dimension. A vector with gp_Pnt is
   *        defined by default by a canonical order, the canonical order is: 
   *        i-index goes faster, then j-index and finally z-index.
   *        This data structure was created to index mesh nodes created by Quadrangle_2D and Prism_3D meshers.
   *        The StructuredCGNS export driver uses this data structure to define zones, grid coordinate points and interfaces.
   */
  class SMESHUtils_EXPORT SMESH_RegularGrid
  {
  public:

    SMESH_RegularGrid( const int id, const int nx, const int ny, const int nz = 1 );
    void SetNode( const std::shared_ptr<gp_Pnt>& point, const int iIndex, const int jIndex, const int zIndex = 0 );
    void SetNode( const SMDS_MeshNode* point, const int iIndex, const int jIndex, const int zIndex = 0 );
    void SetNode( const SMDS_MeshNode* point, const int index );
    const std::shared_ptr<gp_Pnt> GetNode( const int index ) const { return myCoordinates[index]; };
    const std::shared_ptr<gp_Pnt> GetNode( const int iIndex, const int jIndex, const int zIndex );
    int nx() const { return mnx; };
    int ny() const { return mny; };
    int nz() const { return mnz; };
    int Size() { return mns;};
    NCollection_Array1<std::shared_ptr<gp_Pnt>>::Iterator CoordinateBegin() { return NCollection_Array1<std::shared_ptr<gp_Pnt>>::Iterator( myCoordinates ); };
    int id() const { return myId; };

    /*
      *              
      *     o------TOP (j=ny-1)----o
      *     |                      |
      *     |                      |
      *     |                      |
      *    LEFT (i=0)           RIGHT(i=nx-1)
      *     |                      |
      *     |                      |
      *     |                      |
      *     o------BOTTOM(j=0)-----o
    */
    enum EdgeType
    {
      BOTTOM,
      RIGHT,
      TOP,
      LEFT,
      NONE /**/
    };

    enum VertexType
    {
      V0,
      V1,
      V2,
      V3,
      V4,
      V5,
      V6,
      V7
    };
    /*    Define vertex numeration convention
      *
      *               V7                           V6 
      *                +--------+--------+--------+             
      *               /        /        /        /|              
      *              +--------+--------+--------+ | 
      *             /        /        /        /| | 
      *            +--------+--------+--------+ | + 
      *           /        /        /     V5 /| |/| 
      *       V4 +--------+--------+--------+ | + |   
      *          |     |                    | |/| |                
      *          |     |                    | + | +               
      *          |     |                    |/| |/|
      *          +     |                    | + |
      *          |     |                    | |/| |
      *          |     +-------+----------+-| + | + V2
      *          |    / V3                  |/| |/
      *          +   /                      | +
      *          |  /                       | |/
      *          | /                        | +
      *          |/                         |/
      *          +--------+--------+--------+
      *         V0                          V1
      * 
      * Canonical cartesian axis orientation
      * 
      *         ^   ^ j (or y)
      * k (or z)|  /
      *         | /
      *         |/
      *         +------> i (or x)
      * 
      * 
    */

    enum FaceType
    {
      B_BOTTOM, /*k=0   move (i,j)*/ 
      B_RIGHT,  /*i=mnx move (j,k)*/
      B_BACK,   /*j=mny move (i,k)*/
      B_LEFT,   /*i=0   move (j,k) */
      B_FRONT,  /*j=0   move (i,k)*/
      B_TOP,    /*k=mnz move (i,j) */            
      B_NONE    /**/
    };

    /*  
      *                +--------+--------+--------+             
      *               /        /        /        /|              
      *              +--------+--------+--------+ | 
      *             /        /  Top   /        /| | 
      *            +--------+--------+--------+ | + 
      *           /        /        /        /| |/| 
      *          +--------+--------+--------+ | + |   
      *          |        |        |        | |/| |   <- Right               
      *          |        |        |        | + | +               
      *          |        |        |        |/| |/|
      *          +--------+--------+--------+ | + |
      *  Left -> |        | Front  |        | |/| |
      *          |        |        |        | + | +
      *          |        |        |        |/| |/
      *          +--------+--------+--------+ | +
      *          |        |        |        | |/
      *          |        |        |        | +
      *          |        |        |        |/
      *          +--------+--------+--------+
      *                       ^
      *                       |
      *                     Bottom
      * 
      * Canonical cartesian axis orientation
      * 
      *         ^   ^ j (or y)
      * k (or z)|  /
      *         | /
      *         |/
      *         +------> i (or x)
      * 
      * 
    */  

    // \brief Given a grid and a potentially neighbor grid return the vector describing the interface used by the cgns mesh format
    // \remark the interface vector has the follow information:
    //            id      the target range       the donor range  the transformation
    //          {edgeId,    ibegin,jbegin,        iend,jend,    transformation}
    // \remark As interfaces are unique, the dual interface (interaction of the neighbor grid and the current one) is also defined
    // \param the neighbor grid
    // \return the interface vector filled    
    void GetEdgeInterfaces( SMESH_RegularGrid * grid, std::vector<int>& interface );
    
    // \brief Given a grid and a potentially neighbor grid return the vector describing the interface used by the cgns mesh format
    // \remark the interface vector has the follow information:
    //            id      the target range          the donor range  the transformation
    //          {edgeId,  ibegin,jbegin,kbegin,  iend,jend,kend,       transformation}
    // \remark As interfaces are unique, the dual interface (interaction of the neighbor grid and the current one) is also defined
    // \param the neighbor grid
    // \return the interface vector filled
    void GetFaceInterfaces( SMESH_RegularGrid * grid, std::vector<int>& interface );

    // \brief Fill the allRanges vector with the boundaries of the grid
    template<typename T>
    void getAllEdgeIndexLimits( std::vector<std::vector<T>>& allRanges );
    
    // \brief Fill the allRanges vector with the boundaries of the grid
    template<typename T>
    void getAllFaceIndexLimits( std::vector<std::vector<T>>& allRanges );

    // \brief Get limits of the edge in the order (ibegin,jbegin,iend,jend)+1 because index in CGNS are not zero based
    template<typename T>
    std::vector<T> getEdgeIndexLimits( const EdgeType edge ) const;
    
    // \brief Get limits of the edge in the order (iend,jend,ibegin,jbegin)+1 because index in CGNS are not zero based
    std::vector<int> getEdgeIndexLimitsInverted( const EdgeType edge ) const;

    // \brief Get limits of the face in the order (ibegin,jbegin,kbegin,iend,jend,kend)+1 because index in CGNS are not zero based
    template<typename T>
    std::vector<T> getFaceIndexLimits( SMESH_RegularGrid::FaceType face ) const;

    // \brief Return the faceType to which the passed geometrical face belongs to
    SMESH_RegularGrid::FaceType getFaceTypeByGeomFace( TopoDS_Shape shapeFace ) const;
    
    // \brief Return the edgeType to which the passed geometrical edge belongs to
    SMESH_RegularGrid::EdgeType getEdgeTypeByGeomEdge( TopoDS_Shape shapeEdge ) const;
   
    ~SMESH_RegularGrid();    

  protected:

    // Utility functions

    // \brief Given one of the edge types and a grid determine whether or not they are adjacent on that side
    // \remark There are three types of interfaces.
    //          1) end to end intersection, both sides are exactly the same
    //          2) one of the sides is smaller than the other but the grid are conform so the intersection is given exactly node by node
    //          3) the same for 2) but nodes are not coincident
    // \param edge, the edge where we are checking for interface
    // \param grid, a neighbor grid
    // \return interface, the interface vector filled in case an interface was found
    void GetCommontInterface( EdgeType edge, SMESH_RegularGrid * grid, std::vector<int>& interface );    
    void GetCommontInterface( FaceType face, SMESH_RegularGrid * grid, std::vector<int>& interface );

    // \brief Accessor method to interfaces already computed by other grid
    // \param edge, the edge where we are checking for interface
    // \param grid, a neighbor grid
    // \param interface, the interface vector filled in case an interface was found
    bool GetPrecomputedInterface( EdgeType edge, const SMESH_RegularGrid * grid, std::vector<int>& interface );

    // \brief Accessor method to interfaces already computed by other grid
    // \param face, the face where we are checking for interface
    // \param grid, a neighbor grid
    // \param interface, the interface vector filled in case an interface was found
    bool GetPrecomputedInterface( FaceType face, const SMESH_RegularGrid * grid, std::vector<int>& interface );
           
    // \brief Set the interface found between two neighbor grids. 
    // \remark fill the inner map between edges and grids used to recover precomputed interfaces with GetPrecomputedInterface method
    void setInterface( SMESH_RegularGrid::EdgeType edge, const int gridId, std::vector<int>& interface );

    // \brief Set the interface found between two neighbor grids. 
    // \remark fill the inner map between edges and grids used to recover precomputed interfaces with GetPrecomputedInterface method
    void setInterface( SMESH_RegularGrid::FaceType face, const int gridId, std::vector<int>& interface );

    // \brief Get the coordinate indexes defining the limit of the edge
    std::pair<int,int> getEdgeLimits( const SMESH_RegularGrid::EdgeType edge ) const;
    
    // \brief Get the coordinate indexes defining the limit of the face
    std::tuple<int,int,int,int> getFaceLimits( const SMESH_RegularGrid::FaceType face ) const;

    // \brief Get limits of the edge in the order (ibegin,jbegin,iend,jend)+1 because index in CGNS are not zero based
    std::vector<int> getEdgeIndexLimits( const int start, const int end ) const;
    std::vector<int> getFaceIndexLimits( const int start, const int end ) const; 

    // \brief Compute the transformation vector following the rules of the cgns format
    std::vector<int> computeTransformation( const SMESH_RegularGrid::EdgeType edge, SMESH_RegularGrid::EdgeType gridDonorEdge, std::vector<int>& interfaceRange, std::vector<int>& interfaceDonor ) const;

    // \brief Compute the transformation vector following the rules of the cgns format
    std::vector<int> computeTransformation( const SMESH_RegularGrid::FaceType face, SMESH_RegularGrid::FaceType gridDonorFace, std::vector<int>& interfaceRange, std::vector<int>& interfaceDonor ) const;
  
    // \brief Number of nodes on the edge
    int getEdgeSize(const EdgeType edge ) const;
    
    // \brief Number of nodes on the face
    int getFaceSize(const FaceType face ) const;
    
    // \brief Coordinate index of the given vertex
    int getFaceCoordinateIndex( const VertexType v ) const;

    // \brief Given the index of a coordinate return his (i,j,k) location in the grid
    // \param the index in the coordinates vector  
    // \return a tuple with the (i,j,k) coordinates of the point in the grid
    std::tuple<int,int,int> GetIJK( const int index ) const;
    
    // \brief Iterator function to execute the 'fSide' function for each edge side of the grid
    // \remark utility function to be used as:
    //            grid->foreachGridSide( [&]( EdgeType edge )
    //            {
    //                  // Inner logic to be execute for each side of the grid
    //            });    
    template<typename FUNCTION>
    void foreachGridSide( const FUNCTION& fSide ) const;

    // \brief Iterator function to execute the 'fSide' function for each face side of the grid
    // \remark utility function to be used as:
    //            grid->foreachGridFace( [&]( FaceType face )
    //            {
    //                  // Inner logic to be execute for each face side of the grid
    //            });
    template<typename FUNCTION>
    void foreachGridFace( const FUNCTION& fSide ) const;

    // \brief Iterator function to execute the lambda function with argument for each grid point and index in the gridEdge
    // \remark utility function to be used as:
    //            grid->foreachNodeOnSide( gridEdge, [&]( (const std::shared_ptr<gp_Pnt> , const int nodeIndex )  )
    //            {
    //                  // Inner logic to be execute for each node and index of the passed grid edge side
    //            });    
    void foreachNodeOnSide( SMESH_RegularGrid::EdgeType edge, const std::function<void(const std::shared_ptr<gp_Pnt> point, const int index )>& function ) const;
    
    // \brief Iterator function to execute the lambda function with argument for each grid point and index in the gridFace
    // \remark utility function to be used as:
    //            grid->foreachNodeOnFace( gridFace, [&]( (const std::shared_ptr<gp_Pnt>, const int nodeIndex )  )
    //            {
    //                  // Inner logic to be execute for each node and index of the passed grid face side
    //            });
    void foreachNodeOnFace( SMESH_RegularGrid::FaceType face, const std::function<void(const std::shared_ptr<gp_Pnt> point, const int index )>& function ) const;

    // \brief Get all the indices of nodes in an edge side
    // \param edge, the edge of the grid
    // \return the index of all the nodes in the edge    
    std::vector<int> nodesOfSide( SMESH_RegularGrid::EdgeType edge ) const;
    
    // \brief Get all the indices of nodes in an face side
    // \param face, the face of the grid
    // \return the index of all the nodes in the face 
    std::vector<int> nodesOfFace( SMESH_RegularGrid::FaceType face ) const;

    // This function transforms the index limits of a face based on the position of the left bottom corner.
    // It ensures that the face limits are correctly oriented by swapping indices if necessary.
    // Auxiliary for the GetCommontInterface function
    void transformIndexLimits(FaceType face, 
                              std::vector<int>& faceLimits, 
                              const std::shared_ptr<gp_Pnt>& V0, 
                              const std::shared_ptr<gp_Pnt>& V1,
                              const std::shared_ptr<gp_Pnt>& V2, 
                              const std::shared_ptr<gp_Pnt>& V3);

  private:
    int myId;
    int mnx,mny,mnz,mns;
    NCollection_Array1<std::shared_ptr<gp_Pnt>> myCoordinates;
    
    std::map<SMESH_RegularGrid::EdgeType,std::map<int,std::vector<int>>> myInterfaceMap; 
    std::map<SMESH_RegularGrid::FaceType,std::map<int,std::vector<int>>> myFaceInterafaceMap;
  };
}

#endif
