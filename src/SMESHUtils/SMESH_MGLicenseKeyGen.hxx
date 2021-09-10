// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESH_MGLicenseKeyGen.hxx
// Created   : Sat Jul 31 18:41:04 2021
// Author    : Edward AGAPOV (OCC)

#ifndef __SMESHUtils_MGLicenseKeyGen_HXX__
#define __SMESHUtils_MGLicenseKeyGen_HXX__

#include "SMESH_Utils.hxx"

#include <string>

/*!
 * \brief Manage loading libSalomeMeshGemsKeyGenerator.[so|dll] and sing MeshGems CAD or mesh
 */

namespace SMESHUtils_MGLicenseKeyGen
{
  SMESHUtils_EXPORT bool        SignCAD( void* meshgems_cad, std::string& error );

  SMESHUtils_EXPORT bool        SignMesh( void* meshgems_mesh, std::string& error );

  SMESHUtils_EXPORT std::string GetKey(const std::string& gmfFile,
                                       int                nbVertex,
                                       int                nbEdge,
                                       int                nbFace,
                                       int                nbVol,
                                       std::string&       error);

  SMESHUtils_EXPORT bool        CheckKeyGenLibrary( std::string& error );

  SMESHUtils_EXPORT std::string GetLibraryName();
}

#endif
