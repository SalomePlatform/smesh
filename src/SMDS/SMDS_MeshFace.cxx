using namespace std;
// File:	SMDS_MeshFace.cxx
// Created:	Wed Jan 23 17:02:27 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshFace.ixx"


//=======================================================================
//function : SMDS_MeshFace
//purpose  : 
//=======================================================================

SMDS_MeshFace::SMDS_MeshFace(const Standard_Integer ID, const Standard_Integer nb) 
:SMDS_MeshElement(ID,nb,SMDSAbs_Face)
{
}


//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

Standard_Integer SMDS_MeshFace::NbEdges() const
{
  return myNbNodes;
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshFace::Print(Standard_OStream& OS) const
{
  OS << "face <" << myID <<" > : ";
  for (Standard_Integer i=1; i<myNbNodes; ++i)
    OS << GetConnection(i) << ",";
  OS << GetConnection(myNbNodes) << ") " << endl;
}


