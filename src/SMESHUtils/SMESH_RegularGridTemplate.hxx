// Copyright (C) 2016-2024  CEA, EDF
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
// File      : SMESH_TemplateRegularGrid.hxx
// Created   : Fry April 05 11:20 2024
// Author    : Cesar Conopoima (cce)


#ifndef __SMESH_RegularGridTemplate_HXX__
#define __SMESH_RegularGridTemplate_HXX__

//STD
#include <functional>

//SMESH::Utils
#include "SMESH_RegularGrid.hxx"

namespace SMESHUtils
{
  template<typename FUNCTION>
  void SMESH_RegularGrid::foreachGridSide( const FUNCTION& fSide ) const
  {
    for (size_t edgeType = BOTTOM; edgeType <= LEFT; edgeType++)
      fSide( static_cast<EdgeType>( edgeType ) );  
  }

  template<typename FUNCTION>
  void SMESH_RegularGrid::foreachGridFace( const FUNCTION& fSide ) const
  {
    for (size_t faceType = B_BOTTOM; faceType <= B_TOP; faceType++)
      fSide( static_cast<FaceType>( faceType ) );  
  }
  
  void SMESH_RegularGrid::foreachNodeOnSide( SMESH_RegularGrid::EdgeType edge, const std::function<void(const std::shared_ptr<gp_Pnt> point, const int index )>& function ) const
  {
    for (auto node : nodesOfSide(edge))
      function(myCoordinates[node],node);      
  }

  void SMESH_RegularGrid::foreachNodeOnFace( SMESH_RegularGrid::FaceType face, const std::function<void(const std::shared_ptr<gp_Pnt> point, const int index )>& function ) const
  {
    for (auto node : nodesOfFace(face))
      function(myCoordinates[node],node);      
  }

  template<typename T>
  std::vector<T> SMESH_RegularGrid::getEdgeIndexLimits( const EdgeType edge ) const
  {
    switch ( edge ) {
      case EdgeType::BOTTOM:
        return std::vector<T>{1,1,mnx,1};
        break;
      case EdgeType::RIGHT:
        return std::vector<T>{mnx,1,mnx,mny};
        break;
      case EdgeType::TOP:
        return std::vector<T>{mnx,mny,1,mny};
        break;
      case EdgeType::LEFT:
        return std::vector<T>{1,mny,1,1};
        break;
      default:
        return std::vector<T>{1,1,mnx,1};
        break;
    }
  }

  template<typename T>
  std::vector<T> SMESH_RegularGrid::getFaceIndexLimits( const FaceType face ) const
  {
    switch ( face ) {
      case FaceType::B_BOTTOM:
        return std::vector<T>{1,1,1,mnx,mny,1};     /*V0-V2*/
        break;
      case FaceType::B_RIGHT:
        return std::vector<T>{mnx,1,1,mnx,mny,mnz}; /*V1-V6*/
        break;
      case FaceType::B_BACK:
        return std::vector<T>{1,mny,1,mnx,mny,mnz}; /*V3-V6*/
        break;
      case FaceType::B_LEFT:
        return std::vector<T>{1,1,1,1,mny,mnz};     /*V0-V7*/
        break;
      case FaceType::B_FRONT:
        return std::vector<T>{1,1,1,mnx,1,mnz};     /*V0-V5*/
        break;
      case FaceType::B_TOP:
        return std::vector<T>{1,1,mnz,mnx,mny,mnz}; /*V4-V6*/
        break;
      default:
        return std::vector<T>{1,1,1,mnx,mny,1};
        break;
    }
  }

  template<typename T>
  void SMESH_RegularGrid::getAllEdgeIndexLimits(std::vector<std::vector<T>>& allRanges)
  {
    this->foreachGridSide( [&]( EdgeType edge )
    {
      allRanges.push_back( getEdgeIndexLimits<T>(edge) );
    });
  }

  template<typename T>
  void SMESH_RegularGrid::getAllFaceIndexLimits(std::vector<std::vector<T>>& allRanges)
  {
    this->foreachGridFace( [&]( FaceType face )
    {
      allRanges.push_back( getFaceIndexLimits<T>(face) );
    });
  }
}


#endif
