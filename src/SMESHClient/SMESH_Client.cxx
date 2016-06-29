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
//  File   : SMESH_Client.cxx
//  Author : Pavel TELKOV
//  Module : SMESH

#include "SMESH_Client.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_Script.hxx"
#include "SMESH_Mesh.hxx"

#include "SALOME_NamingService.hxx"
#include "SALOME_LifeCycleCORBA.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOME_Component)
#include CORBA_SERVER_HEADER(SALOME_Exception)

#include "Basics_Utils.hxx"
#include "utilities.h"

#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include <stdexcept>

#ifndef EXCEPTION
#define EXCEPTION(TYPE, MSG) {\
  std::ostringstream aStream;\
  aStream<<__FILE__<<"["<<__LINE__<<"]::"<<MSG;\
  throw TYPE(aStream.str());\
}
#endif

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

namespace
{
  using std::runtime_error;

  //=======================================================================
  //function : FindNode
  //=======================================================================
  inline const SMDS_MeshNode* FindNode(const SMDS_Mesh* theMesh, int theId){
    if(const SMDS_MeshNode* anElem = theMesh->FindNode(theId)) return anElem;
    EXCEPTION(runtime_error,"SMDS_Mesh::FindNode - cannot find a SMDS_MeshNode for ID = "<<theId);
  }


  //=======================================================================
  //function : FindElement
  //=======================================================================
  inline const SMDS_MeshElement* FindElement(const SMDS_Mesh* theMesh, int theId){
    if(const SMDS_MeshElement* anElem = theMesh->FindElement(theId)) return anElem;
    EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot find a SMDS_MeshElement for ID = "<<theId);
  }


  //=======================================================================
  //function : AddNodesWithID
  //=======================================================================
  inline void AddNodesWithID(SMDS_Mesh* theMesh,
                             SMESH::log_array_var& theSeq,
                             CORBA::Long theId)
  {
    const SMESH::double_array& aCoords = theSeq[theId].coords;
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(3*aNbElems != (CORBA::Long) aCoords.length())
      EXCEPTION(runtime_error,"AddNodesWithID - 3*aNbElems != aCoords.length()");
    for(CORBA::Long aCoordId = 0; anElemId < aNbElems; anElemId++, aCoordId+=3){
      SMDS_MeshElement* anElem = theMesh->AddNodeWithID(aCoords[aCoordId],
                                                        aCoords[aCoordId+1],
                                                        aCoords[aCoordId+2],
                                                        anIndexes[anElemId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddNodeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : Add0DElementsWithID
  //=======================================================================
  inline void Add0DElementsWithID(SMDS_Mesh* theMesh,
                                  SMESH::log_array_var& theSeq,
                                  CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if (2*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 2*aNbElems != aCoords.length()");
    CORBA::Long anIndexId = 0;
    for (; anElemId < aNbElems; anElemId++, anIndexId+=2)
    {
      SMDS_MeshElement* anElem = theMesh->Add0DElementWithID(anIndexes[anIndexId+1],
                                                             anIndexes[anIndexId]);
      if (!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot Add0DElementWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddBallsWithID
  //=======================================================================
  inline void AddBallsWithID(SMDS_Mesh*            theMesh,
                             SMESH::log_array_var& theSeq,
                             CORBA::Long           theId)
  {
    const SMESH::double_array& aDiameter = theSeq[theId].coords;
    const SMESH::long_array& anIndexes   = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems   = theSeq[theId].number;
    if (2*aNbElems != (CORBA::Long) anIndexes.length() )
      EXCEPTION(runtime_error,"AddEdgeWithID - 2*aNbElems != anIndexes.length()");
    if (aNbElems != (CORBA::Long) aDiameter.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - aNbElems != aDiameter.length()");
    CORBA::Long anIndexId = 0;
    for (; anElemId < aNbElems; anElemId++, anIndexId+=2)
    {
      SMDS_MeshElement* anElem = theMesh->AddBallWithID(anIndexes[anIndexId+1],
                                                        aDiameter[anElemId],
                                                        anIndexes[anIndexId]);
      if (!anElem)
        EXCEPTION(runtime_error,"cannot SMDS_Mesh::AddBallsWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddEdgesWithID
  //=======================================================================
  inline void AddEdgesWithID(SMDS_Mesh* theMesh,
                             SMESH::log_array_var& theSeq,
                             CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(3*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddEdgeWithID - 3*aNbElems != aCoords.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=3){
      SMDS_MeshElement* anElem = theMesh->AddEdgeWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddEdgeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddTriasWithID
  //=======================================================================
  inline void AddTriasWithID(SMDS_Mesh* theMesh,
                             SMESH::log_array_var& theSeq,
                             CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(4*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddTriasWithID - 4*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=4){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadsWithID
  //=======================================================================
  inline void AddQuadsWithID(SMDS_Mesh* theMesh,
                             SMESH::log_array_var theSeq,
                             CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(5*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadsWithID - 4*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=5){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId+4],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddPolygonsWithID
  //=======================================================================
  inline void AddPolygonsWithID(SMDS_Mesh* theMesh,
                                SMESH::log_array_var& theSeq,
                                CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anIndexId = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++) {
      int aFaceId = anIndexes[anIndexId++];

      int aNbNodes = anIndexes[anIndexId++];
      std::vector<int> nodes_ids (aNbNodes);
      for (int i = 0; i < aNbNodes; i++) {
        nodes_ids[i] = anIndexes[anIndexId++];
      }

      SMDS_MeshElement* anElem = theMesh->AddPolygonalFaceWithID(nodes_ids, aFaceId);
      if (!anElem)
        EXCEPTION(runtime_error, "SMDS_Mesh::FindElement - cannot AddPolygonalFaceWithID for ID = "
                  << anElemId);
    }
  }


  //=======================================================================
  //function : AddQaudPolygonsWithID
  //=======================================================================
  inline void AddQuadPolygonsWithID(SMDS_Mesh* theMesh,
                                    SMESH::log_array_var& theSeq,
                                    CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anIndexId = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++) {
      int aFaceId = anIndexes[anIndexId++];

      int aNbNodes = anIndexes[anIndexId++];
      std::vector<int> nodes_ids (aNbNodes);
      for (int i = 0; i < aNbNodes; i++) {
        nodes_ids[i] = anIndexes[anIndexId++];
      }

      SMDS_MeshElement* anElem = theMesh->AddQuadPolygonalFaceWithID(nodes_ids, aFaceId);
      if (!anElem)
        EXCEPTION(runtime_error, "SMDS_Mesh::FindElement - cannot AddQaudPolygonalFaceWithID for ID = "
                  << anElemId);
    }
  }


  //=======================================================================
  //function : AddTetrasWithID
  //=======================================================================
  inline void AddTetrasWithID(SMDS_Mesh* theMesh,
                              SMESH::log_array_var& theSeq,
                              CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(5*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddTetrasWithID - 5*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=5){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddPiramidsWithID
  //=======================================================================
  inline void AddPiramidsWithID(SMDS_Mesh* theMesh,
                                SMESH::log_array_var& theSeq,
                                CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(6*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddPiramidsWithID - 6*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=6){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddPrismsWithID
  //=======================================================================
  inline void AddPrismsWithID(SMDS_Mesh* theMesh,
                              SMESH::log_array_var& theSeq,
                              CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(7*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddPrismsWithID - 7*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=7){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddHexasWithID
  //=======================================================================
  inline void AddHexasWithID(SMDS_Mesh* theMesh,
                             SMESH::log_array_var& theSeq,
                             CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(9*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddHexasWithID - 9*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=9){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }

  //=======================================================================
  //function : AddHexPrismWithID
  //=======================================================================
  inline void AddHexPrismWithID(SMDS_Mesh* theMesh,
                                SMESH::log_array_var& theSeq,
                                CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(13*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddHexPrismWithID - 13*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=13){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId+11],
                                                          anIndexes[anIndexId+12],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"AddHexPrismWithID - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddPolyhedronsWithID
  //=======================================================================
  inline void AddPolyhedronsWithID (SMDS_Mesh* theMesh,
                                    SMESH::log_array_var& theSeq,
                                    CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anIndexId = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++) {
      int aFaceId = anIndexes[anIndexId++];

      int aNbNodes = anIndexes[anIndexId++];
      std::vector<int> nodes_ids (aNbNodes);
      for (int i = 0; i < aNbNodes; i++) {
        nodes_ids[i] = anIndexes[anIndexId++];
      }

      int aNbFaces = anIndexes[anIndexId++];
      std::vector<int> quantities (aNbFaces);
      for (int i = 0; i < aNbFaces; i++) {
        quantities[i] = anIndexes[anIndexId++];
      }

      SMDS_MeshElement* anElem =
        theMesh->AddPolyhedralVolumeWithID(nodes_ids, quantities, aFaceId);
      if (!anElem)
        EXCEPTION(runtime_error, "SMDS_Mesh::FindElement - cannot AddPolyhedralVolumeWithID for ID = "
                  << anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadEdgesWithID
  //=======================================================================
  inline void AddQuadEdgesWithID(SMDS_Mesh* theMesh,
                                 SMESH::log_array_var& theSeq,
                                 CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(4*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadEdgeWithID - 4*aNbElems != aCoords.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=4){
      SMDS_MeshElement* anElem = theMesh->AddEdgeWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddEdgeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadTriasWithID
  //=======================================================================
  inline void AddQuadTriasWithID(SMDS_Mesh* theMesh,
                                 SMESH::log_array_var& theSeq,
                                 CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(7*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadTriasWithID - 7*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=7){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId+4],
                                                        anIndexes[anIndexId+5],
                                                        anIndexes[anIndexId+6],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadQuadsWithID
  //=======================================================================
  inline void AddQuadQuadsWithID(SMDS_Mesh* theMesh,
                                 SMESH::log_array_var theSeq,
                                 CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(9*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadQuadsWithID - 9*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=9){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId+4],
                                                        anIndexes[anIndexId+5],
                                                        anIndexes[anIndexId+6],
                                                        anIndexes[anIndexId+7],
                                                        anIndexes[anIndexId+8],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }

  //=======================================================================
  //function : AddBiQuadQuadsWithID
  //=======================================================================
  inline void AddBiQuadQuadsWithID(SMDS_Mesh* theMesh,
                                   SMESH::log_array_var theSeq,
                                   CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(10*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddBiQuadQuadsWithID - 10*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=10){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId+4],
                                                        anIndexes[anIndexId+5],
                                                        anIndexes[anIndexId+6],
                                                        anIndexes[anIndexId+7],
                                                        anIndexes[anIndexId+8],
                                                        anIndexes[anIndexId+9],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"AddBiQuadQuadsWithID() - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }

  //=======================================================================
  //function : AddBiQuadTriasWithID
  //=======================================================================
  inline void AddBiQuadTriasWithID(SMDS_Mesh* theMesh,
                                   SMESH::log_array_var theSeq,
                                   CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(8*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddBiQuadTriasWithID - 8*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=8){
      SMDS_MeshElement* anElem = theMesh->AddFaceWithID(anIndexes[anIndexId+1],
                                                        anIndexes[anIndexId+2],
                                                        anIndexes[anIndexId+3],
                                                        anIndexes[anIndexId+4],
                                                        anIndexes[anIndexId+5],
                                                        anIndexes[anIndexId+6],
                                                        anIndexes[anIndexId+7],
                                                        anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"AddBiQuadTriasWithID() - cannot AddFaceWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadTetrasWithID
  //=======================================================================
  inline void AddQuadTetrasWithID(SMDS_Mesh* theMesh,
                                  SMESH::log_array_var& theSeq,
                                  CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(11*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadTetrasWithID - 11*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=11){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadPiramidsWithID
  //=======================================================================
  inline void AddQuadPiramidsWithID(SMDS_Mesh* theMesh,
                                    SMESH::log_array_var& theSeq,
                                    CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(14*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadPiramidsWithID - 14*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=14){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId+11],
                                                          anIndexes[anIndexId+12],
                                                          anIndexes[anIndexId+13],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadPentasWithID
  //=======================================================================
  inline void AddQuadPentasWithID(SMDS_Mesh* theMesh,
                                  SMESH::log_array_var& theSeq,
                                  CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(16*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadPentasWithID - 16*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=16){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId+11],
                                                          anIndexes[anIndexId+12],
                                                          anIndexes[anIndexId+13],
                                                          anIndexes[anIndexId+14],
                                                          anIndexes[anIndexId+15],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : AddQuadHexasWithID
  //=======================================================================
  inline void AddQuadHexasWithID(SMDS_Mesh* theMesh,
                                 SMESH::log_array_var& theSeq,
                                 CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(21*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddQuadHexasWithID - 21*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=21){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId+11],
                                                          anIndexes[anIndexId+12],
                                                          anIndexes[anIndexId+13],
                                                          anIndexes[anIndexId+14],
                                                          anIndexes[anIndexId+15],
                                                          anIndexes[anIndexId+16],
                                                          anIndexes[anIndexId+17],
                                                          anIndexes[anIndexId+18],
                                                          anIndexes[anIndexId+19],
                                                          anIndexes[anIndexId+20],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"SMDS_Mesh::FindElement - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }

  //=======================================================================
  //function : AddTriQuadHexasWithID
  //=======================================================================
  inline void AddTriQuadHexasWithID(SMDS_Mesh* theMesh,
                                    SMESH::log_array_var& theSeq,
                                    CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long anElemId = 0, aNbElems = theSeq[theId].number;
    if(28*aNbElems != (CORBA::Long) anIndexes.length())
      EXCEPTION(runtime_error,"AddTriQuadHexasWithID - 28*aNbElems != anIndexes.length()");
    for(CORBA::Long anIndexId = 0; anElemId < aNbElems; anElemId++, anIndexId+=28){
      SMDS_MeshElement* anElem = theMesh->AddVolumeWithID(anIndexes[anIndexId+1],
                                                          anIndexes[anIndexId+2],
                                                          anIndexes[anIndexId+3],
                                                          anIndexes[anIndexId+4],
                                                          anIndexes[anIndexId+5],
                                                          anIndexes[anIndexId+6],
                                                          anIndexes[anIndexId+7],
                                                          anIndexes[anIndexId+8],
                                                          anIndexes[anIndexId+9],
                                                          anIndexes[anIndexId+10],
                                                          anIndexes[anIndexId+11],
                                                          anIndexes[anIndexId+12],
                                                          anIndexes[anIndexId+13],
                                                          anIndexes[anIndexId+14],
                                                          anIndexes[anIndexId+15],
                                                          anIndexes[anIndexId+16],
                                                          anIndexes[anIndexId+17],
                                                          anIndexes[anIndexId+18],
                                                          anIndexes[anIndexId+19],
                                                          anIndexes[anIndexId+20],
                                                          anIndexes[anIndexId+21],
                                                          anIndexes[anIndexId+22],
                                                          anIndexes[anIndexId+23],
                                                          anIndexes[anIndexId+24],
                                                          anIndexes[anIndexId+25],
                                                          anIndexes[anIndexId+26],
                                                          anIndexes[anIndexId+27],
                                                          anIndexes[anIndexId]);
      if(!anElem)
        EXCEPTION(runtime_error,"AddTriQuadHexasWithID() - cannot AddVolumeWithID for ID = "<<anElemId);
    }
  }


  //=======================================================================
  //function : ChangePolyhedronNodes
  //=======================================================================
  inline void ChangePolyhedronNodes (SMDS_Mesh* theMesh,
                                     SMESH::log_array_var& theSeq,
                                     CORBA::Long theId)
  {
    const SMESH::long_array& anIndexes = theSeq[theId].indexes;
    CORBA::Long iind = 0, aNbElems = theSeq[theId].number;

    for (CORBA::Long anElemId = 0; anElemId < aNbElems; anElemId++)
    {
      // find element
      const SMDS_MeshElement* elem = FindElement(theMesh, anIndexes[iind++]);
      // nb nodes
      int nbNodes = anIndexes[iind++];
      // nodes
      std::vector<const SMDS_MeshNode*> aNodes (nbNodes);
      for (int iNode = 0; iNode < nbNodes; iNode++) {
        aNodes[iNode] = FindNode(theMesh, anIndexes[iind++]);
      }
      // nb faces
      int nbFaces = anIndexes[iind++];
      // quantities
      std::vector<int> quantities (nbFaces);
      for (int iFace = 0; iFace < nbFaces; iFace++) {
        quantities[iFace] = anIndexes[iind++];
      }
      // change
      theMesh->ChangePolyhedronNodes(elem, aNodes, quantities);
    }
  }
}

//=======================================================================
SMESH::SMESH_Gen_var
SMESH_Client::GetSMESHGen(CORBA::ORB_ptr theORB,
                          CORBA::Boolean& theIsEmbeddedMode)
{
  static SMESH::SMESH_Gen_var aMeshGen;

  if(CORBA::is_nil(aMeshGen.in())){
#ifdef WIN32
    long aClientPID = (long)_getpid();
#else
    long aClientPID =  (long)getpid();
#endif

    SALOME_NamingService aNamingService(theORB);
    SALOME_LifeCycleCORBA aLifeCycleCORBA(&aNamingService);
    Engines::EngineComponent_var aComponent = aLifeCycleCORBA.FindOrLoad_Component("FactoryServer","SMESH");
    aMeshGen = SMESH::SMESH_Gen::_narrow(aComponent);

    std::string aClientHostName = Kernel_Utils::GetHostname();
    Engines::Container_var aServerContainer = aMeshGen->GetContainerRef();
    CORBA::String_var aServerHostName = aServerContainer->getHostName();
    CORBA::Long aServerPID = aServerContainer->getPID();
    aMeshGen->SetEmbeddedMode((aClientPID == aServerPID) && (aClientHostName == aServerHostName.in()));
  }
  theIsEmbeddedMode = aMeshGen->IsEmbeddedMode();

  return aMeshGen;
}


//=======================================================================
// function : Create()
// purpose  :
//=======================================================================
SMESH_Client::SMESH_Client(CORBA::ORB_ptr theORB,
                           SMESH::SMESH_Mesh_ptr theMesh):
  myMeshServer(SMESH::SMESH_Mesh::_duplicate(theMesh)),
  mySMESHDSMesh(NULL),
  mySMDSMesh(NULL)
{
  if ( MYDEBUG ) MESSAGE("SMESH_Client::SMESH_Client");
  myMeshServer->Register();

  CORBA::Boolean anIsEmbeddedMode;
  GetSMESHGen(theORB,anIsEmbeddedMode);
  if(anIsEmbeddedMode){
    if ( MYDEBUG ) MESSAGE("Info: The same process, update mesh by pointer ");
    // just set client mesh pointer to server mesh pointer
    //SMESH_Mesh* aMesh = reinterpret_cast<SMESH_Mesh*>(theMesh->GetMeshPtr());
    CORBA::LongLong pointeur = theMesh->GetMeshPtr();
    if( MYDEBUG ) MESSAGE("SMESH_Client::SMESH_Client pointeur "<<pointeur);
    SMESH_Mesh* aMesh = reinterpret_cast<SMESH_Mesh*> (pointeur);
    if ( MYDEBUG ) MESSAGE("SMESH_Client::SMESH_Client aMesh "<<aMesh);
    //if(aMesh->GetMeshDS()->IsEmbeddedMode()){
    if(anIsEmbeddedMode){
      mySMESHDSMesh = aMesh->GetMeshDS();
      mySMDSMesh = mySMESHDSMesh;
    }
  }
  if(!mySMDSMesh)
    mySMDSMesh = new SMDS_Mesh();
}


//=================================================================================
// function : ~SMESH_Client
// purpose  : Destructor
//=================================================================================
SMESH_Client::~SMESH_Client()
{
  myMeshServer->UnRegister();
  if(!mySMESHDSMesh)
    delete mySMDSMesh;
}


//=================================================================================
SMDS_Mesh*
SMESH_Client::GetMesh() const
{
  return mySMDSMesh;
}


//=================================================================================
SMDS_Mesh*
SMESH_Client::operator->() const
{
  return GetMesh();
}


//=================================================================================
SMESH::SMESH_Mesh_ptr
SMESH_Client::GetMeshServer()
{
  return myMeshServer.in();
}


//=================================================================================
// function : SMESH_Client
// purpose  : Update mesh
//=================================================================================
bool
SMESH_Client::Update(bool theIsClear)
{
  bool anIsModified = true;
  if(mySMESHDSMesh)
  {
    if ( MYDEBUG ) MESSAGE("Update mySMESHDSMesh");
    SMESHDS_Script* aScript = mySMESHDSMesh->GetScript();
    anIsModified = aScript->IsModified();
    aScript->SetModified(false);
  }
  else
  {
    if ( MYDEBUG ) MESSAGE("Update CORBA");
    SMESH::log_array_var aSeq = myMeshServer->GetLog( theIsClear );
    CORBA::Long aLength = aSeq->length();
    anIsModified = aLength > 0;
    if ( MYDEBUG ) MESSAGE( "Update: length of the script is "<<aLength );

    if ( !anIsModified )
      return false;

    // update client mesh structure by logged changes commands
    try
    {
      for ( CORBA::Long anId = 0; anId < aLength; anId++)
      {
        const SMESH::double_array& aCoords = aSeq[anId].coords;
        const SMESH::long_array& anIndexes = aSeq[anId].indexes;
        CORBA::Long anElemId = 0, aNbElems = aSeq[anId].number;
        CORBA::Long aCommand = aSeq[anId].commandType;

        switch(aCommand)
        {
        case SMESH::ADD_NODE             : AddNodesWithID      ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_EDGE             : AddEdgesWithID      ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_TRIANGLE         : AddTriasWithID      ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADRANGLE       : AddQuadsWithID      ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_POLYGON          : AddPolygonsWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_TETRAHEDRON      : AddTetrasWithID     ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_PYRAMID          : AddPiramidsWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_PRISM            : AddPrismsWithID     ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_HEXAHEDRON       : AddHexasWithID      ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_HEXAGONAL_PRISM  : AddHexPrismWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_POLYHEDRON       : AddPolyhedronsWithID( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_ELEM0D           : Add0DElementsWithID ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_BALL             : AddBallsWithID      ( mySMDSMesh, aSeq, anId ); break;

        case SMESH::ADD_QUADEDGE         : AddQuadEdgesWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADTRIANGLE     : AddQuadTriasWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADQUADRANGLE   : AddQuadQuadsWithID   ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADPOLYGON      : AddQuadPolygonsWithID( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADTETRAHEDRON  : AddQuadTetrasWithID  ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADPYRAMID      : AddQuadPiramidsWithID( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADPENTAHEDRON  : AddQuadPentasWithID  ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_QUADHEXAHEDRON   : AddQuadHexasWithID   ( mySMDSMesh, aSeq, anId ); break;

        case SMESH::ADD_BIQUAD_QUADRANGLE: AddBiQuadQuadsWithID ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_BIQUAD_TRIANGLE  : AddBiQuadTriasWithID ( mySMDSMesh, aSeq, anId ); break;
        case SMESH::ADD_TRIQUAD_HEXA     : AddTriQuadHexasWithID( mySMDSMesh, aSeq, anId ); break;

        case SMESH::CLEAR_MESH:
          mySMDSMesh->Clear();
          break;

        case SMESH::REMOVE_NODE:
          for( ; anElemId < aNbElems; anElemId++ )
            mySMDSMesh->RemoveNode( FindNode( mySMDSMesh, anIndexes[anElemId] ) );
        break;

        case SMESH::REMOVE_ELEMENT:
          for( ; anElemId < aNbElems; anElemId++ )
            mySMDSMesh->RemoveElement( FindElement( mySMDSMesh, anIndexes[anElemId] ) );
        break;

        case SMESH::MOVE_NODE:
          for(CORBA::Long aCoordId=0; anElemId < aNbElems; anElemId++, aCoordId+=3)
          {
            SMDS_MeshNode* node =
              const_cast<SMDS_MeshNode*>( FindNode( mySMDSMesh, anIndexes[anElemId] ));
            node->setXYZ( aCoords[aCoordId], aCoords[aCoordId+1], aCoords[aCoordId+2] );
          }
        break;

        case SMESH::CHANGE_ELEMENT_NODES:
          for ( CORBA::Long i = 0; anElemId < aNbElems; anElemId++ )
          {
            // find element
            const SMDS_MeshElement* elem = FindElement( mySMDSMesh, anIndexes[i++] );
            // nb nodes
            int nbNodes = anIndexes[i++];
            // nodes
            //ASSERT( nbNodes < 9 );
            std::vector<const SMDS_MeshNode*> aNodes( nbNodes );
            for ( int iNode = 0; iNode < nbNodes; iNode++ )
              aNodes[ iNode ] = FindNode( mySMDSMesh, anIndexes[i++] );
            // change
            mySMDSMesh->ChangeElementNodes( elem, &aNodes[0], nbNodes );
          }
          break;

        case SMESH::CHANGE_POLYHEDRON_NODES:
          ChangePolyhedronNodes(mySMDSMesh, aSeq, anId);
          break;
        case SMESH::RENUMBER:
          for(CORBA::Long i=0; anElemId < aNbElems; anElemId++, i+=3)
          {
            mySMDSMesh->Renumber( anIndexes[i], anIndexes[i+1], anIndexes[i+2] );
          }
          break;

        default:;
        }
      }
    }
    catch ( SALOME::SALOME_Exception& exc )
    {
      INFOS("Following exception was cought:\n\t"<<exc.details.text);
    }
    catch( const std::exception& exc)
    {
      INFOS("Following exception was cought:\n\t"<<exc.what());
    }
    catch(...)
    {
      INFOS("Unknown exception was cought !!!");
    }

    if ( MYDEBUG && mySMDSMesh )
    {
      MESSAGE("Update - mySMDSMesh->NbNodes() = "<<mySMDSMesh->NbNodes());
      MESSAGE("Update - mySMDSMesh->Nb0DElements() = "<<mySMDSMesh->Nb0DElements());
      MESSAGE("Update - mySMDSMesh->NbEdges() = "<<mySMDSMesh->NbEdges());
      MESSAGE("Update - mySMDSMesh->NbFaces() = "<<mySMDSMesh->NbFaces());
      MESSAGE("Update - mySMDSMesh->NbVolumes() = "<<mySMDSMesh->NbVolumes());
    }
  } // end of update mesh by log script

  return anIsModified;
}
