using namespace std;
// File:	SMDS_MeshHexahedron.cxx
// Created:	Wed Jan 23 17:02:34 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshHexahedron.ixx"
#include <Standard_ConstructionError.hxx>


//=======================================================================
//function : SMDS_MeshHexahedron
//purpose  : Hexaedra
//=======================================================================

SMDS_MeshHexahedron::SMDS_MeshHexahedron(const Standard_Integer ID,
					 const Standard_Integer	idnode1, 
					 const Standard_Integer	idnode2, 
					 const Standard_Integer	idnode3,
					 const Standard_Integer	idnode4,
					 const Standard_Integer	idnode5,
					 const Standard_Integer	idnode6,
					 const Standard_Integer	idnode7,
					 const Standard_Integer	idnode8):SMDS_MeshVolume(ID,8)
{
  SetConnections(idnode1,idnode2,idnode3,idnode4,idnode5,idnode6,idnode7,idnode8);
  ComputeKey();
}


//=======================================================================
//function : SetConnections
//purpose  : Heaxahedron
//=======================================================================
void SMDS_MeshHexahedron::SetConnections(const Standard_Integer	idnode1, 
					 const Standard_Integer	idnode2,
					 const Standard_Integer	idnode3,
					 const Standard_Integer	idnode4,
					 const Standard_Integer	idnode5,
					 const Standard_Integer	idnode6,
					 const Standard_Integer	idnode7,
					 const Standard_Integer	idnode8)
{
  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  idmin = (idmin < idnode4 ? idmin : idnode4);
  idmin = (idmin < idnode5 ? idmin : idnode5);
  idmin = (idmin < idnode6 ? idmin : idnode6);
  idmin = (idmin < idnode7 ? idmin : idnode7);
  idmin = (idmin < idnode8 ? idmin : idnode8);

  myNodes[0] = idmin;
  if (idmin == idnode1) { // 1 2 3 4 5 6 7 8
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
    myNodes[3] = idnode4;
    myNodes[4] = idnode5;
    myNodes[5] = idnode6;
    myNodes[6] = idnode7;
    myNodes[7] = idnode8;
  } else if (idmin == idnode2) { // 2 3 4 1 6 7 8 5
    myNodes[1] = idnode3;
    myNodes[2] = idnode4;
    myNodes[3] = idnode1;
    myNodes[4] = idnode6;
    myNodes[5] = idnode7;
    myNodes[6] = idnode8;
    myNodes[7] = idnode5;
  } else if (idmin == idnode3) { // 3 4 1 2 7 8 5 6
    myNodes[1] = idnode4;
    myNodes[2] = idnode1;
    myNodes[3] = idnode2;
    myNodes[4] = idnode7;
    myNodes[5] = idnode8;
    myNodes[6] = idnode5;
    myNodes[7] = idnode6;
  } else if (idmin == idnode4) { // 4 1 2 3 8 5 6 7
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
    myNodes[3] = idnode3;
    myNodes[4] = idnode8;
    myNodes[5] = idnode5;
    myNodes[6] = idnode6;
    myNodes[7] = idnode7;
  } else if (idmin == idnode5) { // 5 6 7 8 1 2 3 4
    myNodes[1] = idnode6;
    myNodes[2] = idnode7;
    myNodes[3] = idnode8;
    myNodes[4] = idnode1;
    myNodes[5] = idnode2;
    myNodes[6] = idnode3;
    myNodes[7] = idnode4;
  } else if (idmin == idnode6){  // 6 7 8 5 2 3 4 1
    myNodes[1] = idnode7;
    myNodes[2] = idnode8;
    myNodes[3] = idnode5;
    myNodes[4] = idnode2;
    myNodes[5] = idnode3;
    myNodes[6] = idnode4;
    myNodes[7] = idnode1;
  } else if (idmin == idnode7) {  // 7 8 5 6 3 4 1 2
    myNodes[1] = idnode8;
    myNodes[2] = idnode5;
    myNodes[3] = idnode6;
    myNodes[4] = idnode3;
    myNodes[5] = idnode4;
    myNodes[6] = idnode1;
    myNodes[7] = idnode2;
  } else {                        // 8 5 6 7 4 1 2 3
    myNodes[1] = idnode5;
    myNodes[2] = idnode6;
    myNodes[3] = idnode7;
    myNodes[4] = idnode4;
    myNodes[5] = idnode1;
    myNodes[6] = idnode2;
    myNodes[7] = idnode3;
  }

}

//=======================================================================
//function : NodesOfFace
//purpose  : returns the rank node in mynodes. Useful to extract faces from volume
//=======================================================================
Standard_Integer SMDS_MeshHexahedron::NodesOfFace(const Standard_Integer rankface, 
					       const Standard_Integer ranknode)
{
  static Standard_Integer facenode[6][4] = {
    {0,1,2,3},
    {4,7,6,5},
    {0,4,5,1},
    {1,5,6,2},
    {2,6,7,3},
    {0,3,7,4}
  };

  return facenode[rankface-1][ranknode-1];
}

//=======================================================================
//function : NodesOfEdge
//purpose  : returns the rank node in mynodes. Useful to extract edges from volume
//=======================================================================
Standard_Integer SMDS_MeshHexahedron::NodesOfEdge(const Standard_Integer rankedge, 
						  const Standard_Integer ranknode) const
{
  static Standard_Integer faceedge[12][2] = {
    {0,1},
    {1,2},
    {2,3},
    {0,3},
    {4,7},
    {6,7},
    {5,6},
    {4,5},
    {0,4},
    {1,5},
    {2,6},
    {3,7}

  };

  return faceedge[rankedge-1][ranknode-1];
}

//=======================================================================
//function : GetFaceDefinedByNodes
//purpose  : 
//=======================================================================
void SMDS_MeshHexahedron::GetFaceDefinedByNodes(const Standard_Integer rank, 
						const Standard_Address idnode,
						Standard_Integer& nb) const
{
  Standard_Integer *ptr;
  nb = 4;
  ptr = (Standard_Integer *)idnode;
  ptr[0] = myNodes[NodesOfFace(rank,1)];
  ptr[1] = myNodes[NodesOfFace(rank,2)];
  ptr[2] = myNodes[NodesOfFace(rank,3)];
  ptr[3] = myNodes[NodesOfFace(rank,4)];

}

//=======================================================================
//function : GetEdgeDefinedByNodes
//purpose  : 
//=======================================================================
void SMDS_MeshHexahedron::GetEdgeDefinedByNodes(const Standard_Integer rank, 
					    Standard_Integer& idnode1,
					    Standard_Integer& idnode2) const
{
  idnode1 = myNodes[NodesOfEdge(rank,1)];
  idnode2 = myNodes[NodesOfEdge(rank,2)];
}

