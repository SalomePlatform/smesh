using namespace std;
// File:	SMDS_MeshNodesIterator.cxx
// Created:	Thu Jan 24 12:09:12 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshNodesIterator.ixx"

//=======================================================================
//function : SMDS_MeshNodesIterator
//purpose  : 
//=======================================================================

SMDS_MeshNodesIterator::SMDS_MeshNodesIterator()
{
}

//=======================================================================
//function : SMDS_MeshNodesIterator
//purpose  : 
//=======================================================================

SMDS_MeshNodesIterator::SMDS_MeshNodesIterator(const Handle(SMDS_Mesh)& M)
{
  Initialize(M);
}

//=======================================================================
//function : Initialize
//purpose  : 
//=======================================================================

void SMDS_MeshNodesIterator::Initialize(const Handle(SMDS_Mesh)& M)
{

  myMapIterator.Initialize(M->myNodes);
  if (More()) {
    myCurrentMeshElement = myMapIterator.Key();
  }
}

