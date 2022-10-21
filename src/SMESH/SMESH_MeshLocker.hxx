// Copyright (C) 2007-2022  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : SMESH_Mesh.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#ifndef _SMESH_MESHLOCKER_HXX_
#define _SMESH_MESHLOCKER_HXX_
#include "SMESH_SMESH.hxx"

class SMESH_Mesh;

class SMESH_EXPORT SMESH_MeshLocker{
public:
  SMESH_MeshLocker(SMESH_Mesh * aMesh);
  ~SMESH_MeshLocker();

protected:
  SMESH_MeshLocker();

private:
  SMESH_Mesh * _myMesh=nullptr;
};

#endif
