using namespace std;
// File:	SMDS_EdgePosition.cxx
// Created:	Mon May 13 14:49:28 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_EdgePosition.ixx"
#include "utilities.h"

//=======================================================================
//function : SMDS_EdgePosition
//purpose  : 
//=======================================================================

SMDS_EdgePosition::SMDS_EdgePosition()
  :SMDS_Position(0,SMDS_TOP_EDGE),myUParameter(0.)
{
}

//=======================================================================
//function : SMDS_EdgePosition
//purpose  : 
//=======================================================================

SMDS_EdgePosition::SMDS_EdgePosition(const Standard_Integer aEdgeId,
				     const Standard_Real aUParam)
  :SMDS_Position(aEdgeId,SMDS_TOP_EDGE),myUParameter(aUParam)
{
}

//=======================================================================
//function : Coords
//purpose  : 
//=======================================================================

gp_Pnt SMDS_EdgePosition::Coords() const
{
  MESSAGE( "SMDS_EdgePosition::Coords not implemented" );
  return gp_Pnt(0,0,0);
}
