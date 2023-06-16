// Copyright (C) 2021-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMeshHelper.h"

#include "SALOME_KernelServices.hxx"

#include <cstring>

#include <DriverGMF_Read.hxx>
#include <SMESH_MGLicenseKeyGen.hxx>

std::string GetMGLicenseKeyImpl(const char* gmfFile)
{
  smIdType nbVertex, nbEdge, nbFace, nbVol;
  DriverGMF_Read gmfReader;
  gmfReader.SetFile( gmfFile );
  gmfReader.GetMeshInfo( nbVertex, nbEdge, nbFace, nbVol );

  std::string errorTxt;
  std::string key = SMESHUtils_MGLicenseKeyGen::GetKey( gmfFile,
                                                        FromSmIdType<int>( nbVertex ),
                                                        FromSmIdType<int>( nbEdge ),
                                                        FromSmIdType<int>( nbFace ),
                                                        FromSmIdType<int>( nbVol ),
                                                        errorTxt );
  if ( !errorTxt.empty() )
  {
    std::cerr << "Error: Pb with MeshGems license: " << errorTxt << std::endl;
    key = "<" + errorTxt + ">";
  }
  return key;
}
