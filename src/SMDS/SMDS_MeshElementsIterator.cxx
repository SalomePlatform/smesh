using namespace std;
// File:	SMDS_MeshElementsIterator.cxx
// Created:	Thu Jan 24 12:09:12 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_MeshElementsIterator.ixx"

void SMDS_MeshElementsIterator::Delete()
{}

//=======================================================================
//function : Next
//purpose  : 
//=======================================================================

void SMDS_MeshElementsIterator::Next()
{
  myMapIterator.Next();
  if (More()) {
    myCurrentMeshElement = myMapIterator.Key();
  }
}
