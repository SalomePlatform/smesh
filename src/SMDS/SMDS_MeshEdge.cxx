using namespace std;
// File:	SMDS_MeshEdge.cxx
// Created:	Wed Jan 23 17:02:20 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshEdge.ixx"

//=======================================================================
//function : SMDS_MeshEdge
//purpose  : 
//=======================================================================

SMDS_MeshEdge::SMDS_MeshEdge(const Standard_Integer ID,
			     const Standard_Integer idnode1, 
			     const Standard_Integer idnode2) :
SMDS_MeshElement(ID,2,SMDSAbs_Edge)
{
  SetConnections(idnode1,idnode2);
  ComputeKey();
}

//=======================================================================
//function : SetConnections
//purpose  : 
//=======================================================================
void SMDS_MeshEdge::SetConnections(const Standard_Integer idnode1, 
				   const Standard_Integer idnode2)
{
  Standard_Integer idmin = (idnode1 < idnode2? idnode1 : idnode2);

  myNodes[0] = idmin;
  if (idmin == idnode1) {
    myNodes[1] = idnode2;
  } else {
    myNodes[1] = idnode1;
  }
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshEdge::Print(Standard_OStream& OS) const
{
  OS << "edge <" << myID <<"> : (" << myNodes[0] << " , " << myNodes[1] << ") " << endl;  
}




