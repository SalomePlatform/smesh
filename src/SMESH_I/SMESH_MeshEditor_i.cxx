//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"

#include "utilities.h"

#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(SMESHDS_Mesh* theMesh)
{
	_myMeshDS = theMesh;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveElements(const SMESH::
	long_array & IDsOfElements)
{
	for (int i = 0; i < IDsOfElements.length(); i++)
	{
		CORBA::Long index = IDsOfElements[i];
		_myMeshDS->RemoveElement(index);
		MESSAGE("Element " << index << " was removed")
	}
	return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::
	long_array & IDsOfNodes)
{
	// It's nodes' turn to die
	for (int i = 0; i < IDsOfNodes.length(); i++)
	{
		const SMDS_MeshNode * node=_myMeshDS->FindNode(IDsOfNodes[i]);
		if(node==NULL)
		{
			MESSAGE("SMESH_MeshEditor_i::RemoveNodes: Node "<<IDsOfNodes[i]
				<<" not found");
			continue;
		}
		_myMeshDS->RemoveNode(IDsOfNodes[i]);
		MESSAGE("Node " << index << " was removed")
	}
	return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddEdge(const SMESH::long_array & IDsOfNodes)
{
	int NbNodes = IDsOfNodes.length();
	if (NbNodes == 2)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		_myMeshDS->AddEdge(index1, index2);
	}
	return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddNode(CORBA::Double x,
	CORBA::Double y, CORBA::Double z)
{
	MESSAGE(" AddNode " << x << " , " << y << " , " << z)
		int idNode = _myMeshDS->AddNode(x, y, z)->GetID();
	MESSAGE(" idNode " << idNode) return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddFace(const SMESH::long_array & IDsOfNodes)
{
	int NbNodes = IDsOfNodes.length();
	if (NbNodes == 3)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		_myMeshDS->AddFace(index1, index2, index3);
	}
	else if (NbNodes == 4)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		CORBA::Long index4 = IDsOfNodes[3];
		_myMeshDS->AddFace(index1, index2, index3, index4);
	}
	return true;
};

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::AddVolume(const SMESH::
	long_array & IDsOfNodes)
{
	int NbNodes = IDsOfNodes.length();
	if (NbNodes == 4)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		CORBA::Long index4 = IDsOfNodes[3];
		_myMeshDS->AddVolume(index1, index2, index3, index4);
	}
	else if (NbNodes == 5)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		CORBA::Long index4 = IDsOfNodes[3];
		CORBA::Long index5 = IDsOfNodes[4];
		_myMeshDS->AddVolume(index1, index2, index3, index4, index5);
	}
	else if (NbNodes == 6)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		CORBA::Long index4 = IDsOfNodes[3];
		CORBA::Long index5 = IDsOfNodes[4];
		CORBA::Long index6 = IDsOfNodes[5];
		_myMeshDS->AddVolume(index1, index2, index3, index4, index5, index6);
	}
	else if (NbNodes == 8)
	{
		CORBA::Long index1 = IDsOfNodes[0];
		CORBA::Long index2 = IDsOfNodes[1];
		CORBA::Long index3 = IDsOfNodes[2];
		CORBA::Long index4 = IDsOfNodes[3];
		CORBA::Long index5 = IDsOfNodes[4];
		CORBA::Long index6 = IDsOfNodes[5];
		CORBA::Long index7 = IDsOfNodes[6];
		CORBA::Long index8 = IDsOfNodes[7];
		_myMeshDS->AddVolume(index1, index2, index3, index4, index5, index6,
			index7, index8);
	}
	return true;
};
