//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESH_Command.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_Command.ixx"
#include "SMESHDS_CommandType.hxx"
#include "utilities.h"

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
SMESHDS_Command::SMESHDS_Command(const SMESHDS_CommandType aType)
  : myType(aType), myNumber(0)
{}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddNode(const Standard_Integer NewNodeID,
			      const Standard_Real x,
			      const Standard_Real y,
			      const Standard_Real z) 
{
  if (!myType == SMESHDS_AddNode) {
    MESSAGE("SMESHDS_Command::AddNode : Bad Type");    
    return;
  }
  myIntegers.Append(NewNodeID);
  myReals.Append(x);
  myReals.Append(y);
  myReals.Append(z);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::MoveNode(const Standard_Integer NodeID,
			      const Standard_Real x,
			      const Standard_Real y,
			      const Standard_Real z) 
{
  if (!myType == SMESHDS_MoveNode) {
    MESSAGE("SMESHDS_Command::MoveNode : Bad Type");    
    return;
  }
  myIntegers.Append(NodeID);
  myReals.Append(x);
  myReals.Append(y);
  myReals.Append(z);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddEdge(const Standard_Integer NewEdgeID,
			      const Standard_Integer idnode1,
			      const Standard_Integer idnode2)
{
  if (!myType == SMESHDS_AddEdge) {
    MESSAGE("SMESHDS_Command::AddEdge : Bad Type");    
    return;
  }
  myIntegers.Append(NewEdgeID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(const Standard_Integer NewFaceID,
			      const Standard_Integer idnode1,
			      const Standard_Integer idnode2,
			      const Standard_Integer idnode3)
{
  if (!myType == SMESHDS_AddTriangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");    
    return;
  }
  myIntegers.Append(NewFaceID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(const Standard_Integer NewFaceID,
			      const Standard_Integer idnode1,
			      const Standard_Integer idnode2,
			      const Standard_Integer idnode3,
			      const Standard_Integer idnode4)
{
  if (!myType == SMESHDS_AddQuadrangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");    
    return;
  }
  myIntegers.Append(NewFaceID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myIntegers.Append(idnode4);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(const Standard_Integer NewVolID,
				const Standard_Integer idnode1,
				const Standard_Integer idnode2,
				const Standard_Integer idnode3,
				const Standard_Integer idnode4)
{
  if (!myType == SMESHDS_AddTetrahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");    
    return;
  }
  myIntegers.Append(NewVolID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myIntegers.Append(idnode4);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(const Standard_Integer NewVolID,
				const Standard_Integer idnode1,
				const Standard_Integer idnode2,
				const Standard_Integer idnode3,
				const Standard_Integer idnode4,
				const Standard_Integer idnode5)
{
  if (!myType == SMESHDS_AddPyramid) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");    
    return;
  }
  myIntegers.Append(NewVolID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myIntegers.Append(idnode4);
  myIntegers.Append(idnode5);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(const Standard_Integer NewVolID,
				const Standard_Integer idnode1,
				const Standard_Integer idnode2,
				const Standard_Integer idnode3,
				const Standard_Integer idnode4,
				const Standard_Integer idnode5,
				const Standard_Integer idnode6)
{
  if (!myType == SMESHDS_AddPrism) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");    
    return;
  }
  myIntegers.Append(NewVolID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myIntegers.Append(idnode4);
  myIntegers.Append(idnode5);
  myIntegers.Append(idnode6);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(const Standard_Integer NewVolID,
				const Standard_Integer idnode1,
				const Standard_Integer idnode2,
				const Standard_Integer idnode3,
				const Standard_Integer idnode4,
				const Standard_Integer idnode5,
				const Standard_Integer idnode6,
				const Standard_Integer idnode7,
				const Standard_Integer idnode8)
{
  if (!myType == SMESHDS_AddHexahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");    
    return;
  }
  myIntegers.Append(NewVolID);
  myIntegers.Append(idnode1);
  myIntegers.Append(idnode2);
  myIntegers.Append(idnode3);
  myIntegers.Append(idnode4);
  myIntegers.Append(idnode5);
  myIntegers.Append(idnode6);
  myIntegers.Append(idnode7);
  myIntegers.Append(idnode8);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::RemoveNode(const Standard_Integer NodeID)
{
  if (!myType == SMESHDS_RemoveNode) {
    MESSAGE("SMESHDS_Command::RemoveNode : Bad Type");    
    return;
  }
  myIntegers.Append(NodeID);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::RemoveElement(const Standard_Integer ElementID)
{
  if (!myType == SMESHDS_RemoveElement) {
    MESSAGE("SMESHDS_Command::RemoveElement : Bad Type");    
    return;
  }
  myIntegers.Append(ElementID);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
SMESHDS_CommandType SMESHDS_Command::GetType()
{
  return myType;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Command::GetNumber()
{
  return myNumber;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const TColStd_ListOfInteger& SMESHDS_Command::GetIndexes()
{
  return myIntegers;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const TColStd_ListOfReal& SMESHDS_Command::GetCoords()
{
  return myReals;
}
