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

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_VolumeOfNodes.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
/// Create an hexahedron. node 1,2,3,4 and 5,6,7,8 are quadrangle and
/// 5,1 and 7,3 are an edges.
///////////////////////////////////////////////////////////////////////////////
SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		const SMDS_MeshNode * node1,
		const SMDS_MeshNode * node2,
		const SMDS_MeshNode * node3,
		const SMDS_MeshNode * node4,
		const SMDS_MeshNode * node5,
		const SMDS_MeshNode * node6,
		const SMDS_MeshNode * node7,
		const SMDS_MeshNode * node8)
{
	myNbNodes = 8;
	myNodes = new const SMDS_MeshNode* [myNbNodes];
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
		const SMDS_MeshNode * node1,
		const SMDS_MeshNode * node2,
		const SMDS_MeshNode * node3,
		const SMDS_MeshNode * node4)
{
	myNbNodes = 4;
	myNodes = new const SMDS_MeshNode* [myNbNodes];
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
}

SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		const SMDS_MeshNode * node1,
		const SMDS_MeshNode * node2,
		const SMDS_MeshNode * node3,
		const SMDS_MeshNode * node4,
		const SMDS_MeshNode * node5)
{
	myNbNodes = 5;
	myNodes = new const SMDS_MeshNode* [myNbNodes];
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
}

SMDS_VolumeOfNodes::SMDS_VolumeOfNodes(
		const SMDS_MeshNode * node1,
		const SMDS_MeshNode * node2,
		const SMDS_MeshNode * node3,
		const SMDS_MeshNode * node4,
		const SMDS_MeshNode * node5,
		const SMDS_MeshNode * node6)
{
	myNbNodes = 6;
	myNodes = new const SMDS_MeshNode* [myNbNodes];
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;
	myNodes[4]=node5;
	myNodes[5]=node6;
}

bool SMDS_VolumeOfNodes::ChangeNodes(const SMDS_MeshNode* nodes[],
                                     const int            nbNodes)
{
  if (nbNodes < 4 || nbNodes > 8 || nbNodes == 7)
    return false;

  delete [] myNodes;
  myNbNodes = nbNodes;
  myNodes = new const SMDS_MeshNode* [myNbNodes];
  for ( int i = 0; i < nbNodes; i++ )
    myNodes[ i ] = nodes [ i ];

  return true;
}

SMDS_VolumeOfNodes::~SMDS_VolumeOfNodes()
{
  if (myNodes != NULL) {
    delete [] myNodes;
    myNodes = NULL;
  }
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_VolumeOfNodes::Print(ostream & OS) const
{
	OS << "volume <" << GetID() << "> : ";
	int i;
	for (i = 0; i < NbNodes(); ++i) OS << myNodes[i] << ",";
	OS << myNodes[NbNodes()-1]<< ") " << endl;
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
        return 0;
}

int SMDS_VolumeOfNodes::NbNodes() const
{
	return myNbNodes;
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
        return 0;
}

class SMDS_VolumeOfNodes_MyIterator:public SMDS_ElemIterator
{
  const SMDS_MeshNode* const* mySet;
  int myLength;
  int index;
 public:
  SMDS_VolumeOfNodes_MyIterator(const SMDS_MeshNode* const* s, int l):
    mySet(s),myLength(l),index(0) {}

  bool more()
  {
    return index<myLength;
  }

  const SMDS_MeshElement* next()
  {
    index++;
    return mySet[index-1];
  }
};

SMDS_ElemIteratorPtr SMDS_VolumeOfNodes::
	elementsIterator(SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Volume:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Volume);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_VolumeOfNodes_MyIterator(myNodes,myNbNodes));
  default:
    MESSAGE("ERROR : Iterator not implemented");
    return SMDS_ElemIteratorPtr((SMDS_ElemIterator*)NULL);
  }
}

SMDSAbs_ElementType SMDS_VolumeOfNodes::GetType() const
{
	return SMDSAbs_Volume;
}
