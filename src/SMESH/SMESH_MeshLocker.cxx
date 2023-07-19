// Copyright (C) 2007-2023  CEA, EDF, OPEN CASCADE
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

//  File   : SMESH_MeshLocker.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//

#include "SMESH_MeshLocker.hxx"

#include "SMESH_Mesh.hxx"

/*
 * When instanced will run the command Lock from a SMESH_Mesh
 */
SMESH_MeshLocker::SMESH_MeshLocker(SMESH_Mesh * aMesh) : _myMesh(aMesh)
{
  _myMesh->Lock();
}

/*
 * When freed will run the command Unlock from the SMESH_Mesh associated
 */
SMESH_MeshLocker::~SMESH_MeshLocker()
{
  _myMesh->Unlock();
}
