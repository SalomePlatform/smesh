using namespace std;
// File:	SMDS_MeshPyramid.cxx
// Created:	Wed Jan 23 17:02:34 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshPyramid.ixx"
#include <Standard_ConstructionError.hxx>


//=======================================================================
//function : SMDS_MeshPyramid
//purpose  : Pyramid
//=======================================================================

SMDS_MeshPyramid::SMDS_MeshPyramid(const Standard_Integer ID,
				   const Standard_Integer idnode1, 
				   const Standard_Integer idnode2, 
				   const Standard_Integer idnode3,
				   const Standard_Integer idnode4,
				   const Standard_Integer idnode5):SMDS_MeshVolume(ID,5)
{
  SetConnections(idnode1,idnode2,idnode3,idnode4,idnode5);
  ComputeKey();
}



//=======================================================================
//function : SetConnections
//purpose  : Pyramid
//=======================================================================
void SMDS_MeshPyramid::SetConnections(const Standard_Integer idnode1, 
				      const Standard_Integer idnode2,
				      const Standard_Integer idnode3,
				      const Standard_Integer idnode4,
				      const Standard_Integer idnode5)
{

  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  idmin = (idmin < idnode4 ? idmin : idnode4);
  idmin = (idmin < idnode5 ? idmin : idnode5);

  myNodes[0] = idmin;
  if (idmin == idnode1) { // 1 2 3 4 5
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
    myNodes[3] = idnode4;
    myNodes[4] = idnode5;
  } else if (idmin == idnode2) { // 2 3 4 5 1
    myNodes[1] = idnode3;
    myNodes[2] = idnode4;
    myNodes[3] = idnode5;
    myNodes[4] = idnode1;
  } else if (idmin == idnode3) { // 3 4 5 1 2
    myNodes[1] = idnode4;
    myNodes[2] = idnode5;
    myNodes[3] = idnode1;
    myNodes[4] = idnode2;
  } else if (idmin == idnode4) { // 4 5 1 2 3
    myNodes[1] = idnode5;
    myNodes[2] = idnode1;
    myNodes[3] = idnode2;
    myNodes[4] = idnode3;
  } else {                      // 5 1 2 3 4
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
    myNodes[3] = idnode3;
    myNodes[4] = idnode4;
  }

}

//=======================================================================
//function : NodesOfFace
//purpose  : returns the rank node in mynodes. Useful to extract faces from volume
//=======================================================================
Standard_Integer SMDS_MeshPyramid::NodesOfFace(const Standard_Integer rankface, 
					       const Standard_Integer ranknode)
{
  static Standard_Integer facenode[5][4] = {
    {0,1,2,3},
    {0,4,1,-1},
    {1,4,2,-1},
    {2,4,3,-1},
    {0,3,4,-1}
  };

  return facenode[rankface-1][ranknode-1];
}

//=======================================================================
//function : NodesOfEdge
//purpose  : returns the rank node in mynodes. Useful to extract edges from volume
//=======================================================================
Standard_Integer SMDS_MeshPyramid::NodesOfEdge(const Standard_Integer rankedge, 
					       const Standard_Integer ranknode) const
{
  static Standard_Integer faceedge[8][2] = {
    {0,1},
    {1,2},
    {2,3},
    {0,3},
    {0,4},
    {1,4},
    {2,4},
    {3,4}

  };

  return faceedge[rankedge-1][ranknode-1];
}


//=======================================================================
//function : GetFaceDefinedByNodes
//purpose  : 
//=======================================================================
void SMDS_MeshPyramid::GetFaceDefinedByNodes(const Standard_Integer rank, 
					     const Standard_Address idnode,
					     Standard_Integer& nb) const
{
  Standard_Integer *ptr;
  ptr = (Standard_Integer *)idnode;
  ptr[0] = myNodes[NodesOfFace(rank,1)];
  ptr[1] = myNodes[NodesOfFace(rank,2)];
  ptr[2] = myNodes[NodesOfFace(rank,3)];
  nb = (NodesOfFace(rank,4) == -1 ? 3 : 4);
  if (nb == 4)
    ptr[3] = myNodes[NodesOfFace(rank,4)];

}

//=======================================================================
//function : GetEdgeDefinedByNodes
//purpose  : 
//=======================================================================
void SMDS_MeshPyramid::GetEdgeDefinedByNodes(const Standard_Integer rank, 
					    Standard_Integer& idnode1,
					    Standard_Integer& idnode2) const
{
  idnode1 = myNodes[NodesOfEdge(rank,1)];
  idnode2 = myNodes[NodesOfEdge(rank,2)];
}

