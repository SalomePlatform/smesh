using namespace std;
// File:	SMDS_MeshTriangle.cxx
// Created:	Wed Jan 23 17:02:27 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshTriangle.ixx"


//=======================================================================
//function : SMDS_MeshTriangle
//purpose  : 
//=======================================================================

SMDS_MeshTriangle::SMDS_MeshTriangle(const Standard_Integer ID,
				     const Standard_Integer idnode1, 
				     const Standard_Integer idnode2, 
				     const Standard_Integer idnode3):SMDS_MeshFace(ID,3)
{
  SetConnections(idnode1,idnode2,idnode3);
  ComputeKey();
}


//=======================================================================
//function : SetConnections
//purpose  : 
//=======================================================================
void SMDS_MeshTriangle::SetConnections(const Standard_Integer idnode1, 
				       const Standard_Integer idnode2,
				       const Standard_Integer idnode3)
{
  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  
  myNodes[0] = idmin;
  if (idmin == idnode1) {
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
  } else if (idmin == idnode2) {
    myNodes[1] = idnode3;
    myNodes[2] = idnode1;
  } else {
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
  }

}

