using namespace std;
// File:	SMDS_MeshVolumesIterator.cxx
// Created:	Thu Jan 24 12:09:12 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshVolumesIterator.ixx"

//=======================================================================
//function : SMDS_MeshVolumesIterator
//purpose  : 
//=======================================================================

SMDS_MeshVolumesIterator::SMDS_MeshVolumesIterator()
{
}

//=======================================================================
//function : SMDS_MeshVolumesIterator
//purpose  : 
//=======================================================================

SMDS_MeshVolumesIterator::SMDS_MeshVolumesIterator(const Handle(SMDS_Mesh)& M)
{
  Initialize(M);
}

//=======================================================================
//function : Initialize
//purpose  : 
//=======================================================================

void SMDS_MeshVolumesIterator::Initialize(const Handle(SMDS_Mesh)& M)
{

  myMapIterator.Initialize(M->myVolumes);
  if (More()) {
    myCurrentMeshElement = myMapIterator.Key();
  }
}

