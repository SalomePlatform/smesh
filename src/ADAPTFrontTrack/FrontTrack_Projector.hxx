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
// File      : FrontTrack_Projector.hxx
// Created   : Wed Apr 26 20:12:13 2017
// Author    : Edward AGAPOV (eap)

#ifndef __FrontTrack_Projector_HXX__
#define __FrontTrack_Projector_HXX__

#include <TopoDS_Shape.hxx>
#include <Bnd_Box.hxx>

struct FT_RealProjector;

/*!
 * \brief Projector of a point to a boundary shape. Wrapper of a real projection algo
 */
class FT_Projector
{
public:

  FT_Projector(const TopoDS_Shape& shape = TopoDS_Shape());
  FT_Projector(const FT_Projector& other);
  ~FT_Projector();

  // initialize with a boundary shape, compute the bounding box
  void setBoundaryShape(const TopoDS_Shape& shape);

  // return the boundary shape
  const TopoDS_Shape& getShape() const { return _shape; }

  // return the bounding box
  const Bnd_Box getBoundingBox() const { return _bndBox; }


  // create a real projector
  void prepareForProjection();

  // return true if a previously found solution can be used to speed up the projection
  bool canUsePrevSolution() const;

  // return true if projection is not needed
  bool isPlanarBoundary() const;


  // switch a mode of usage of prevSolution.
  // If projection fails, to try to project without usage of prevSolution.
  // By default this mode is off
  void tryWithoutPrevSolution( bool toTry ) { _tryWOPrevSolution = toTry; }

  // project a point to the boundary shape
  bool project( const gp_Pnt& point,
                const double  maxDist2,
                gp_Pnt&       projection,
                double*       newSolution,
                const double* prevSolution = 0);

  // project a point to the boundary shape and check if the projection is within the shape boundary
  bool projectAndClassify( const gp_Pnt& point,
                           const double  maxDist2,
                           gp_Pnt&       projection,
                           double*       newSolution,
                           const double* prevSolution = 0);

  // check if a point lies on the boundary shape
  bool isOnShape( const gp_Pnt& point,
                  const double  tol2,
                  double*       newSolution,
                  const double* prevSolution = 0);

private:

  FT_RealProjector* _realProjector;
  Bnd_Box           _bndBox;
  TopoDS_Shape      _shape;
  bool              _tryWOPrevSolution;
};

#endif
