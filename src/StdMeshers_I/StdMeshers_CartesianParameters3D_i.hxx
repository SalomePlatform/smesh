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
//  File   : StdMeshers_CartesianParameters3D_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_CartesianParameters3D_I_HXX_
#define _SMESH_CartesianParameters3D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

class SMESH_Gen;
class StdMeshers_CartesianParameters3D;

// ======================================================
// "CartesianParameters3D" hypothesis
// ======================================================

class STDMESHERS_I_EXPORT StdMeshers_CartesianParameters3D_i:
  public virtual POA_StdMeshers::StdMeshers_CartesianParameters3D,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  StdMeshers_CartesianParameters3D_i( PortableServer::POA_ptr thePOA,
                                      int                     theStudyId,
                                      ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_CartesianParameters3D_i();

  /*!
   * Set size threshold. A polyhedral cell got by cutting an initial
   * hexahedron by geometry boundary is considered small and is removed if
   * it's size is \athreshold times less than the size of the initial hexahedron. 
   * threshold must be > 1.0
   */
  void SetSizeThreshold(CORBA::Double threshold) throw (SALOME::SALOME_Exception);
  CORBA::Double GetSizeThreshold();

  /*!
   * \brief Set node coordinates along an axis (counterd from zero)
   */
  void SetGrid(const SMESH::double_array& coords,
               CORBA::Short               axis) throw (SALOME::SALOME_Exception);
  SMESH::double_array* GetGrid(CORBA::Short axis) throw (SALOME::SALOME_Exception);

  /*!
   * \brief Set grid spacing along an axis
   *  \param spaceFunctions - functions defining spacing value at given point on axis
   *  \param internalPoints - points dividing a grid into parts along each direction
   *  \param axis - index of an axis counterd from zero, i.e. 0==X, 1==Y, 2==Z
   *
   * Parameter t of spaceFunction f(t) is a position [0,1] withing bounding box of
   * the shape to mesh or withing an interval defined by internal points
   */
  void SetGridSpacing(const SMESH::string_array& spaceFunctions,
                      const SMESH::double_array& internalPoints,
                      CORBA::Short               axis) throw (SALOME::SALOME_Exception);

  void GetGridSpacing(SMESH::string_array_out xSpaceFunctions,
                      SMESH::double_array_out xInternalPoints,
                      CORBA::Short            axis) throw (SALOME::SALOME_Exception);
  /*!
   * Set custom direction of axes
   */
  void SetAxesDirs(const SMESH::DirStruct& x,
                   const SMESH::DirStruct& y,
                   const SMESH::DirStruct& z) throw (SALOME::SALOME_Exception);
  void GetAxesDirs(SMESH::DirStruct& x,
                   SMESH::DirStruct& y,
                   SMESH::DirStruct& z);
  /*!
   * Set/unset a fixed point, at which a node will be created provided that grid
   * is defined by spacing in all directions
   */
  void SetFixedPoint(const ::SMESH::PointStruct& p, CORBA::Boolean toUnset);
  CORBA::Boolean GetFixedPoint(::SMESH::PointStruct& p);


  /*!
   * \brief Enables implementation of geometrical edges into the mesh. If this feature
   *        is disabled, sharp edges of the shape are lost ("smoothed") in the mesh if
   *        they don't coincide with the grid lines
   */
  void SetToAddEdges(CORBA::Boolean toAdd);
  CORBA::Boolean GetToAddEdges();

  /*!
   * \brief Return true if the grid is defined by spacing functions and 
   *        not by node coordinates
   */
  CORBA::Boolean IsGridBySpacing(CORBA::Short axis);

  /*!
   * Returns axes at which number of hexahedra is maximal
   */
  void ComputeOptimalAxesDirs(GEOM::GEOM_Object_ptr shape,
                              CORBA::Boolean        isOrthogonal,
                              SMESH::DirStruct&     x,
                              SMESH::DirStruct&     y,
                              SMESH::DirStruct&     z) throw (SALOME::SALOME_Exception);
  /*!
   * \brief Computes node coordinates by spacing functions
   *  \param x0 - lower coordinate
   *  \param x1 - upper coordinate
   *  \param spaceFuns - space functions
   *  \param points - internal points
   *  \param coords - the computed coordinates
   */
  SMESH::double_array* ComputeCoordinates(CORBA::Double              x0,
                                          CORBA::Double              x1,
                                          const SMESH::string_array& spaceFuns,
                                          const SMESH::double_array& points,
                                          const char*                axisName )
    throw (SALOME::SALOME_Exception);

  // Get implementation
  ::StdMeshers_CartesianParameters3D* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif

