using namespace std;
// File:	SMDS_Position.cxx
// Created:	Mon May 13 13:50:20 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_Position.ixx"

//=======================================================================
//function : SMDS_Position
//purpose  : 
//=======================================================================

SMDS_Position::SMDS_Position(const Standard_Integer aShapeId, 
			     const SMDS_TypeOfPosition aType)
  :myShapeId(aShapeId),myType(aType)
{
}
