using namespace std;
// File:	SMDS_MeshVolume.cxx
// Created:	Wed Jan 23 17:02:34 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshVolume.ixx"
#include <Standard_ConstructionError.hxx>

//=======================================================================
//function : SMDS_MeshVolume
//purpose  : 
//=======================================================================

SMDS_MeshVolume::SMDS_MeshVolume(const Standard_Integer ID, const Standard_Integer nb) 
:SMDS_MeshElement(ID,nb,SMDSAbs_Volume)
{
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshVolume::Print(Standard_OStream& OS) const
{
  OS << "volume <" << myID <<"> : " ;
  for (Standard_Integer i=1; i<myNbNodes; ++i)
    OS << GetConnection(i) << ",";
  OS << GetConnection(myNbNodes) << ") " << endl;
}


