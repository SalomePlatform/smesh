using namespace std;
// File:	SMDS_VertexPosition.cxx
// Created:	Mon May 13 14:41:57 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_VertexPosition.ixx"
#include "utilities.h"

//=======================================================================
//function : SMDS_VertexPosition
//purpose  : 
//=======================================================================

SMDS_VertexPosition::SMDS_VertexPosition()
  :SMDS_Position(0,SMDS_TOP_VERTEX)
{
}

//=======================================================================
//function : SMDS_VertexPosition
//purpose  : 
//=======================================================================

SMDS_VertexPosition::SMDS_VertexPosition(const Standard_Integer aVertexId)
  :SMDS_Position(aVertexId,SMDS_TOP_VERTEX)
{
}


//=======================================================================
//function : Coords
//purpose  : 
//=======================================================================

gp_Pnt SMDS_VertexPosition::Coords() const
{
  MESSAGE( "SMDS_VertexPosition::Coords not implemented" );
  return gp_Pnt(0,0,0);
}
