using namespace std;
// File:	SMDS_MeshQuadrangle.cxx
// Created:	Wed Jan 23 17:02:27 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshQuadrangle.ixx"



//=======================================================================
//function : SMDS_MeshQuadrangle
//purpose  : 
//=======================================================================

SMDS_MeshQuadrangle::SMDS_MeshQuadrangle(const Standard_Integer ID,
					 const Standard_Integer	idnode1, 
					 const Standard_Integer	idnode2, 
					 const Standard_Integer	idnode3,
					 const Standard_Integer	idnode4):SMDS_MeshFace(ID,4)
{
  SetConnections(idnode1,idnode2,idnode3,idnode4);
  ComputeKey();
}



//=======================================================================
//function : SetConnections
//purpose  : 
//=======================================================================
void SMDS_MeshQuadrangle::SetConnections(const Standard_Integer	idnode1, 
					 const Standard_Integer	idnode2,
					 const Standard_Integer	idnode3,
					 const Standard_Integer	idnode4)
{
  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  idmin = (idmin < idnode4 ? idmin : idnode4);
  
  myNodes[0] = idmin;
  if (idmin == idnode1) {
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
    myNodes[3] = idnode4;
  } else if (idmin == idnode2) {
    myNodes[1] = idnode3;
    myNodes[2] = idnode4;
    myNodes[3] = idnode1;
  } else if (idmin == idnode3) {
    myNodes[1] = idnode4;
    myNodes[2] = idnode1;
    myNodes[3] = idnode2;
  } else {
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
    myNodes[3] = idnode3;
  }

}

