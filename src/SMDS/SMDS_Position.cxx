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
//  File   : SMDS_Position.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

#include "SMDS_Position.hxx"

//=======================================================================
//function : SMDS_Position
//purpose  : 
//=======================================================================

SMDS_Position::SMDS_Position(int aShapeId) :myShapeId(aShapeId)
{
}

//=======================================================================
//function : destructor SMDS_Position
//purpose  :
//=======================================================================

SMDS_Position::~SMDS_Position()
{
}

//=======================================================================
//function : SetShapeId
//purpose  : 
//=======================================================================

void SMDS_Position::SetShapeId(int aShapeId)
{
	myShapeId = aShapeId;
}

//=======================================================================
//function : GetShapeId
//purpose  : 
//=======================================================================

int SMDS_Position::GetShapeId() const
{
	return myShapeId;
}
