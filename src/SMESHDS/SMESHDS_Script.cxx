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

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddNode(int NewNodeID, double x, double y, double z)
{
	SMESHDS_Command* com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddNode);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddNode)
		{
			com = new SMESHDS_Command(SMESHDS_AddNode);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(int NewEdgeID, int idnode1, int idnode2)
{
	SMESHDS_Command* com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddEdge);
		myCommands.insert(myCommands.end(), com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddEdge)
		{
			com = new SMESHDS_Command(SMESHDS_AddEdge);
			myCommands.insert(myCommands.end(), com);
		}
	}
	com->AddEdge(NewEdgeID, idnode1, idnode2);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
	int idnode1, int idnode2, int idnode3)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddTriangle);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddTriangle)
		{
			com = new SMESHDS_Command(SMESHDS_AddTriangle);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddFace(NewFaceID, idnode1, idnode2, idnode3);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(int NewFaceID,
	int idnode1, int idnode2, int idnode3, int idnode4)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddQuadrangle);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddQuadrangle)
		{
			com = new SMESHDS_Command(SMESHDS_AddQuadrangle);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddFace(NewFaceID, idnode1, idnode2, idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
	int idnode1, int idnode2, int idnode3, int idnode4)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddTetrahedron);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddTetrahedron)
		{
			com = new SMESHDS_Command(SMESHDS_AddTetrahedron);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddVolume(NewID, idnode1, idnode2, idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
	int idnode1, int idnode2, int idnode3, int idnode4, int idnode5)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddPyramid);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddPyramid)
		{
			com = new SMESHDS_Command(SMESHDS_AddPyramid);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddVolume(NewID, idnode1, idnode2, idnode3, idnode4, idnode5);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
	int idnode1,
	int idnode2, int idnode3, int idnode4, int idnode5, int idnode6)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddPrism);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddPrism)
		{
			com = new SMESHDS_Command(SMESHDS_AddPrism);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddVolume(NewID, idnode1, idnode2, idnode3, idnode4, idnode5, idnode6);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(int NewID,
	int idnode1,
	int idnode2,
	int idnode3,
	int idnode4, int idnode5, int idnode6, int idnode7, int idnode8)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_AddHexahedron);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_AddHexahedron)
		{
			com = new SMESHDS_Command(SMESHDS_AddHexahedron);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->AddVolume(NewID, idnode1, idnode2, idnode3, idnode4,
		idnode5, idnode6, idnode7, idnode8);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::MoveNode(int NewNodeID, double x, double y, double z)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_MoveNode);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_MoveNode)
		{
			com = new SMESHDS_Command(SMESHDS_MoveNode);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->MoveNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveNode(int ID)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_RemoveNode);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_RemoveNode)
		{
			com = new SMESHDS_Command(SMESHDS_RemoveNode);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->RemoveNode(ID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveElement(int ElementID)
{
	SMESHDS_Command * com;
	if (myCommands.empty())
	{
		com = new SMESHDS_Command(SMESHDS_RemoveElement);
		myCommands.insert(myCommands.end(),com);
	}
	else
	{
		com = myCommands.back();
		if (com->GetType() != SMESHDS_RemoveElement)
		{
			com = new SMESHDS_Command(SMESHDS_RemoveElement);
			myCommands.insert(myCommands.end(),com);
		}
	}
	com->RemoveElement(ElementID);
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

/**
 * Add UpdateAll command to the log of this mesh. Once interpreted by the
 * graphical client it will (re)draw the full mesh.
 */
void SMESHDS_Script::UpdateAll()
{
	myCommands.insert(myCommands.end(), new SMESHDS_Command(SMESHDS_UpdateAll));
}
