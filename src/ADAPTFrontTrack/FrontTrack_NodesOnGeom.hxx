// Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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
// File      : FrontTrack_NodesOnGeom.hxx
// Created   : Tue Apr 25 19:12:25 2017
// Author    : Edward AGAPOV (eap)


#ifndef __FrontTrack_NodesOnGeom_HXX__
#define __FrontTrack_NodesOnGeom_HXX__

#include "FrontTrack_Projector.hxx"

#include <Bnd_Box.hxx>
#include <NCollection_DataMap.hxx>
#include <TopoDS_Shape.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>

#include <string>
#include <vector>

namespace FT_Utils {
  struct XaoGroups;
}
namespace MEDCoupling {
  class DataArrayDouble;
}
namespace XAO {
  class BrepGeometry;
}

  //--------------------------------------------------------------------------------------------
/*!
 * \brief Node group and geometry to project onto
 */
class FT_NodesOnGeom
{
public:

  // read node IDs form a file and try to find a boundary sub-shape by name
  void read( const std::string&            nodesFile,
             const FT_Utils::XaoGroups&    xaoGroups,
             MEDCoupling::DataArrayDouble* nodeCoords,
             std::vector< FT_Projector > * allProjectorsByDim);

  // chose boundary shapes by evaluating distance between nodes and shapes
  //void choseShape( const std::vector< FT_Utils::ShapeAndBndBox >& shapeAndBoxList );

  // project nodes to the shapes and move them to new positions
  void projectAndMove();

  // return true if all nodes were successfully relocated
  bool isOK() const { return _OK; }

  // return dimension of boundary shapes
  int getShapeDim() const { return _shapeDim; }

  // return nb of nodes to move
  int nbNodes() const { return _nodes.size(); }


private:

  // put nodes in the order for optimal projection
  void putNodesInOrder();

  // get node coordinates
  gp_Pnt getPoint( const int nodeID );

  // change node coordinates
  void moveNode( const int nodeID, const gp_Pnt& xyz );


  // Ids of a node to move and its 2 or 4 neighbors
  struct FT_NodeToMove
  {
    int                _nodeToMove;
    std::vector< int > _neighborNodes;

    double             _params[2];   // parameters on shape (U or UV) found by projection
    double            *_nearParams; // _params of a neighbor already projected node

    FT_NodeToMove(): _nearParams(0) {}
  };

  std::vector< std::string >    _groupNames;
  int                           _shapeDim;   // dimension of boundary shapes
  std::vector< FT_NodeToMove >  _nodes;      // ids of nodes to move and their neighbors
  std::vector< FT_Projector >   _projectors; // FT_Projector's initialized with boundary shapes
  std::vector< FT_Projector > * _allProjectors; // FT_Projector's for all shapes of _shapeDim
  MEDCoupling::DataArrayDouble* _nodeCoords;
  bool                          _OK;          // projecting is successful 

  // map of { FT_NodeToMove::_neighborNodes[i] } to { FT_NodeToMove* }
  // this map is used to find neighbor nodes
  typedef NCollection_DataMap< int, std::vector< FT_NodeToMove* > > TNodeIDToLinksMap;
  TNodeIDToLinksMap             _neigborsMap;
  std::vector<int>              _nodesOrder;

};

#endif
