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
//  File   : SMESHDS_Command.hxx
//  Module : SMESH

#ifndef _SMESHDS_Command_HeaderFile
#define _SMESHDS_Command_HeaderFile

#include "SMESHDS_CommandType.hxx"
#include <list>
using namespace std;

class SMESHDS_Command
{

  public:
	SMESHDS_Command(const SMESHDS_CommandType aType);
	void AddNode(int NewNodeID, double x, double y, double z);
	void AddEdge(int NewEdgeID, int idnode1, int idnode2);
	void AddFace(int NewFaceID, int idnode1, int idnode2, int idnode3);
	void AddFace(int NewFaceID, int idnode1, int idnode2, int idnode3,
		int idnode4);
	void AddVolume(int NewVolID, int idnode1, int idnode2, int idnode3,
		int idnode4);
	void AddVolume(int NewVolID, int idnode1, int idnode2, int idnode3,
		int idnode4, int idnode5);
	void AddVolume(int NewVolID, int idnode1, int idnode2, int idnode3,
		int idnode4, int idnode5, int idnode6);
	void AddVolume(int NewVolID, int idnode1, int idnode2, int idnode3,
		int idnode4, int idnode5, int idnode6, int idnode7, int idnode8);
	void MoveNode(int NewNodeID, double x, double y, double z);
	void RemoveNode(int NodeID);
	void RemoveElement(int ElementID);
	SMESHDS_CommandType GetType();
	int GetNumber();
	const list<int> & GetIndexes();
	const list<double> & GetCoords();
	 ~SMESHDS_Command();
  private:
	SMESHDS_CommandType myType;
	int myNumber;
	list<double> myReals;
	list<int> myIntegers;
};
#endif
