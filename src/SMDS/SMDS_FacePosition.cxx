using namespace std;
// File:	SMDS_FacePosition.cxx
// Created:	Mon May 13 14:56:28 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_FacePosition.ixx"
#include "utilities.h"

//=======================================================================
//function : SMDS_FacePosition
//purpose  : 
//=======================================================================

SMDS_FacePosition::SMDS_FacePosition()
  :SMDS_Position(0,SMDS_TOP_FACE),myUParameter(0.),myVParameter(0.)
{
}

//=======================================================================
//function : SMDS_FacePosition
//purpose  : 
//=======================================================================

SMDS_FacePosition::SMDS_FacePosition(const Standard_Integer aEdgeId,
				     const Standard_Real aUParam,
				     const Standard_Real aVParam)
  :SMDS_Position(aEdgeId,SMDS_TOP_FACE),
   myUParameter(aUParam),myVParameter(aVParam)
{
}

//=======================================================================
//function : Coords
//purpose  : 
//=======================================================================

gp_Pnt SMDS_FacePosition::Coords() const
{
  MESSAGE( "SMDS_FacePosition::Coords not implemented" );
  return gp_Pnt(0,0,0);
}
