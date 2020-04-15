// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include <algorithm>

#include "DriverUNV_W_SMDS_Mesh.h"

#include "SMDS_Mesh.hxx"
#include "SMESHDS_GroupBase.hxx"

#include "utilities.h"

#include "UNV164_Structure.hxx"
#include "UNV2411_Structure.hxx"
#include "UNV2412_Structure.hxx"
#include "UNV2417_Structure.hxx"
#include "UNV2420_Structure.hxx"
#include "UNV_Utilities.hxx"

#include <Basics_Utils.hxx>

using namespace std;
using namespace UNV;

Driver_Mesh::Status DriverUNV_W_SMDS_Mesh::Perform()
{
  Kernel_Utils::Localizer loc;
  Status aResult = DRS_OK;
#if defined(WIN32) && defined(UNICODE)
  std::wstring aFile = Kernel_Utils::utf8_decode_s(myFile);
  std::ofstream out_stream(aFile.c_str());
#else
  std::ofstream out_stream(myFile.c_str());
#endif
  try{

    UNV164::Write( out_stream ); // unit system
    UNV2420::Write( out_stream, myMeshName ); // Coordinate system

    std::vector< size_t > nodeLabelByID;
    if ( myMesh->HasNumerationHoles() )
      nodeLabelByID.resize( myMesh->MaxNodeID() + 1 );

    {
      using namespace UNV2411;
      TDataSet aDataSet2411;
      // -----------------------------------
      // Storing SMDS nodes to the UNV file
      // -----------------------------------
      SMDS_NodeIteratorPtr aNodesIter = myMesh->nodesIterator();
      TRecord aRec;
      for ( aRec.label = 1; aNodesIter->more(); ++aRec.label )
      {
        const SMDS_MeshNode* aNode = aNodesIter->next();
        // aRec.label    = aNode->GetID(); -- IPAL54452
        if ( !nodeLabelByID.empty() )
          nodeLabelByID[ aNode->GetID() ] = aRec.label;
        aRec.coord[0] = aNode->X();
        aRec.coord[1] = aNode->Y();
        aRec.coord[2] = aNode->Z();
        aDataSet2411.push_back( aRec );
      }
      UNV2411::Write(out_stream,aDataSet2411);
    }

    std::vector< size_t > elemLabelByID;
    if ( !myGroups.empty() )
      elemLabelByID.resize( myMesh->MaxElementID() + 1 );

    {
      using namespace UNV2412;
      TDataSet aDataSet2412;
      TRecord aRec;
      aRec.label = 0;

      // -------------------
      // Storing SMDS Edges
      // -------------------
      if ( myMesh->NbEdges() )
      {
        SMDS_EdgeIteratorPtr anIter = myMesh->edgesIterator();
        while ( anIter->more() )
        {
          const SMDS_MeshEdge* anElem = anIter->next();
          // aRec.label = anElem->GetID();  -- IPAL54452
          ++aRec.label;
          if ( !elemLabelByID.empty() )
            elemLabelByID[ anElem->GetID() ] = aRec.label;

          aRec.fe_descriptor_id = anElem->IsQuadratic() ? 22 : 11;

          SMDS_NodeIteratorPtr aNodesIter = anElem->nodesIteratorToUNV();
          for ( aRec.node_labels.clear(); aNodesIter->more(); )
          {
            const SMDS_MeshNode* aNode = aNodesIter->next();
            if ( nodeLabelByID.empty() )
              aRec.node_labels.push_back( aNode->GetID() );
            else
              aRec.node_labels.push_back( nodeLabelByID[ aNode->GetID() ]);
          }

          aDataSet2412.push_back(aRec);
        }
      }

      // -------------------
      // Storing SMDS Faces
      // -------------------
      if ( myMesh->NbFaces() )
      {
        SMDS_FaceIteratorPtr anIter = myMesh->facesIterator();
        while ( anIter->more() )
        {
          const SMDS_MeshFace* anElem = anIter->next();
          if ( anElem->IsPoly() ) continue;

          SMDS_NodeIteratorPtr aNodesIter = anElem->nodesIteratorToUNV();
          for ( aRec.node_labels.clear(); aNodesIter->more();  ) {
            const SMDS_MeshNode* aNode = aNodesIter->next();
            if ( nodeLabelByID.empty() )
              aRec.node_labels.push_back( aNode->GetID() );
            else
              aRec.node_labels.push_back( nodeLabelByID[ aNode->GetID() ]);
          }
          switch ( anElem->NbNodes() ) {
          case 3: aRec.fe_descriptor_id = 41; break;
          case 4: aRec.fe_descriptor_id = 44; break;
          case 6: aRec.fe_descriptor_id = 42; break;
          case 7: aRec.fe_descriptor_id = 42; break;
          case 8: aRec.fe_descriptor_id = 45; break;
          case 9: aRec.fe_descriptor_id = 45; aRec.node_labels.resize( 8 ); break;
          default:
            continue;
          }
          // aRec.label = anElem->GetID(); -- IPAL54452
          ++aRec.label;
          if ( !elemLabelByID.empty() )
            elemLabelByID[ anElem->GetID() ] = aRec.label;

          aDataSet2412.push_back(aRec);
        }
      }

      // ---------------------
      // Storing SMDS Volumes
      // ---------------------
      if ( myMesh->NbVolumes() )
      {
        SMDS_VolumeIteratorPtr anIter = myMesh->volumesIterator();
        while ( anIter->more() )
        {
          const SMDS_MeshVolume* anElem = anIter->next();
          if ( anElem->IsPoly() )
            continue;
          size_t aNbNodes = anElem->NbNodes();
          switch( aNbNodes ) {
          case 4:  aRec.fe_descriptor_id = 111; break;
          case 6:  aRec.fe_descriptor_id = 112; break;
          case 8:  aRec.fe_descriptor_id = 115; break;
          case 10: aRec.fe_descriptor_id = 118; break;
          case 13: aRec.fe_descriptor_id = 114; break;
          case 15: aRec.fe_descriptor_id = 113; break;
          case 20:
          case 27: aRec.fe_descriptor_id = 116; aNbNodes = 20; break;
          default:
            continue;
          }
          // aRec.label = anElem->GetID(); -- IPAL54452
          ++aRec.label;
          if ( !elemLabelByID.empty() )
            elemLabelByID[  anElem->GetID() ] = aRec.label;

          aRec.node_labels.clear();
          SMDS_NodeIteratorPtr aNodesIter = anElem->nodesIteratorToUNV();
          while ( aNodesIter->more() && aRec.node_labels.size() < aNbNodes )
          {
            const SMDS_MeshElement* aNode = aNodesIter->next();
            if ( nodeLabelByID.empty() )
              aRec.node_labels.push_back( aNode->GetID() );
            else
              aRec.node_labels.push_back( nodeLabelByID[ aNode->GetID() ]);
          }
          aDataSet2412.push_back(aRec);
        }
      }
      UNV2412::Write(out_stream,aDataSet2412);
    }

    // --------------------
    // Storing SMDS Groups
    // --------------------
    {
      using namespace UNV2417;
      if ( myGroups.size() > 0 ) {
        TRecord aRec;
        TDataSet aDataSet2417;
        TGroupList::const_iterator aIter = myGroups.begin();
        for ( ; aIter != myGroups.end(); aIter++ )
        {
          SMESHDS_GroupBase* aGroupDS = *aIter;
          aRec.GroupName = aGroupDS->GetStoreName();
          aRec.NodeList.clear();
          aRec.ElementList.clear();

          SMDS_ElemIteratorPtr aIter = aGroupDS->GetElements();
          if ( aGroupDS->GetType() == SMDSAbs_Node ) {
            while ( aIter->more() ) {
              const SMDS_MeshElement* aNode = aIter->next();
              if ( nodeLabelByID.empty() )
                aRec.NodeList.push_back( aNode->GetID() );
              else
                aRec.NodeList.push_back( nodeLabelByID[ aNode->GetID() ]);
            }
          }
          else
          {
            while ( aIter->more() ) {
              const SMDS_MeshElement* aElem = aIter->next();
              if ( elemLabelByID.empty() )
                aRec.ElementList.push_back( aElem->GetID() );
              else
                aRec.ElementList.push_back( elemLabelByID[ aElem->GetID() ]);
            }
          }
          // 0019936: EDF 794 SMESH : Export UNV : Node color and group id
          //aDataSet2417.insert(TDataSet::value_type(aGroupDS->GetID(), aRec));
          aDataSet2417.insert(TDataSet::value_type(aGroupDS->GetID()+1, aRec));
        }
        UNV2417::Write(out_stream,aDataSet2417);
        myGroups.clear();
      }
    }

    out_stream.flush();
    out_stream.close();
    if (!check_file(myFile))
      EXCEPTION(runtime_error,"ERROR: Output file not good.");
  }
  catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
    throw;
  }
  catch(...){
    INFOS("Unknown exception was cought !!!");
    throw;
  }
  return aResult;
}
