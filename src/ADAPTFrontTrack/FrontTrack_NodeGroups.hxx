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
// File      : FrontTrack_NodeGroups.hxx
// Created   : Tue Apr 25 19:02:49 2017
// Author    : Edward AGAPOV (eap)

#ifndef __FrontTrack_NodeGroups_HXX__
#define __FrontTrack_NodeGroups_HXX__

#include "FrontTrack_NodesOnGeom.hxx"
#include "FrontTrack_Projector.hxx"

#include <vector>
#include <string>

namespace MEDCoupling {
  class DataArrayDouble;
}
namespace XAO {
  class Xao;
}

/*!
 * \brief Container of node groups.
 */
class FT_NodeGroups
{
public:

  // Load node groups from files
  void read( const std::vector< std::string >& nodeFiles,
             const XAO::Xao*                   xaoGeom,
             MEDCoupling::DataArrayDouble*     nodeCoords );

  // return number of groups of nodes to move
  int nbOfGroups() const { return _nodesOnGeom.size(); }

  // Move nodes of a group in parallel mode
  void operator() ( const int groupIndex ) const
  {
    const_cast< FT_NodeGroups* >( this )->projectAndMove( groupIndex );
  }

  // Project and move nodes of a given group of nodes
  void projectAndMove( const int groupIndex );

  // return true if all nodes were successfully relocated
  bool isOK() const;

  // print some statistics on node groups
  void dumpStat() const;

private:

  std::vector< FT_NodesOnGeom > _nodesOnGeom;
  std::vector< FT_Projector >   _projectors[2]; // curves and surfaces separately

};

#endif
