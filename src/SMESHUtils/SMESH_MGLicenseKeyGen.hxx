// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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

#define MESHGEMS_215 (2 << 16 | 15 << 8 | 0)

/*!
 * \brief Manage loading libSalomeMeshGemsKeyGenerator.[so|dll] and sing MeshGems CAD or mesh
 */

namespace SMESHUtils_MGLicenseKeyGen
{
  // MeshGems 2.13, 2.14 (for CADSurf)
  SMESHUtils_EXPORT bool        SignCAD( void* meshgems_cad, std::string& error );

  // MeshGems 2.15 (for products launched as library)
  SMESHUtils_EXPORT bool        UnlockProduct( const std::string& product, std::string& error );

  // MeshGems 2.13, 2.14 (for products launched as library)
  SMESHUtils_EXPORT bool        SignMesh( void* meshgems_mesh, const std::string& product, std::string& error );

  // MeshGems 2.13, 2.14 and 2.15 (for products launched as executables)
  SMESHUtils_EXPORT std::string GetKey(const std::string& gmfFile,
                                       int                nbVertex,
                                       int                nbEdge,
                                       int                nbFace,
                                       int                nbVol,
                                       std::string&       error);

  // MeshGems 2.15 (for products launched as library)
  SMESHUtils_EXPORT std::string GetKey( std::string& error );

  SMESHUtils_EXPORT bool        CheckKeyGenLibrary( std::string& error );

  SMESHUtils_EXPORT std::string GetLibraryName();
  SMESHUtils_EXPORT int         GetMGVersionFromFunction(const char* function_name);
  SMESHUtils_EXPORT int         GetMGVersionFromEnv(const char* env_variable);
  SMESHUtils_EXPORT int         GetMGVersionHex(std::string& error);
  SMESHUtils_EXPORT bool        NeedsMGSpatialEnvLicense(std::string& error);
  SMESHUtils_EXPORT bool        SetMGSpatialEnvLicense(std::string& error);
}

#endif
