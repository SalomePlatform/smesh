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
#include "SMDS_HexahedronOfNodes.hxx"
#include "SMDS_IteratorOfArray.hxx"
#include "SMDS_MeshNode.hxx"

///////////////////////////////////////////////////////////////////////////////
/// Create an hexahedron. node 1,2,3,4 and 5,6,7,8 are quadrangle and
/// 5,1 and 7,3 are an edges.
///////////////////////////////////////////////////////////////////////////////
SMDS_HexahedronOfNodes::SMDS_HexahedronOfNodes(
		const SMDS_MeshNode * node1,
		const SMDS_MeshNode * node2,
		const SMDS_MeshNode * node3,
		const SMDS_MeshNode * node4,
		const SMDS_MeshNode * node5,
		const SMDS_MeshNode * node6,
		const SMDS_MeshNode * node7,
		const SMDS_MeshNode * node8)
{
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
	myNodes[5]=node6;
	myNodes[6]=node7;
	myNodes[7]=node8;
}
//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_HexahedronOfNodes::Print(ostream & OS) const
{
	OS << "volume <" << GetID() << "> : ";
	int i;
	for (i = 0; i < 7; ++i) OS << myNodes[i] << ",";
	OS << myNodes[7]<< ") " << endl;
}

int SMDS_HexahedronOfNodes::NbFaces() const
{
	return 6;
}

int SMDS_HexahedronOfNodes::NbNodes() const
{
	return 8;
}

int SMDS_HexahedronOfNodes::NbEdges() const
{
	return 12;
}

SMDS_ElemIteratorPtr SMDS_HexahedronOfNodes::
	elementsIterator(SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Volume:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Volume);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr (new SMDS_IteratorOfArray<const SMDS_MeshElement *, 8,
                                 const SMDS_MeshNode*>(myNodes));
  default: MESSAGE("ERROR : Iterator not implemented");
  }
}

SMDSAbs_ElementType SMDS_HexahedronOfNodes::GetType() const
{
	return SMDSAbs_Volume;
}
