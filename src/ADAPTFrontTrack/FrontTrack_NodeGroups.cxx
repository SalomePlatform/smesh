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
// File      : FrontTrack_NodeGroups.cxx
// Created   : Tue Apr 25 19:17:47 2017
// Author    : Edward AGAPOV (eap)

#include "FrontTrack_NodeGroups.hxx"
#include "FrontTrack_Projector.hxx"
#include "FrontTrack_Utils.hxx"

#include <MEDCouplingMemArray.hxx>
#include <XAO_BrepGeometry.hxx>
#include <XAO_Xao.hxx>

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>

namespace
{
  //================================================================================
  /*!
   * \brief Initialize FT_Projector's with all sub-shapes of given type
   *  \param [in] theMainShape - the shape to explore
   *  \param [in] theSubType - the type of sub-shapes
   *  \param [out] theProjectors - the projectors
   */
  //================================================================================

  void getProjectors( const TopoDS_Shape&           theMainShape,
                      const TopAbs_ShapeEnum        theSubType,
                      std::vector< FT_Projector > & theProjectors )
  {
    TopTools_IndexedMapOfShape subShapes;
    TopExp::MapShapes( theMainShape, theSubType, subShapes );
#ifdef _DEBUG_
    std::cout << ". Nombre de subShapes : " << subShapes.Size() << std::endl;
#endif

    theProjectors.resize( subShapes.Size() );
    for ( int i = 1; i <= subShapes.Size(); ++i )
      theProjectors[ i-1 ].setBoundaryShape( subShapes( i ));
  }
}

//================================================================================
/*!
 * \brief Load node groups from files
 *  \param [in] theNodeFiles - an array of names of files describing groups of nodes that
 *         will be moved onto geometry
 *  \param [in] theXaoGeom - the whole geometry to project on
 *  \param [inout] theNodeCoords - array of node coordinates
 */
//================================================================================

void FT_NodeGroups::read( const std::vector< std::string >& theNodeFiles,
                          const XAO::Xao*                   theXao,
                          MEDCoupling::DataArrayDouble*     theNodeCoords )
{
  // get projectors for all boundary sub-shapes;
  // index of a projector in the vector corresponds to a XAO index of a sub-shape
  XAO::BrepGeometry* xaoGeom = dynamic_cast<XAO::BrepGeometry*>( theXao->getGeometry() );
  getProjectors( xaoGeom->getTopoDS_Shape(), TopAbs_EDGE, _projectors[0] );
  getProjectors( xaoGeom->getTopoDS_Shape(), TopAbs_FACE, _projectors[1] );

  _nodesOnGeom.resize( theNodeFiles.size() );

  // read node IDs and look for projectors to boundary sub-shapes by group name
  FT_Utils::XaoGroups xaoGroups( theXao );
  for ( size_t i = 0; i < theNodeFiles.size(); ++i )
  {
    _nodesOnGeom[i].read( theNodeFiles[i], xaoGroups, theNodeCoords, _projectors );
  }
}

//================================================================================
/*!
 * \brief Project and move nodes of a given group of nodes
 */
//================================================================================

void FT_NodeGroups::projectAndMove( const int groupIndex )
{
  _nodesOnGeom[ groupIndex ].projectAndMove();
}

//================================================================================
/*!
 * \brief Return true if all nodes were successfully relocated
 */
//================================================================================

bool FT_NodeGroups::isOK() const
{
  for ( size_t i = 0; i < _nodesOnGeom.size(); ++i )
    if ( ! _nodesOnGeom[ i ].isOK() )
      return false;

  return true;
}

//================================================================================
/*!
 * \brief Print some statistics on node groups
 */
//================================================================================

void FT_NodeGroups::dumpStat() const
{
  for ( size_t i = 0; i < _nodesOnGeom.size(); ++i )
  {
    std::cout << _nodesOnGeom[i].getShapeDim() << "D "
              << _nodesOnGeom[i].nbNodes() << " nodes" << std::endl;
  }
}
