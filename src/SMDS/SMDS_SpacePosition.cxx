//  SMESH SMDS : implementaion of Salome mesh data structure
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMDS_SpacePosition.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
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
