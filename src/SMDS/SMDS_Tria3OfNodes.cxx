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

#include "SMDS_Tria3OfNodes.hxx"
#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

int SMDS_Tria3OfNodes::NbEdges() const
{
	return 3;
}

int SMDS_Tria3OfNodes::NbFaces() const
{
	return 3;
}

int SMDS_Tria3OfNodes::NbNodes() const
{
	return 3;
}
//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_Tria3OfNodes::Print(ostream & OS) const
{
	OS << "face <" << GetID() << " > : ";
	int i;
	for (i = 0; i < NbNodes() - 1; i++) OS << myNodes[i] << ",";
	OS << myNodes[i] << ") " << endl;
}

SMDS_Iterator<const SMDS_MeshElement *> * SMDS_Tria3OfNodes::
	elementsIterator(SMDSAbs_ElementType type) const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshElement*>
	{
		const SMDS_MeshNode * const* mySet;
		int index;
	  public:
		MyIterator(const SMDS_MeshNode * const* s):mySet(s),index(0)
		{}

		bool more()
		{
			return index<3;
		}

		const SMDS_MeshElement* next()
		{
			index++;
			return mySet[index-1];
		}	
	};

	switch(type)
	{
	case SMDSAbs_Face:return SMDS_MeshElement::elementsIterator(SMDSAbs_Face);
	case SMDSAbs_Node:return new MyIterator(myNodes);
	case SMDSAbs_Edge:
		MESSAGE("Error : edge iterator for SMDS_FaceOfNodes not implemented");
		break;
	default:return new SMDS_IteratorOfElements(this,type,new MyIterator(myNodes));
	}
}

SMDS_Tria3OfNodes::SMDS_Tria3OfNodes(SMDS_MeshNode* node1, SMDS_MeshNode* node2,
	SMDS_MeshNode* node3)
{
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
}


