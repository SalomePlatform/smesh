using namespace std;
// File:	SMDS_SpacePosition.cxx
// Created:	Mon May 13 14:19:18 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDS_SpacePosition.ixx"

//=======================================================================
//function : SMDS_SpacePosition
//purpose  : 
//=======================================================================

SMDS_SpacePosition::SMDS_SpacePosition()
  :SMDS_Position(0,SMDS_TOP_3DSPACE),myCoords(0.,0.,0.)
{
}

//=======================================================================
//function : SMDS_SpacePosition
//purpose  : 
//=======================================================================

SMDS_SpacePosition::SMDS_SpacePosition(const Standard_Real x,
				       const Standard_Real y,
				       const Standard_Real z)
  :SMDS_Position(0,SMDS_TOP_3DSPACE),myCoords(x,y,z)
{
}

//=======================================================================
//function : SMDS_SpacePosition
//purpose  : 
//=======================================================================

SMDS_SpacePosition::SMDS_SpacePosition(const gp_Pnt& aCoords)
  :SMDS_Position(0,SMDS_TOP_3DSPACE),myCoords(aCoords)
{
}

//=======================================================================
//function : Coords
//purpose  : 
//=======================================================================

gp_Pnt SMDS_SpacePosition::Coords() const
{
  return myCoords;
}
