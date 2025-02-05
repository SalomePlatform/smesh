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
// File      : DriverStructuredCGNS_Write.hxx
// Created   : Tuesday March 19 2024
// Author    : Cesar Conopoima (cce)

#ifndef __DriverStructuredCGNS_Write_HXX__
#define __DriverStructuredCGNS_Write_HXX__

// occt
#include <TopoDS.hxx>

// smesh
#include "SMESH_DriverCGNS.hxx"
#include "SMESH_RegularGridTemplate.hxx"
#include "Driver_SMESHDS_Mesh.h"

#include <vector>
#include <string>

/*!
 * \brief Driver writinging a mesh into the CGNS file.
 */
class MESHDriverCGNS_EXPORT DriverStructuredCGNS_Write : public Driver_SMESHDS_Mesh
{
public:

  DriverStructuredCGNS_Write();
  ~DriverStructuredCGNS_Write();

  virtual Status Perform();
  
  /*!
  * \brief Search for a SMESHDS_GroupOnGeom associated to the geometry and return his name
  * \param shapeToIndex, the index of the shape as given by shapeToIndex method
  * \return the name of the group if found or an empty name
  */
  std::string GetGroupName( const int shapeToIndex, int dim );

  /*!
  * \brief Fill name of face interfaces associated to mesh groups
  */
  void CheckForGroupNameOnFaceInterfaces( const SMESHUtils::SMESH_RegularGrid* grid, std::vector<std::string>& interfaceNames );

  /*!
  * \brief Fill name of edge interfaces associated to mesh groups
  */
  void CheckForGroupNameOnEdgeInterfaces( const SMESHUtils::SMESH_RegularGrid* grid, std::vector<std::string>& interfaceNames );

private:
  int _fn; //!< file index
};

#endif
