using namespace std;
// File:	SMDS_MeshFacesIterator.cxx
// Created:	Thu Jan 24 12:09:12 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshFacesIterator.ixx"

//=======================================================================
//function : SMDS_MeshFacesIterator
//purpose  : 
//=======================================================================

SMDS_MeshFacesIterator::SMDS_MeshFacesIterator()
{
}

//=======================================================================
//function : SMDS_MeshFacesIterator
//purpose  : 
//=======================================================================

SMDS_MeshFacesIterator::SMDS_MeshFacesIterator(const Handle(SMDS_Mesh)& M)
{
  Initialize(M);
}

//=======================================================================
//function : Initialize
//purpose  : 
//=======================================================================

void SMDS_MeshFacesIterator::Initialize(const Handle(SMDS_Mesh)& M)
{

  myMapIterator.Initialize(M->myFaces);
  if (More()) {
    myCurrentMeshElement = myMapIterator.Key();
  }
}

