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
}


#endif
