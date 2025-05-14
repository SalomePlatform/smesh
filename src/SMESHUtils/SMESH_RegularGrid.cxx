// Copyright (C) 2016-2025  CEA, EDF
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
// File      : SMESH_StructuredGrid.cxx
// Created   : Sun March 24 09:58 2024
// Author    : Cesar Conopoima (cce)

//CAS
#include <BRep_Tool.hxx>

// SMESH
#include "SMDS_MeshNode.hxx"
#include "SMESH_TypeDefs.hxx"
#include "SMESH_RegularGridTemplate.hxx"

using namespace SMESHUtils;

SMESH_RegularGrid::SMESH_RegularGrid( const int id, const int nx, const int ny, const int nz ) :
myId(id),
mnx(nx),
mny(ny),
mnz(nz),
mns(nx*ny*nz)
{
  myCoordinates.Resize(0,mnx*mny*mnz-1, false);
  myCoordinates.Init(nullptr);
}

void SMESH_RegularGrid::SetNode( const std::shared_ptr<gp_Pnt>& point, const int iIndex, const int jIndex, const int zIndex )
{
  int index =  zIndex * mnx * mny + jIndex * mnx + iIndex;
  if ( mns > 0 && index < mns && myCoordinates[ index ] == nullptr )
    myCoordinates[ index ] = point;
}

void SMESH_RegularGrid::SetNode( const SMDS_MeshNode* point, const int iIndex, const int jIndex, const int zIndex )
{

  int index =  zIndex * mnx * mny + jIndex * mnx + iIndex;
  if ( mns > 0 && index < mns && myCoordinates[ index ] == nullptr )
    myCoordinates[ index ] = std::move( std::make_shared<gp_Pnt>(point->X(),point->Y(),point->Z()));
}

void SMESH_RegularGrid::SetNode( const SMDS_MeshNode* point, const int index )
{
  if ( mns > 0 && index < mns && myCoordinates[ index ] == nullptr )
    myCoordinates[ index ] = std::move( std::make_shared<gp_Pnt>(point->X(),point->Y(),point->Z()));
}

const std::shared_ptr<gp_Pnt> SMESH_RegularGrid::GetNode( const int iIndex, const int jIndex, const int zIndex )
{
  int index =  zIndex * mnx * mny + jIndex * mnx + iIndex;
  if ( index > -1 && index < mns )
    return myCoordinates[ index ];
  else
    return nullptr;
}


std::tuple<int,int,int> SMESH_RegularGrid::GetIJK(const int index) const
{
  int K = int(index/(mnx*mny));
  int J = int(index/mnx) % mny;
  int I = index % mnx;
  return std::tuple<int,int,int>(I,J,K);
}

std::vector<int> SMESH_RegularGrid::getEdgeIndexLimitsInverted( const EdgeType edge ) const
{
  auto limits = getEdgeIndexLimits<int>( edge );
  std::swap(limits[0],limits[2]);
  std::swap(limits[1],limits[3]);
  return limits;
}

std::vector<int> SMESH_RegularGrid::getFaceIndexLimits( const int start, const int end ) const
{
  auto startIJK = GetIJK(start);
  auto endIJK   = GetIJK(end);
  int iStart    = std::get<0>(startIJK);
  int jStart    = std::get<1>(startIJK);
  int kStart    = std::get<2>(startIJK);
  int iEnd      = std::get<0>(endIJK);
  int jEnd      = std::get<1>(endIJK);
  int kEnd      = std::get<2>(endIJK);
  return std::vector<int>{iStart+1,jStart+1,kStart+1,iEnd+1,jEnd+1,kEnd+1};   
}

std::vector<int> SMESH_RegularGrid::getEdgeIndexLimits( const int start, const int end ) const
{
  auto startIJK = GetIJK(start);
  auto endIJK   = GetIJK(end);
  int iStart    = std::get<0>(startIJK);
  int jStart    = std::get<1>(startIJK);
  int iEnd      = std::get<0>(endIJK);
  int jEnd      = std::get<1>(endIJK);  
  return std::vector<int>{iStart+1,jStart+1,iEnd+1,jEnd+1};   
}

int SMESH_RegularGrid::getEdgeSize( const EdgeType edge ) const
{
   switch ( edge ) {
    case EdgeType::BOTTOM:
    case EdgeType::TOP:
      return mnx;
      break;
    case EdgeType::RIGHT:
    case EdgeType::LEFT:
      return mny;
      break;
    default:
      return mnx;
      break;
  }
}

int SMESH_RegularGrid::getFaceSize( const FaceType edge ) const
{
   switch ( edge ) {
    case FaceType::B_BOTTOM:
    case FaceType::B_TOP:
      return mnx*mny;
      break;
    case FaceType::B_RIGHT:
    case FaceType::B_LEFT:
      return mny*mnz;
      break;
    case FaceType::B_FRONT:
    case FaceType::B_BACK:
      return mnx*mnz;
      break;      
    default:
      return mnx;
      break;
  }
}


std::vector<int> SMESH_RegularGrid::nodesOfFace( SMESH_RegularGrid::FaceType face ) const
{
  size_t faceSize=0;
  if ( face == SMESH_RegularGrid::B_BOTTOM || face == SMESH_RegularGrid::B_TOP )
    faceSize = mnx*mny;
  if ( face == SMESH_RegularGrid::B_LEFT || face == SMESH_RegularGrid::B_RIGHT )
    faceSize = mny*mnz;
  if ( face == SMESH_RegularGrid::B_BACK || face == SMESH_RegularGrid::B_FRONT )
    faceSize = mnx*mnz;
  
  std::vector<int> index(faceSize);
  switch ( face ) {
    case FaceType::B_BOTTOM:
      for (size_t i = 0; i < faceSize; i++)      
        index[ i ] = i;            
      break;
    case FaceType::B_TOP:
      for (size_t i = 0; i < faceSize; i++)
        index[ i ] = mnx * mny * mnz - mnx * mny + i;
      break;
    case FaceType::B_RIGHT:
      for (size_t i = 0; i < faceSize; i++)
      {
        auto dv = std::div(i,mny);
        index[ i ] = dv.quot * mnx * mny + (mnx-1)*(dv.rem+1)+dv.rem;        
      }
      break;
    case FaceType::B_LEFT:
      for (size_t i = 0; i < faceSize; i++)
      {
        auto dv = std::div(i,mny);
        index[ i ] =  dv.quot * mnx * mny + (dv.rem) * mnx;      
      }
      break;
    case FaceType::B_BACK:
      for (size_t i = 0; i < faceSize; i++)
      {
        auto dv = std::div(i,mnx);
        index[ i ] = dv.quot * mnx * mny + mny * mnx - mnx + dv.rem; 
      }
      break;
    case FaceType::B_FRONT:
      for (size_t i = 0; i < faceSize; i++)
      {
        auto dv = std::div(i,mnx);
        index[ i ] = dv.quot * mnx * mny + dv.rem;      
      }
      break;
    default:
      break;
  }
  return index;
}

std::vector<int> SMESH_RegularGrid::nodesOfSide( SMESH_RegularGrid::EdgeType edge ) const
{
  size_t sideSize = ( edge == SMESH_RegularGrid::BOTTOM ) || ( edge == SMESH_RegularGrid::TOP ) ? mnx : mny;
  std::vector<int> index(sideSize);
  switch ( edge ) {
    case EdgeType::BOTTOM:
      for (size_t i = 0; i < sideSize; i++)
        index[ i ] = i;      
      break;
    case EdgeType::RIGHT:
      for (size_t i = 0; i < sideSize; i++)
        index[ i ] = mnx*(i+1)-1;
      break;
    case EdgeType::TOP:
      for (size_t i = 0; i < sideSize; i++)
        index[ i ] = mnx*mny-(i+1);      
      break;
    case EdgeType::LEFT:
      for (size_t i = 0; i < sideSize; i++)
        index[ i ] = mnx*mny-(i+1)*mnx;
      break;
    default:
      break;
  }
  return index;
}

// Return the index of the nodes at begin and end of the edge
std::pair<int,int> SMESH_RegularGrid::getEdgeLimits( const EdgeType edge ) const
{
  switch ( edge ) {
    case EdgeType::BOTTOM:
      return std::make_pair(0,mnx-1);
      break;
    case EdgeType::RIGHT:
      return std::make_pair(mnx-1,mnx*mny-1);
      break;
    case EdgeType::TOP:
      return std::make_pair(mnx*mny-1,mnx*mny-mnx);
      break;
    case EdgeType::LEFT:
      return std::make_pair(mnx*mny-mnx,0);
      break;
    default:
      return std::make_pair(0,mnx-1);
      break;
  }
}

int SMESH_RegularGrid::getFaceCoordinateIndex( const VertexType v ) const
{
  switch ( v ) {
    case VertexType::V0:
      return 0;
      break;
    case VertexType::V1:
      return mnx-1;
      break;
    case VertexType::V2:
      return mnx*mny-1;
      break;
    case VertexType::V3:
      return mnx*mny-mnx;
      break;
    case VertexType::V4:
      return mnx*mny*mnz-mnx*mny;
      break;
    case VertexType::V5:
      return mnx*mny*mnz-mnx*mny+mnx-1;
      break;
    case VertexType::V6:
      return mnx*mny*mnz-1;
      break;
    case VertexType::V7:
      return mnx*mny*mnz-mnx;
      break;
    default:
      return 0;
      break;
  }
}

std::tuple<int,int,int,int> SMESH_RegularGrid::getFaceLimits( const FaceType face ) const
{
  switch ( face ) {
    case FaceType::B_BOTTOM: /*V0-V2-V1-V3*/
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V0),
                                          getFaceCoordinateIndex(VertexType::V2),
                                          getFaceCoordinateIndex(VertexType::V1),
                                          getFaceCoordinateIndex(VertexType::V3)};    
      break;
    case FaceType::B_RIGHT:  /*V1-V6-V2-V5*/
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V1),
                                          getFaceCoordinateIndex(VertexType::V6),
                                          getFaceCoordinateIndex(VertexType::V2),
                                          getFaceCoordinateIndex(VertexType::V5)}; 
      break;
    case FaceType::B_BACK:  /*V3-V6-V2-V7*/
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V3),
                                          getFaceCoordinateIndex(VertexType::V6),
                                          getFaceCoordinateIndex(VertexType::V2),
                                          getFaceCoordinateIndex(VertexType::V7)};
      break;
    case FaceType::B_LEFT:  /*V0-V7-V3-V4*/
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V0),
                                          getFaceCoordinateIndex(VertexType::V7),
                                          getFaceCoordinateIndex(VertexType::V3),
                                          getFaceCoordinateIndex(VertexType::V4)};
      break;
    case FaceType::B_FRONT: /*V0-V5-V1-V4*/
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V0),
                                          getFaceCoordinateIndex(VertexType::V5),
                                          getFaceCoordinateIndex(VertexType::V1),
                                          getFaceCoordinateIndex(VertexType::V4)};
      break;
    case FaceType::B_TOP:
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V4),
                                          getFaceCoordinateIndex(VertexType::V6),
                                          getFaceCoordinateIndex(VertexType::V5),
                                          getFaceCoordinateIndex(VertexType::V7)};
      break;
    default:
      return std::tuple<int,int,int,int>{getFaceCoordinateIndex(VertexType::V0),
                                          getFaceCoordinateIndex(VertexType::V2),
                                          getFaceCoordinateIndex(VertexType::V1),
                                          getFaceCoordinateIndex(VertexType::V3)};    
      break;
  }
}

// Find the left bottom corner of the box face from the 4 points and return its index
// Auxiliary for the GetCommontInterface function
static int findLeftBottomCorner(const std::shared_ptr<gp_Pnt>& V0, 
                                const std::shared_ptr<gp_Pnt>& V1,
                                const std::shared_ptr<gp_Pnt>& V2,
                                const std::shared_ptr<gp_Pnt>& V3)
{
  // Find the left bottom corner of the box face from the 4 points
  auto isLeftBottom = [](const std::shared_ptr<gp_Pnt>& a, const std::shared_ptr<gp_Pnt>& b) {
    const double tol = Precision::Confusion();
    return (b->X()-a->X() > tol) ||
      (fabs(b->X()-a->X()) < tol && b->Y()-a->Y() > tol) ||
      (fabs(b->X()-a->X()) < tol && fabs(b->Y()-a->Y()) < tol && b->Z()-a->Z() > tol);
  };

  int index = 0;

  if (isLeftBottom(V1, V0)) {
    index = 1;
  }
  if (isLeftBottom(V2, (index == 1 ? V1 : V0))) {
    index = 2;
  }
  if (isLeftBottom(V3, (index == 2 ? V2 : (index == 1 ? V1 : V0)))) {
    index = 3;
  }

  return index;
}

// This function transforms the index limits of a face based on the position of the left bottom corner.
// It ensures that the face limits are correctly oriented by swapping indices if necessary.
// Auxiliary for the GetCommontInterface function
void SMESH_RegularGrid::transformIndexLimits(FaceType face,
                                             std::vector<int>& faceLimits,
                                             const std::shared_ptr<gp_Pnt>& V0, 
                                             const std::shared_ptr<gp_Pnt>& V1,
                                             const std::shared_ptr<gp_Pnt>& V2, 
                                             const std::shared_ptr<gp_Pnt>& V3)
{
  int indexOfLeftBottom = findLeftBottomCorner(V0, V1, V2, V3);
  if(indexOfLeftBottom == 0)
    return;

  if(indexOfLeftBottom == 1)
  {
    std::swap(faceLimits[0],faceLimits[3]);
    std::swap(faceLimits[1],faceLimits[4]);
    std::swap(faceLimits[2],faceLimits[5]);
    return;
  }

  if(face == FaceType::B_BOTTOM || face == FaceType::B_TOP)
  {
    indexOfLeftBottom == 2? std::swap(faceLimits[0],faceLimits[3]) : std::swap(faceLimits[1],faceLimits[4]);
  }
  else if(face == FaceType::B_RIGHT || face == FaceType::B_LEFT)
  {
    indexOfLeftBottom == 2? std::swap(faceLimits[1],faceLimits[4]) : std::swap(faceLimits[2],faceLimits[5]);
  }
  else if(face == FaceType::B_BACK || face == FaceType::B_FRONT)
  {
    indexOfLeftBottom == 2? std::swap(faceLimits[0],faceLimits[3]) : std::swap(faceLimits[2],faceLimits[5]);
  }
}

void SMESH_RegularGrid::GetCommontInterface( FaceType face, SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  const double tol = Precision::Confusion(); /*confusion is 1e-7, the recommended tolerance to find coincident points in 3D*/
  auto vertex = getFaceLimits( face );
  auto v0 = myCoordinates[std::get<0>(vertex)];
  auto v1 = myCoordinates[std::get<1>(vertex)];
  auto v2 = myCoordinates[std::get<2>(vertex)];
  auto v3 = myCoordinates[std::get<3>(vertex)];

  auto foundOneTrue = [](int begin, int end, std::vector<bool>& v ) -> bool
  { 
    for ( int i = begin; i <= end; i++ )
      if ( v[i] )
        return true;
    return false;
  };

  auto foundTrueColum = [](int colum, std::vector<bool>& v ) -> bool
  { 
    int strikeSize = 4;
    for ( int i = 0; i < strikeSize; i++ )
      if ( v[colum + i*strikeSize] )
        return true;
    return false;
  };

  grid->foreachGridFace( [&]( FaceType gridFace )
  {
    std::vector<int> interfaceRange,interfaceDonor,tranformationRange,tranformationDonor;
    
    auto neighboorVertex = grid->getFaceLimits( gridFace );

    auto v4 = grid->GetNode(std::get<0>(neighboorVertex));
    auto v5 = grid->GetNode(std::get<1>(neighboorVertex));
    auto v6 = grid->GetNode(std::get<2>(neighboorVertex));
    auto v7 = grid->GetNode(std::get<3>(neighboorVertex));

    std::vector<bool> trueTable({ v0->IsEqual(*v4,tol),v0->IsEqual(*v5,tol),v0->IsEqual(*v6,tol),v0->IsEqual(*v7,tol),
                                  v1->IsEqual(*v4,tol),v1->IsEqual(*v5,tol),v1->IsEqual(*v6,tol),v1->IsEqual(*v7,tol),
                                  v2->IsEqual(*v4,tol),v2->IsEqual(*v5,tol),v2->IsEqual(*v6,tol),v2->IsEqual(*v7,tol),
                                  v3->IsEqual(*v4,tol),v3->IsEqual(*v5,tol),v3->IsEqual(*v6,tol),v3->IsEqual(*v7,tol)});
                                  
    std::vector<bool> trueCorner({foundOneTrue(0,3,trueTable),
                                  foundOneTrue(4,7,trueTable),
                                  foundOneTrue(8,11,trueTable),
                                  foundOneTrue(12,15,trueTable) });
  
    if ( trueCorner[0] && 
         trueCorner[1] && 
         trueCorner[2] &&
         trueCorner[3] )   /*Face to Face interface 100% conform*/
    {
      interfaceRange = this->getFaceIndexLimits<int>( face );
      interfaceDonor = grid->getFaceIndexLimits<int>( gridFace );
    }
    else if ( trueCorner[0] || trueCorner[1]  ||
              trueCorner[2] || trueCorner[3] ) /*Partial Face to Face. Only one intersection then all the other 3 vertex are: 2 in the edges 1 inside the face*/
    {
      // Two possible cases:
      // 1) face < gridFace
      if ( this->getFaceSize( face ) < grid->getFaceSize( gridFace ) )
      {
        auto nodeToSearch = trueCorner[0] ? v1 : trueCorner[2] ? v3 : trueCorner[1] ? v0 : v2;

        grid->foreachNodeOnFace( gridFace, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex) 
        {
          if ( nodeToSearch->IsEqual( *sidePoint, tol ) ) 
          {
            interfaceRange = this->getFaceIndexLimits<int>( face );
            
            auto startIndex = foundTrueColum(0,trueTable) ? std::get<0>(neighboorVertex) : 
                                foundTrueColum(2,trueTable) ? std::get<2>(neighboorVertex) : 
                                  foundTrueColum(1,trueTable) ? std::get<1>(neighboorVertex) :
                                    std::get<3>(neighboorVertex);

            interfaceDonor = startIndex < nodeIndex ? 
                              grid->getFaceIndexLimits( startIndex, nodeIndex ) : 
                                grid->getFaceIndexLimits( nodeIndex, startIndex );
            return;
          }
        });
      }
      // 2) face > gridEdge
      else if ( this->getFaceSize( face ) > grid->getFaceSize( gridFace )  )
      {
        auto nodeToSearch = foundTrueColum(0,trueTable) ? v5 : 
                                foundTrueColum(2,trueTable) ? v7 : 
                                  foundTrueColum(1,trueTable) ? v4 : v6;

        this->foreachNodeOnFace( face, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex ) 
        {     
          if ( nodeToSearch->IsEqual( *sidePoint, tol ) ) 
          {      
            auto startIndex = trueCorner[0] ? std::get<0>(vertex) : 
                                trueCorner[2] ? std::get<2>(vertex) : 
                                  trueCorner[1] ? std::get<1>(vertex) : std::get<3>(vertex);

            interfaceRange = startIndex < nodeIndex ? 
                                this->getFaceIndexLimits( startIndex, nodeIndex) : 
                                  this->getFaceIndexLimits( nodeIndex, startIndex );

            interfaceDonor = grid->getFaceIndexLimits<int>( gridFace );
            return;
          }
        });
      }
    }

    if ( !interfaceRange.empty() && !interfaceDonor.empty() )
    {
      // Transform the index limits of the face based on the position of the left bottom corner
      transformIndexLimits(face, interfaceRange, v0, v1, v2, v3);
      transformIndexLimits(gridFace, interfaceDonor, v4, v5, v6, v7);

      tranformationRange  = this->computeTransformation( face, gridFace, interfaceRange, interfaceDonor );
      tranformationDonor  = grid->computeTransformation( gridFace, face, interfaceDonor, interfaceRange );
      interface           = std::vector<int>{(int)face}+interfaceRange+interfaceDonor+tranformationRange;
      auto dualInterface  = std::vector<int>{(int)face}+interfaceDonor+interfaceRange+tranformationDonor;
      this->setInterface( face, grid->id(), interface );
      grid->setInterface( gridFace, this->id(), dualInterface );
    }
  });
}

void SMESH_RegularGrid::GetCommontInterface( EdgeType edge, SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  const double tol = Precision::Confusion(); /*confusion is 1e-7, the recommended tolerance to find coincident points in 3D*/
  auto vertex = getEdgeLimits( edge );
  auto v0 = myCoordinates[vertex.first];
  auto v1 = myCoordinates[vertex.second];

  grid->foreachGridSide( [&]( EdgeType gridEdge )
  {
    std::vector<int> interfaceRange,interfaceDonor,tranformationRange,tranformationDonor;
    auto neighboorVertex = grid->getEdgeLimits( gridEdge );
    auto v2 = grid->GetNode(neighboorVertex.first);
    auto v3 = grid->GetNode(neighboorVertex.second);  

    std::vector<bool> trueTable({ v0->IsEqual(*v2,tol),
                                  v0->IsEqual(*v3,tol),
                                  v1->IsEqual(*v2,tol),
                                  v1->IsEqual(*v3,tol) });

    if ( (trueTable[0] || trueTable[1] ) && /*Case start-end vertex are coincident in both edges trivial*/
          (trueTable[2] ||trueTable[3] ) )
    {
      interfaceRange = this->getEdgeIndexLimits<int>( edge );
      interfaceDonor = grid->getEdgeIndexLimitsInverted( gridEdge );   
    }
    else if ( trueTable[0] || /*Case start OR end vertex are coincident in both edges*/
              trueTable[1] || 
              trueTable[2] || 
              trueTable[3] ) 
    {      
      // Two possible cases:
      // 1) edge < gridEdge means that v1 is going to be found as an intersection inside gridEdge side!!!
      if ( this->getEdgeSize( edge ) < grid->getEdgeSize( gridEdge ) )
      {
        auto nodeToSearch = (trueTable[0] || trueTable[1]) ? v1 : v0;
        grid->foreachNodeOnSide( gridEdge, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex) 
        {       
          if ( nodeToSearch->IsEqual( *sidePoint, tol ) ) 
          {
            interfaceRange = this->getEdgeIndexLimits<int>( edge );
            auto startIndex = (trueTable[0] || trueTable[2]) ? neighboorVertex.first : neighboorVertex.second;
            interfaceDonor = startIndex < nodeIndex ? 
                              grid->getEdgeIndexLimits( startIndex, nodeIndex ) : 
                                grid->getEdgeIndexLimits( nodeIndex, startIndex );
              
          }          
        });
      }
      // 2) edge > gridEdge means that v2 or v3 are going to be found as an intersection inside edge side!!!
      else if ( this->getEdgeSize( edge ) > grid->getEdgeSize( gridEdge )  )
      {
        auto nodeToSearch = (trueTable[0] || trueTable[2]) ? v3 : v2;
        this->foreachNodeOnSide( edge, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex) 
        {                  
          if ( nodeToSearch->IsEqual( *sidePoint, tol ) ) 
          {          
            auto startIndex = (trueTable[0] || trueTable[1]) ? vertex.first : vertex.second;            
            interfaceRange = startIndex < nodeIndex ? 
                                this->getEdgeIndexLimits( startIndex, nodeIndex) : 
                                  this->getEdgeIndexLimits( nodeIndex, startIndex );

            interfaceDonor = grid->getEdgeIndexLimitsInverted( gridEdge );              
          }          
        });
      }      
    }
    else  /*Case nor start nor end are coincident in both edges but one of the edges can be embemded completely in the other*/
    {
      // Discard quickly non parallel and far edges
      auto v01 = v1->Coord()-v0->Coord();
      auto v23 = v3->Coord()-v2->Coord();
      if ( std::fabs( v01.Dot(v23) ) == 1.0 /*parallel edges might have intersection*/ )
      {
        double v01Module = v01.Modulus();
        double v23Module = v23.Modulus();
        auto ref         = v01Module > v23Module ? v01 : v23;
        auto v0Ref       = v01Module > v23Module ? v0->Coord() : v2->Coord();
        auto v2Ref       = v01Module > v23Module ? std::pair<gp_XYZ,gp_XYZ>(v2->Coord(),v3->Coord()) : std::pair<gp_XYZ,gp_XYZ>(v0->Coord(),v1->Coord());

        auto t  = ref.X() != 0. ? std::pair<double,double>( (v2Ref.first.X()-v0Ref.X())/ref.X(), (v2Ref.second.X()-v0Ref.X())/ref.X()) : 
                    ref.Y() != 0. ? std::pair<double,double>( (v2Ref.first.Y()-v0Ref.Y())/ref.Y(), (v2Ref.second.Y()-v0Ref.Y())/ref.Y()) : 
                      std::pair<double,double>( (v2Ref.first.Z()-v0Ref.Z())/ref.Z(), (v2Ref.second.Z()-v0Ref.Z())/ref.Z());
        
        // check if the smaller edge contains a point inside the bigger one
        // use the 3D line equation for this. 
        //  1) t goes from [0,1] in the reference (bigger) edge. Check if any of the points of the other edge is 0 <= t12 <= 1       
        
        if ( (t.first >= 0. && t.first <= 1.) || (t.second >= 0. && t.second <= 1.) )
        {
          bool isFirstVertex  = t.first >= 0. && t.first <= 1.; 
          bool isSecondVertex = t.second >= 0. && t.second <= 1.;

          if ( v01Module > v23Module && (isFirstVertex && isSecondVertex) /*100% edge embedded in the other && edge>gridEdge*/ )
          {
            int startIndex  = -1;
            int endIndex    = -1;
            bool firstToBeFound = true; 
            this->foreachNodeOnSide( edge, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex) 
            {                  
              if ( v2->IsEqual( *sidePoint, tol ) || v3->IsEqual( *sidePoint, tol ) ) 
              {       
                if ( firstToBeFound )
                {
                  startIndex = nodeIndex;
                  firstToBeFound = false;
                }
                else
                  endIndex = nodeIndex;                  
              }          
            });

            if ( startIndex != -1 && endIndex != -1 )
            {
              interfaceRange = this->getEdgeIndexLimits( startIndex, endIndex );
              interfaceDonor = grid->getEdgeIndexLimitsInverted( gridEdge );
            }             
          }
          else if ( (isFirstVertex && isSecondVertex) /*100% edge embedded in the other && gridEdge>edge*/  )
          {
            int startIndex  = -1;
            int endIndex    = -1;
            bool firstToBeFound = true;
            grid->foreachNodeOnSide( gridEdge, [&] (const std::shared_ptr<gp_Pnt> sidePoint, const int nodeIndex) 
            {       
              if ( v0->IsEqual( *sidePoint, tol ) || v1->IsEqual( *sidePoint, tol ) ) 
              {
                if ( firstToBeFound )
                {
                  startIndex = nodeIndex;
                  firstToBeFound = false;
                }
                else
                  endIndex = nodeIndex;                  
              }          
            });

            if ( startIndex != -1 && endIndex != -1 )
            {
              interfaceRange = this->getEdgeIndexLimits<int>( edge );
              interfaceDonor = grid->getEdgeIndexLimits( startIndex, endIndex );
            }
          }          
        } 
      }
    }

    if ( !interfaceRange.empty() && !interfaceDonor.empty() )
    {      
      tranformationRange  = this->computeTransformation( edge, gridEdge, interfaceRange, interfaceDonor );
      tranformationDonor  = grid->computeTransformation( gridEdge, edge, interfaceDonor, interfaceRange );
      interface           = std::vector<int>{(int)edge} + interfaceRange+interfaceDonor+tranformationRange;
      auto dualInterface  = std::vector<int>{(int)edge} + interfaceDonor+interfaceRange+tranformationDonor;
      this->setInterface( edge, grid->id(), interface );
      grid->setInterface( gridEdge, this->id(), dualInterface );
    }    
  });
}

std::vector<int> SMESH_RegularGrid::computeTransformation( const SMESH_RegularGrid::EdgeType edge, SMESH_RegularGrid::EdgeType gridDonorEdge, 
                                                            std::vector<int>& interfaceRange, std::vector<int>& interfaceDonor ) const
{
  std::vector<int> transform(2);
  int r[2], d[2];
  for(int i = 0; i < 2; i++) {
    r[i] = interfaceRange[i + 2] - interfaceRange[i];
    d[i] = interfaceDonor[i + 2] - interfaceDonor[i];
  }
  for(int i = 0; i < 2; i++) {
    transform[i] = 0;
    for(int j = 0; j < 2; j++) {
      if(std::abs(r[i]) == std::abs(d[j])) { // == 0 on an interface
        transform[i] = j + 1;
        if(!r[i] && !d[j]) { // on an interface
          // both interfaces correspond to a min index or to a max index
          bool topBottom = (edge == SMESH_RegularGrid::BOTTOM || edge == SMESH_RegularGrid::TOP) && (gridDonorEdge == SMESH_RegularGrid::BOTTOM || gridDonorEdge == SMESH_RegularGrid::TOP);
          bool leftRight = (edge == SMESH_RegularGrid::RIGHT || edge == SMESH_RegularGrid::LEFT) && (gridDonorEdge == SMESH_RegularGrid::RIGHT || gridDonorEdge == SMESH_RegularGrid::LEFT);
          if( topBottom || leftRight ) 
            transform[i] *= -1;
        }
        else {
          if(r[i] * d[j] < 0) transform[i] *= -1;
        }
      }
    }
  }
  return transform;
}

std::vector<int> SMESH_RegularGrid::computeTransformation( const SMESH_RegularGrid::FaceType face, SMESH_RegularGrid::FaceType gridDonorFace, 
                                                            std::vector<int>& interfaceRange, std::vector<int>& interfaceDonor ) const
{
  std::vector<int> transform(3);
  int r[3], d[3];
  for(int i = 0; i < 3; i++) {
    r[i] = interfaceRange[i + 3] - interfaceRange[i];
    d[i] = interfaceDonor[i + 3] - interfaceDonor[i];
  }
  for(int i = 0; i < 3; i++) {
    transform[i] = 0;
    for(int j = 0; j < 3; j++) {
      if(std::abs(r[i]) == std::abs(d[j])) { // == 0 on an interface
        transform[i] = j + 1;
        if(!r[i] && !d[j]) { // on an interface
          // both interfaces correspond to a min index or to a max index
          bool bothMin = ( face == SMESH_RegularGrid::B_BOTTOM || 
                            face == SMESH_RegularGrid::B_LEFT || 
                            face == SMESH_RegularGrid::B_FRONT ) && 
                            (gridDonorFace == SMESH_RegularGrid::B_BOTTOM || 
                              gridDonorFace == SMESH_RegularGrid::B_LEFT || 
                              gridDonorFace == SMESH_RegularGrid::B_FRONT );

          bool bothMax = (face == SMESH_RegularGrid::B_TOP || 
                          face == SMESH_RegularGrid::B_RIGHT || 
                          face == SMESH_RegularGrid::B_BACK ) && 
                            (gridDonorFace == SMESH_RegularGrid::B_TOP || 
                              gridDonorFace == SMESH_RegularGrid::B_RIGHT ||
                              gridDonorFace == SMESH_RegularGrid::B_BACK );

          if( bothMin || bothMax ) 
            transform[i] *= -1;
        }
        else {
          if(r[i] * d[j] < 0) transform[i] *= -1;
        }
      }
    }
  }  
  return transform;
}

void SMESH_RegularGrid::setInterface( SMESH_RegularGrid::EdgeType edge, const int gridId, std::vector<int>& interface )
{
  myInterfaceMap[edge][gridId] = interface;  
}

void SMESH_RegularGrid::setInterface( SMESH_RegularGrid::FaceType face, const int gridId, std::vector<int>& interface )
{
  myFaceInterafaceMap[face][gridId] = interface;  
}

SMESH_RegularGrid::FaceType SMESH_RegularGrid::getFaceTypeByGeomFace( TopoDS_Shape shapeFace ) const
{
  const double tol = Precision::Confusion(); 
  SMESH_RegularGrid::FaceType foundFace = SMESH_RegularGrid::B_NONE;
  this->foreachGridFace( [&]( FaceType face )
  {    
    auto faceLimits = getFaceLimits(face);
    auto v0 = myCoordinates[std::get<0>(faceLimits)];
    auto v1 = myCoordinates[std::get<1>(faceLimits)];
    auto v2 = myCoordinates[std::get<2>(faceLimits)];
    auto v3 = myCoordinates[std::get<3>(faceLimits)];  
    int nVertex = 0;
    int nVertexInFace = 0;
    for ( TopExp_Explorer fEx( shapeFace, TopAbs_VERTEX ); fEx.More(); fEx.Next() )
    {
      gp_Pnt pV = BRep_Tool::Pnt( TopoDS::Vertex(fEx.Current()) );
      nVertexInFace += int( v0->IsEqual(pV,tol) ) + int( v1->IsEqual(pV,tol) ) + int( v2->IsEqual(pV,tol) ) + int( v3->IsEqual(pV,tol) ) ;
      nVertex++;        
    }
    if ( nVertex == nVertexInFace ) 
    {
      foundFace = face;
      return;
    }
  });
 
  return foundFace;
}

SMESH_RegularGrid::EdgeType SMESH_RegularGrid::getEdgeTypeByGeomEdge( TopoDS_Shape shapeEdge ) const
{
  const double tol = Precision::Confusion(); 
  SMESH_RegularGrid::EdgeType foundEdge = SMESH_RegularGrid::NONE;
  this->foreachGridSide ( [&]( EdgeType edge )
  {    
    auto edgeLimits = getEdgeLimits(edge);
    auto v0 = myCoordinates[edgeLimits.first];
    auto v1 = myCoordinates[edgeLimits.second];
    int nVertex = 0;
    int nVertexInEdge = 0;
    for ( TopExp_Explorer fEx( shapeEdge, TopAbs_VERTEX ); fEx.More(); fEx.Next() )
    {
      gp_Pnt pV = BRep_Tool::Pnt( TopoDS::Vertex(fEx.Current()) );
      nVertexInEdge += int( v0->IsEqual(pV,tol) ) + int( v1->IsEqual(pV,tol) );
      nVertex++;        
    }
    if ( nVertex == nVertexInEdge ) 
    {
      foundEdge = edge;
      return;
    }
  });
 
  return foundEdge;
}

bool SMESH_RegularGrid::GetPrecomputedInterface( EdgeType edge, const SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  if ( myInterfaceMap.count(edge) && myInterfaceMap[edge].count(grid->id()) )
  {
    interface = myInterfaceMap[edge][grid->id()];
    return true;
  }
  return false;
}

bool SMESH_RegularGrid::GetPrecomputedInterface( FaceType face, const SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  if ( myFaceInterafaceMap.count(face) && myFaceInterafaceMap[face].count(grid->id()) )
  {
    interface = myFaceInterafaceMap[face][grid->id()];
    return true;
  }
  return false;
}

void SMESH_RegularGrid::GetEdgeInterfaces( SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  this->foreachGridSide( [&]( EdgeType edge )
  {
    if ( !this->GetPrecomputedInterface( edge, grid, interface ) )
      this->GetCommontInterface( edge, grid, interface );
  });
}

void SMESH_RegularGrid::GetFaceInterfaces( SMESH_RegularGrid * grid, std::vector<int>& interface )
{
  this->foreachGridFace( [&]( FaceType face )
  {
    if ( !this->GetPrecomputedInterface( face, grid, interface ) )
      this->GetCommontInterface( face, grid, interface );
  });
}


SMESH_RegularGrid::~SMESH_RegularGrid()
{
  myInterfaceMap.clear();
  myFaceInterafaceMap.clear();
}
