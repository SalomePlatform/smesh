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

#include "SMESHDS_Script.hxx"

using namespace std;

//=======================================================================
//function : getCommand
//purpose  : 
//=======================================================================
SMESHDS_Command* SMESHDS_Script::getCommand(const SMESHDS_CommandType aType)
{
  SMESHDS_Command* com;
  if (myCommands.empty())
  {
    com = new SMESHDS_Command(aType);
    myCommands.insert(myCommands.end(),com);
  }
  else
  {
    com = myCommands.back();
    if (com->GetType() != aType)
    {
      com = new SMESHDS_Command(aType);
      myCommands.insert(myCommands.end(),com);
    }
  }
  return com;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddNode(int NewNodeID, double x, double y, double z)
{
  getCommand(SMESHDS_AddNode)->AddNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(int NewEdgeID, int idnode1, int idnode2)
{
  getCommand(SMESHDS_AddEdge)->AddEdge(NewEdgeID, idnode1, idnode2);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
                             int idnode1, int idnode2, int idnode3)
{
  getCommand(SMESHDS_AddTriangle)->AddFace(NewFaceID,
                                           idnode1, idnode2, idnode3);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
                             int idnode1, int idnode2,
                             int idnode3, int idnode4)
{
  getCommand(SMESHDS_AddQuadrangle)->AddFace(NewFaceID,
                                             idnode1, idnode2,
                                             idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2,
                               int idnode3, int idnode4)
{
  getCommand(SMESHDS_AddTetrahedron)->AddVolume(NewID,
                                                idnode1, idnode2,
                                                idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2,
                               int idnode3, int idnode4, int idnode5)
{
  getCommand(SMESHDS_AddPyramid)->AddVolume(NewID,
                                            idnode1, idnode2,
                                            idnode3, idnode4, idnode5);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2, int idnode3,
                               int idnode4, int idnode5, int idnode6)
{
  getCommand(SMESHDS_AddPrism)->AddVolume(NewID,
                                          idnode1, idnode2, idnode3,
                                          idnode4, idnode5, idnode6);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
                               int idnode1, int idnode2, int idnode3, int idnode4,
                               int idnode5, int idnode6, int idnode7, int idnode8)
{
  getCommand(SMESHDS_AddHexahedron)->AddVolume(NewID,
                                               idnode1, idnode2, idnode3, idnode4,
                                               idnode5, idnode6, idnode7, idnode8);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::MoveNode(int NewNodeID, double x, double y, double z)
{
  getCommand(SMESHDS_MoveNode)->MoveNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveNode(int ID)
{
  getCommand(SMESHDS_RemoveNode)->RemoveNode(ID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveElement(int ElementID)
{
  getCommand(SMESHDS_RemoveElement)->RemoveElement(ElementID);
}

//=======================================================================
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

void SMESHDS_Script::ChangeElementNodes(int ElementID, int nodes[], int nbnodes)
{
  getCommand(SMESHDS_ChangeElementNodes)->ChangeElementNodes( ElementID, nodes, nbnodes );
}

//=======================================================================
//function : Renumber
//purpose  : 
//=======================================================================

void SMESHDS_Script::Renumber (const bool isNodes, const int startID, const int deltaID)
{
  getCommand(SMESHDS_Renumber)->Renumber( isNodes, startID, deltaID );
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::Clear()
{
	myCommands.clear();
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const list<SMESHDS_Command*>& SMESHDS_Script::GetCommands()
{
	return myCommands;
}
