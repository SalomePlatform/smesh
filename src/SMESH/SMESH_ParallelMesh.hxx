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

//  File   : SMESH_ParallelMesh.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//
#ifndef _SMESH_PARALLELMESH_HXX_
#define _SMESH_PARALLELMESH_HXX_

#include "SMESH_Mesh.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_subMesh.hxx"

class SMESH_EXPORT SMESH_ParallelMesh: public SMESH_Mesh
{
 public:
  SMESH_ParallelMesh(int               theLocalId,
                       SMESH_Gen*        theGen,
                       bool              theIsEmbeddedMode,
                       SMESHDS_Document* theDocument);

  virtual ~SMESH_ParallelMesh();

  void Lock() override {_my_lock.lock();};
  void Unlock() override {_my_lock.unlock();};

  int GetNbThreads() override{return _NbThreads;};
  void SetNbThreads(long nbThreads) override{_NbThreads=nbThreads;};

  void InitPoolThreads() override {_pool = new boost::asio::thread_pool(_NbThreads);};
  void DeletePoolThreads() override {delete _pool;};

  void wait() override {_pool->join(); DeletePoolThreads(); InitPoolThreads(); };

  bool IsParallel() override {return _NbThreads > 0;};

  void CreateTmpFolder();
  void DeleteTmpFolder();

  boost::filesystem::path GetTmpFolder() override {return tmp_folder;};
  boost::asio::thread_pool* GetPool() override {return _pool;};

  bool ComputeSubMeshes(
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
  SMESH_ParallelMesh():SMESH_Mesh() {};
  SMESH_ParallelMesh(const SMESH_ParallelMesh& aMesh):SMESH_Mesh(aMesh) {};
 private:
  boost::filesystem::path tmp_folder;
  boost::asio::thread_pool *     _pool = nullptr; //thread pool for computation
};
#endif
