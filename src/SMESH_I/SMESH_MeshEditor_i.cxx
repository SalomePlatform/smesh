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
                const SMDS_MeshElement * elem = _myMeshDS->FindElement(index);
                // an element may be removed as a result of preceding
                // loop removal
                if ( elem )
                {
                  _myMeshDS->RemoveElement( elem );
                  MESSAGE("Element " << index << " was removed");
                }
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
		_myMeshDS->RemoveNode(node);
		MESSAGE("Node " << IDsOfNodes[i] << " was removed")
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
		_myMeshDS->AddEdge(_myMeshDS->FindNode(index1), _myMeshDS->FindNode(index2));
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
	const SMDS_MeshNode* nodes[4];
	for(int i=0;i<NbNodes;i++) nodes[i]=_myMeshDS->FindNode(IDsOfNodes[i]);
	if (NbNodes == 3)
	{
		_myMeshDS->AddFace(nodes[0], nodes[1], nodes[2]);
	}
	else if (NbNodes == 4)
	{
		_myMeshDS->AddFace(nodes[0], nodes[1], nodes[2], nodes[3]);
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
	const SMDS_MeshNode* n[8];
	for(int i=0;i<NbNodes;i++) n[i]=_myMeshDS->FindNode(IDsOfNodes[i]);

	switch(NbNodes)
	{
	case 4:_myMeshDS->AddVolume(n[0],n[1],n[2],n[3]); break;
	case 5:_myMeshDS->AddVolume(n[0],n[1],n[2],n[3],n[4]); break;
	case 6:_myMeshDS->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5]); break;
	case 8:_myMeshDS->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]); break;
	}
	return true;
};
