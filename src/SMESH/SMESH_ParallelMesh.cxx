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

//  File   : SMESH_ParallelMesh.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH
//
#include "SMESH_ParallelMesh.hxx"

#include "SMESH_Gen.hxx"

#ifdef WIN32
  #include <windows.h>
#endif

#ifndef WIN32
#include <boost/filesystem.hpp>
namespace fs=boost::filesystem;
#endif

#ifndef WIN32
#include <boost/asio.hpp>
#endif

#include <utilities.h>

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif

SMESH_ParallelMesh::SMESH_ParallelMesh(int               theLocalId,
                       SMESH_Gen*        theGen,
                       bool              theIsEmbeddedMode,
                       SMESHDS_Document* theDocument) :SMESH_Mesh(theLocalId,
                                                                  theGen,
                                                                  theIsEmbeddedMode,
                                                                  theDocument)
{
  MESSAGE("SMESH_ParallelMesh::SMESH_ParallelMesh(int localId)");
  CreateTmpFolder();
};

SMESH_ParallelMesh::~SMESH_ParallelMesh()
{
  DeletePoolThreads();
  if(!MYDEBUG)
    DeleteTmpFolder();
};



//=============================================================================
/*!
 * \brief Build folder for parallel computation
 */
//=============================================================================
void SMESH_ParallelMesh::CreateTmpFolder()
{
#ifndef WIN32
  // Temporary folder that will be used by parallel computation
  tmp_folder = fs::temp_directory_path()/fs::unique_path(fs::path("SMESH_%%%%-%%%%"));
  fs::create_directories(tmp_folder);
#endif
}
//
//=============================================================================
/*!
 * \brief Delete temporary folder used for parallel computation
 */
//=============================================================================
void SMESH_ParallelMesh::DeleteTmpFolder()
{
#ifndef WIN32
    fs::remove_all(tmp_folder);
#endif
}

//=============================================================================
/*!
 * \brief Get the number of Threads to be used for the pool of Threads
 */
//=============================================================================
int SMESH_ParallelMesh::GetPoolNbThreads()
{
  int nbThreads = -1;

  if(_method == ParallelismMethod::MultiThread){
    nbThreads = _NbThreads;
  }else if( _method == ParallelismMethod::MultiNode){
    //TODO: Check of that is the right way
    nbThreads = std::max(_nbProc, _nbNode*_nbProcPerNode);
  } else {
    throw SALOME_Exception("Unknown method "+std::to_string(_method));
  }

  return nbThreads;
}

//=============================================================================
/*!
 * \brief Set Number of thread for multithread run
 */
//=============================================================================
void SMESH_ParallelMesh::SetNbThreads(long nbThreads)
{
  if(nbThreads < 1)
    throw SALOME_Exception("Number of threads should be higher than 1");
  _NbThreads=nbThreads;
};

bool SMESH_ParallelMesh::ComputeSubMeshes(
          SMESH_Gen* gen,
          SMESH_Mesh & aMesh,
          const TopoDS_Shape & aShape,
          const ::MeshDimension       aDim,
          TSetOfInt*                  aShapesId /*=0*/,
          TopTools_IndexedMapOfShape* allowedSubShapes,
          SMESH_subMesh::compute_event &computeEvent,
          const bool includeSelf,
          const bool complexShapeFirst,
          const bool   aShapeOnly)
{
  InitPoolThreads();
  return gen->parallelComputeSubMeshes(
            aMesh, aShape, aDim,
            aShapesId, allowedSubShapes,
            computeEvent,
            includeSelf,
            complexShapeFirst,
            aShapeOnly);
}
