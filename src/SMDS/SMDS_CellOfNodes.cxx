// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMDS : implementation of Salome mesh data structure
//

#include "SMDS_CellOfNodes.hxx"

SMDS_CellOfNodes::SMDS_CellOfNodes( int id, int shapeID )
  : myID( id )
{
  setShapeID( shapeID );
}

void SMDS_CellOfNodes::setID(const int id)
{
  myID = id;
}

int SMDS_CellOfNodes::GetID() const
{
  return myID;
}

void SMDS_CellOfNodes::setShapeID( const int shapeID )
{
  myShapeID = ( shapeID << BITS_SHIFT ) | ( myShapeID & BIT_IS_MARKED );
}

int SMDS_CellOfNodes::GetShapeID() const
{
  return myShapeID >> BITS_SHIFT;
}

void SMDS_CellOfNodes::setIsMarked( bool is ) const
{
  const_cast< SMDS_CellOfNodes* >( this )->myShapeID = ( myShapeID & ~BIT_IS_MARKED ) | is;
}

bool SMDS_CellOfNodes::isMarked() const
{
  return myShapeID & BIT_IS_MARKED;
}
