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
//  File   : SMESH_Script.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_Script.ixx"
#include "SMESHDS_Command.hxx"
#include "SMESHDS_CommandType.hxx"
#include "SMESHDS_ListOfCommand.hxx"

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddNode(const Standard_Integer NewNodeID,
			     const Standard_Real x,
			     const Standard_Real y,
			     const Standard_Real z) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddNode);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddNode) {
      com = new SMESHDS_Command(SMESHDS_AddNode);
      myCommands.Append (com);
    }
  }
  com->AddNode (NewNodeID,x,y,z);
}


//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(const Standard_Integer NewEdgeID,
			     const Standard_Integer idnode1,
			     const Standard_Integer idnode2) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddEdge);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddEdge) {
      com = new SMESHDS_Command(SMESHDS_AddEdge);
      myCommands.Append (com);
    }
  }
  com->AddEdge (NewEdgeID,idnode1,idnode2);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(const Standard_Integer NewFaceID,
			     const Standard_Integer idnode1,
			     const Standard_Integer idnode2,
			     const Standard_Integer idnode3) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddTriangle);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddTriangle) {
      com = new SMESHDS_Command(SMESHDS_AddTriangle);
      myCommands.Append (com);
    }
  }
  com->AddFace (NewFaceID,idnode1,idnode2,idnode3);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(const Standard_Integer NewFaceID,
			     const Standard_Integer idnode1,
			     const Standard_Integer idnode2,
			     const Standard_Integer idnode3,
			     const Standard_Integer idnode4) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddQuadrangle);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddQuadrangle) {
      com = new SMESHDS_Command(SMESHDS_AddQuadrangle);
      myCommands.Append (com);
    }
  }
  com->AddFace (NewFaceID,idnode1,idnode2,idnode3,idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(const Standard_Integer NewID,
			       const Standard_Integer idnode1,
			       const Standard_Integer idnode2,
			       const Standard_Integer idnode3,
			       const Standard_Integer idnode4) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddTetrahedron);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddQuadrangle) {
      com = new SMESHDS_Command(SMESHDS_AddQuadrangle);
      myCommands.Append (com);
    }
  }
  com->AddVolume (NewID,idnode1,idnode2,idnode3,idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(const Standard_Integer NewID,
			       const Standard_Integer idnode1,
			       const Standard_Integer idnode2,
			       const Standard_Integer idnode3,
			       const Standard_Integer idnode4,
			       const Standard_Integer idnode5) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddPyramid);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddPyramid) {
      com = new SMESHDS_Command(SMESHDS_AddPyramid);
      myCommands.Append (com);
    }
  }
  com->AddVolume (NewID,idnode1,idnode2,idnode3,idnode4,idnode5);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(const Standard_Integer NewID,
			       const Standard_Integer idnode1,
			       const Standard_Integer idnode2,
			       const Standard_Integer idnode3,
			       const Standard_Integer idnode4,
			       const Standard_Integer idnode5,
			       const Standard_Integer idnode6) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddPrism);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddPrism) {
      com = new SMESHDS_Command(SMESHDS_AddPrism);
      myCommands.Append (com);
    }
  }
  com->AddVolume (NewID,idnode1,idnode2,idnode3,idnode4,idnode5,idnode6);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(const Standard_Integer NewID,
			       const Standard_Integer idnode1,
			       const Standard_Integer idnode2,
			       const Standard_Integer idnode3,
			       const Standard_Integer idnode4,
			       const Standard_Integer idnode5,
			       const Standard_Integer idnode6,
			       const Standard_Integer idnode7,
			       const Standard_Integer idnode8) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_AddHexahedron);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_AddHexahedron) {
      com = new SMESHDS_Command(SMESHDS_AddHexahedron);
      myCommands.Append (com);
    }
  }
  com->AddVolume (NewID,idnode1,idnode2,idnode3,idnode4,
		 idnode5,idnode6,idnode7,idnode8);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::MoveNode(const Standard_Integer NewNodeID,
			      const Standard_Real x,
			      const Standard_Real y,
			      const Standard_Real z) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_MoveNode);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_MoveNode) {
      com = new SMESHDS_Command(SMESHDS_MoveNode);
      myCommands.Append (com);
    }
  }
  com->MoveNode (NewNodeID,x,y,z);
}


//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveNode(const Standard_Integer ID) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_RemoveNode);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_RemoveNode) {
      com = new SMESHDS_Command(SMESHDS_RemoveNode);
      myCommands.Append (com);
    }
  }
  com->RemoveNode (ID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
 void SMESHDS_Script::RemoveElement(const Standard_Integer ElementID) 
{
  Handle(SMESHDS_Command) com;
  if (myCommands.IsEmpty()) {
    com = new SMESHDS_Command(SMESHDS_RemoveElement);
    myCommands.Append (com);
  }
  else {
    com = myCommands.Last();
    if (com->GetType() != SMESHDS_RemoveElement) {
      com = new SMESHDS_Command(SMESHDS_RemoveElement);
      myCommands.Append (com);
    }
  }
  com->RemoveElement (ElementID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::Clear() 
{
  myCommands.Clear();
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const SMESHDS_ListOfCommand&  SMESHDS_Script::GetCommands() 
{
  return myCommands;
}
