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
//  File   : SMESH_SubMesh.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_SubMesh.hxx"

//=======================================================================
//function : SMESHDS_SubMesh
//purpose  : 
//=======================================================================
SMESHDS_SubMesh::SMESHDS_SubMesh(const SMDS_Mesh * M):myMesh(M)
{
	myListOfEltIDIsUpdate = false;
	myListOfNodeIDIsUpdate = false;
}

//=======================================================================
//function : AddElement
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddElement(const SMDS_MeshElement * ME)
{
	myElements.insert(ME);
	myListOfEltIDIsUpdate = false;
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::RemoveElement(const SMDS_MeshElement * ME)
{
	myElements.erase(ME);
	myListOfEltIDIsUpdate = false;
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddNode(const SMDS_MeshNode * N)
{
	myNodes.insert(N);
	myListOfNodeIDIsUpdate = false;
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::RemoveNode(const SMDS_MeshNode * N)
{
	myNodes.erase(N);
	myListOfNodeIDIsUpdate = false;
}

//=======================================================================
//function : NbElements
//purpose  : 
//=======================================================================
int SMESHDS_SubMesh::NbElements() const
{
	return myElements.size();
}

//=======================================================================
//function : GetElements
//purpose  : 
//=======================================================================
const set<const SMDS_MeshElement*> & SMESHDS_SubMesh::GetElements()
{
	return myElements;
}

//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
int SMESHDS_SubMesh::NbNodes() const
{
	return myNodes.size();
}

//=======================================================================
//function : GetNodes
//purpose  : 
//=======================================================================
const set<const SMDS_MeshNode*> & SMESHDS_SubMesh::GetNodes() const
{
	return myNodes;
}

//=======================================================================
//function : GetIDElements
//purpose  : 
//=======================================================================
const vector<int> & SMESHDS_SubMesh::GetIDElements()
{
	if (!myListOfEltIDIsUpdate)
	{
		myListOfEltID.clear();
		set<const SMDS_MeshElement*>::iterator it=myElements.begin();
		for (; it!=myElements.end(); it++)
		{
			myListOfEltID.push_back((*it)->GetID());
		}
		myListOfEltIDIsUpdate = true;
	}
	return myListOfEltID;
}

//=======================================================================
//function : GetIDNodes
//purpose  : 
//=======================================================================
const vector<int> & SMESHDS_SubMesh::GetIDNodes()
{
	if (!myListOfNodeIDIsUpdate)
	{
		myListOfNodeID.clear();
		set<const SMDS_MeshNode*>::iterator it=myNodes.begin();
		for (; it!=myNodes.end(); it++)
		{
			myListOfNodeID.push_back((*it)->GetID());
		}
		myListOfNodeIDIsUpdate = true;
	}
	return myListOfNodeID;
}
