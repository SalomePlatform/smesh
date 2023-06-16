// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : SMESH_SequentialMesh.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//
#ifndef _SMESH_SEQUENTIALMESH_HXX_
#define _SMESH_SEQUENTIALMESH_HXX_

#include "SMESH_Mesh.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_subMesh.hxx"

class SMESH_EXPORT SMESH_SequentialMesh: public SMESH_Mesh
{
 public:
  SMESH_SequentialMesh(int               theLocalId,
                       SMESH_Gen*        theGen,
                       bool              theIsEmbeddedMode,
                       SMESHDS_Document* theDocument);

  virtual ~SMESH_SequentialMesh();

  void Lock() override {};
  void Unlock() override {};

  int GetNbThreads() override {return 0;};
  void SetNbThreads(long nbThreads) {(void) nbThreads;};

  void InitPoolThreads() override {};
  void DeletePoolThreads() override {};
  void wait() override {};

  bool IsParallel() override {return false;};

  bool ComputeSubMeshes (
            SMESH_Gen* gen,
            SMESH_Mesh & aMesh,
            const TopoDS_Shape & aShape,
            const ::MeshDimension       aDim,
            TSetOfInt*                  aShapesId /*=0*/,
            TopTools_IndexedMapOfShape* allowedSubShapes,
            SMESH_subMesh::compute_event &computeEvent,
            const bool includeSelf,
            const bool complexShapeFirst,
            const bool   aShapeOnly) override;

 protected:
  SMESH_SequentialMesh():SMESH_Mesh() {};
  SMESH_SequentialMesh(const SMESH_SequentialMesh& aMesh):SMESH_Mesh(aMesh) {};
};
#endif
