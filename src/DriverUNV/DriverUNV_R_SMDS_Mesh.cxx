// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
#include "DriverUNV_R_SMDS_Mesh.h"
#include "SMDS_Mesh.hxx"

#include "utilities.h"

#include "UNV2411_Structure.hxx"
#include "UNV2412_Structure.hxx"
#include "UNV_Utilities.hxx"

using namespace std;


#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif


Driver_Mesh::Status DriverUNV_R_SMDS_Mesh::Perform()
{
  Status aResult = DRS_OK;
  std::ifstream in_stream(myFile.c_str());
  try{
    {
      using namespace UNV2411;
      TDataSet aDataSet2411;
      UNV2411::Read(in_stream,aDataSet2411);
      if(MYDEBUG) MESSAGE("Perform - aDataSet2411.size() = "<<aDataSet2411.size());
      TDataSet::const_iterator anIter = aDataSet2411.begin();
      for(; anIter != aDataSet2411.end(); anIter++){
	const TNodeLab& aLabel = anIter->first;
	const TRecord& aRec = anIter->second;
	myMesh->AddNodeWithID(aRec.coord[0],aRec.coord[1],aRec.coord[2],aLabel);
      }
    }
    {
      using namespace UNV2412;
      in_stream.seekg(0);
      TDataSet aDataSet2412;
      UNV2412::Read(in_stream,aDataSet2412);
      TDataSet::const_iterator anIter = aDataSet2412.begin();
      if(MYDEBUG) MESSAGE("Perform - aDataSet2412.size() = "<<aDataSet2412.size());
      for(; anIter != aDataSet2412.end(); anIter++){
	SMDS_MeshElement* anElement = NULL;
	const TElementLab& aLabel = anIter->first;
	const TRecord& aRec = anIter->second;
	if(IsBeam(aRec.fe_descriptor_id)) {
          if(aRec.fe_descriptor_id == 11) {
            // edge with two nodes
            anElement = myMesh->AddEdgeWithID(aRec.node_labels[0],
                                              aRec.node_labels[1],
                                              aLabel);
          }
          else {
            // quadratic edge (with 3 nodes)
            anElement = myMesh->AddEdgeWithID(aRec.node_labels[0],
                                              aRec.node_labels[1],
                                              aRec.node_labels[2],
                                              aLabel);
          }
	}
        else if(IsFace(aRec.fe_descriptor_id)) {
	  switch(aRec.fe_descriptor_id){
	  case 71: // TRI3
	  case 72:
	  case 74:
	    
	  case 41: // Plane Stress Linear Triangle - TRI3
	  case 91: // Thin Shell Linear Triangle - TRI3
	    anElement = myMesh->AddFaceWithID(aRec.node_labels[0],
					      aRec.node_labels[1],
					      aRec.node_labels[2],
					      aLabel);
	    break;
	    
	  case 42: // Plane Stress Quadratic Triangle - TRI6
	  case 92: // Thin Shell Quadratic Triangle - TRI6
	    anElement = myMesh->AddFaceWithID(aRec.node_labels[0],
					      aRec.node_labels[1],
					      aRec.node_labels[2],
					      aRec.node_labels[3],
					      aRec.node_labels[4],
					      aRec.node_labels[5],
					      aLabel);
	    break;
	    
	  case 44: // Plane Stress Linear Quadrilateral - QUAD4
	  case 94: // Thin Shell   Linear Quadrilateral -  QUAD4
	    anElement = myMesh->AddFaceWithID(aRec.node_labels[0],
					      aRec.node_labels[1],
					      aRec.node_labels[2],
					      aRec.node_labels[3],
					      aLabel);
	    break;
	    
	  case 45: // Plane Stress Quadratic Quadrilateral - QUAD8
	  case 95: // Thin Shell   Quadratic Quadrilateral - QUAD8
	    anElement = myMesh->AddFaceWithID(aRec.node_labels[0],
					      aRec.node_labels[1],
					      aRec.node_labels[2],
					      aRec.node_labels[3],
					      aRec.node_labels[4],
					      aRec.node_labels[5],
					      aRec.node_labels[6],
					      aRec.node_labels[7],
					      aLabel);
	    break;
	  }
	}
        else if(IsVolume(aRec.fe_descriptor_id)){
	  switch(aRec.fe_descriptor_id){
	    
	  case 111: // Solid Linear Tetrahedron - TET4
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[3],
						aLabel);
	    break;

	  case 118: // Solid Quadratic Tetrahedron - TET10
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[3],
						aRec.node_labels[6],
						aRec.node_labels[5],
						aRec.node_labels[4],
						aRec.node_labels[7],
						aRec.node_labels[9],
						aRec.node_labels[8],
						aLabel);
	    break;
	    
	  case 112: // Solid Linear Prism - PRISM6
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[3],
						aRec.node_labels[5],
						aRec.node_labels[4],
						aLabel);
	    break;
	    
	  case 113: // Solid Quadratic Prism - PRISM15
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[3],
						aRec.node_labels[5],
						aRec.node_labels[4],
						aRec.node_labels[8],
						aRec.node_labels[7],
						aRec.node_labels[6],
						aRec.node_labels[11],
						aRec.node_labels[10],
						aRec.node_labels[9],
						aRec.node_labels[12],
						aRec.node_labels[14],
						aRec.node_labels[13],
						aLabel);
	    break;
	    
	  case 115: // Solid Linear Brick - HEX8
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[3],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[4],
						aRec.node_labels[7],
						aRec.node_labels[6],
						aRec.node_labels[5],
						aLabel);
	    break;

	  case 116: // Solid Quadratic Brick - HEX20
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[3],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[4],
						aRec.node_labels[7],
						aRec.node_labels[6],
						aRec.node_labels[5],
						aRec.node_labels[11],
						aRec.node_labels[10],
						aRec.node_labels[9],
						aRec.node_labels[8],
						aRec.node_labels[15],
						aRec.node_labels[14],
						aRec.node_labels[13],
						aRec.node_labels[12],
						aRec.node_labels[16],
						aRec.node_labels[19],
						aRec.node_labels[18],
						aRec.node_labels[17],
						aLabel);
	    break;

	  case 114: // pyramid of 13 nodes (quadratic) - PIRA13
	    anElement = myMesh->AddVolumeWithID(aRec.node_labels[0],
						aRec.node_labels[3],
						aRec.node_labels[2],
						aRec.node_labels[1],
						aRec.node_labels[4],
						aRec.node_labels[8],
						aRec.node_labels[7],
						aRec.node_labels[6],
						aRec.node_labels[5],
						aRec.node_labels[9],
						aRec.node_labels[12],
						aRec.node_labels[11],
						aRec.node_labels[10],
						aLabel);
	    break;

	  }
	}
	if(!anElement)
	  MESSAGE("DriverUNV_R_SMDS_Mesh::Perform - can not add element with ID = "<<aLabel<<" and type = "<<aRec.fe_descriptor_id);
      }
    }
  }
  catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }
  catch(...){
    INFOS("Unknown exception was cought !!!");
  }
  return aResult;
}
