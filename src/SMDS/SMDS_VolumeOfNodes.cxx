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

#include "utilities.h"
#include "SMDS_VolumeOfNodes.hxx"
#include "SMDS_MeshNode.hxx"

///////////////////////////////////////////////////////////////////////////////
/// Create an hexahedron. node 1,2,3,4 and 5,6,7,8 are quadrangle and
/// 5,1 and 7,3 are an edges.
///////////////////////////////////////////////////////////////////////////////
SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		SMDS_MeshNode * node1,
		SMDS_MeshNode * node2,
		SMDS_MeshNode * node3,
		SMDS_MeshNode * node4,
		SMDS_MeshNode * node5,
		SMDS_MeshNode * node6,
		SMDS_MeshNode * node7,
		SMDS_MeshNode * node8)
{
	myNodes.resize(8);
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
	myNodes[5]=node6;
	myNodes[6]=node7;
	myNodes[7]=node8;
}

SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		SMDS_MeshNode * node1,
		SMDS_MeshNode * node2,
		SMDS_MeshNode * node3,
		SMDS_MeshNode * node4)
{
	myNodes.resize(4);
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
}

SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		SMDS_MeshNode * node1,
		SMDS_MeshNode * node2,
		SMDS_MeshNode * node3,
		SMDS_MeshNode * node4,
		SMDS_MeshNode * node5)
{
	myNodes.resize(5);
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
}

SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		SMDS_MeshNode * node1,
		SMDS_MeshNode * node2,
		SMDS_MeshNode * node3,
		SMDS_MeshNode * node4,
		SMDS_MeshNode * node5,
		SMDS_MeshNode * node6)
{
	myNodes.resize(6);
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
	myNodes[5]=node6;
}
//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_VolumeOfNodes::Print(ostream & OS) const
{
	OS << "volume <" << GetID() << "> : ";
	int i;
	for (i = 0; i < 7; ++i) OS << myNodes[i] << ",";
	OS << myNodes[7]<< ") " << endl;
}

int SMDS_VolumeOfNodes::NbFaces() const
{
	switch(NbNodes())
	{
	case 4: return 4;
	case 5: return 5;
	case 6: return 5;
	case 8: return 6;
	default: MESSAGE("invalid number of nodes");
	}
}

int SMDS_VolumeOfNodes::NbNodes() const
{
	return myNodes.size();
}

int SMDS_VolumeOfNodes::NbEdges() const
{
	switch(NbNodes())
	{
	case 4: return 6;
	case 5: return 8;
	case 6: return 9;
	case 8: return 12;
	default: MESSAGE("invalid number of nodes");
	}
}

SMDS_Iterator<const SMDS_MeshElement *> * SMDS_VolumeOfNodes::
	elementsIterator(SMDSAbs_ElementType type) const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshElement*>
	{
		const vector<const SMDS_MeshNode*>& mySet;
		int index;
	  public:
		MyIterator(const vector<const SMDS_MeshNode*>& s):mySet(s),index(0)
		{}

		bool more()
		{
			return index<mySet.size();
		}

		const SMDS_MeshElement* next()
		{
			index++;
			return mySet[index-1];
		}	
	};
	switch(type)
	{
	case SMDSAbs_Volume:return SMDS_MeshElement::elementsIterator(SMDSAbs_Volume);
	case SMDSAbs_Node:return new MyIterator(myNodes);
	default: MESSAGE("ERROR : Iterator not implemented");
	}
}

SMDSAbs_ElementType SMDS_VolumeOfNodes::GetType() const
{
	return SMDSAbs_Volume;
}
