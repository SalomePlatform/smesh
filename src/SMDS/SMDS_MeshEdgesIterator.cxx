using namespace std;
// File:	SMDS_MeshEdgesIterator.cxx
// Created:	Thu Jan 24 12:09:12 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshEdgesIterator.ixx"

//=======================================================================
//function : SMDS_MeshEdgesIterator
//purpose  : 
//=======================================================================

SMDS_MeshEdgesIterator::SMDS_MeshEdgesIterator()
{
}

//=======================================================================
//function : SMDS_MeshEdgesIterator
//purpose  : 
//=======================================================================

SMDS_MeshEdgesIterator::SMDS_MeshEdgesIterator(const Handle(SMDS_Mesh)& M)
{
  Initialize(M);
}

//=======================================================================
//function : Initialize
//purpose  : 
//=======================================================================

void SMDS_MeshEdgesIterator::Initialize(const Handle(SMDS_Mesh)& M)
{

  myMapIterator.Initialize(M->myEdges);
  if (More()) {
    myCurrentMeshElement = myMapIterator.Key();
  }
}

