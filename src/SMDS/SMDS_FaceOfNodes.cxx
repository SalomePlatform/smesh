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

#include "SMDS_FaceOfNodes.hxx"
#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

using namespace std;

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

int SMDS_FaceOfNodes::NbEdges() const
{
	return NbNodes();
}

int SMDS_FaceOfNodes::NbFaces() const
{
	return 1;
}

int SMDS_FaceOfNodes::NbNodes() const
{
	return myNbNodes;
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_FaceOfNodes::Print(ostream & OS) const
{
	OS << "face <" << GetID() << " > : ";
	int i;
	for (i = 0; i < NbNodes() - 1; i++) OS << myNodes[i] << ",";
	OS << myNodes[i] << ") " << endl;
}

//=======================================================================
//function : elementsIterator
//purpose  : 
//=======================================================================

class SMDS_FaceOfNodes_MyIterator:public SMDS_ElemIterator
{
  const SMDS_MeshNode* const *mySet;
  int myLength;
  int index;
 public:
  SMDS_FaceOfNodes_MyIterator(const SMDS_MeshNode* const *s, int l):
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

SMDS_ElemIteratorPtr SMDS_FaceOfNodes::elementsIterator
                         (SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Face:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Face);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_FaceOfNodes_MyIterator(myNodes,myNbNodes));
  case SMDSAbs_Edge:
    MESSAGE("Error : edge iterator for SMDS_FaceOfNodes not implemented");
    break;
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr
        (new SMDS_FaceOfNodes_MyIterator(myNodes,myNbNodes))));
  }
  return SMDS_ElemIteratorPtr();
}

SMDS_FaceOfNodes::SMDS_FaceOfNodes(const SMDS_MeshNode* node1,
                                   const SMDS_MeshNode* node2,
                                   const SMDS_MeshNode* node3)
{
	myNbNodes = 3;
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=0;
}

SMDS_FaceOfNodes::SMDS_FaceOfNodes(const SMDS_MeshNode* node1,
                                   const SMDS_MeshNode* node2,
                                   const SMDS_MeshNode* node3,
                                   const SMDS_MeshNode* node4)
{
	myNbNodes = 4;
	myNodes[0]=node1;
	myNodes[1]=node2;
	myNodes[2]=node3;
	myNodes[3]=node4;	
}
bool SMDS_FaceOfNodes::ChangeNodes(const SMDS_MeshNode* nodes[],
                                   const int            nbNodes)
{
  myNbNodes = nbNodes;
  myNodes[0]=nodes[0];
  myNodes[1]=nodes[1];
  myNodes[2]=nodes[2];
  if (nbNodes == 4)
    myNodes[3]=nodes[3];
  else if (nbNodes != 3)
    return false;

  return true;
}

/*bool operator<(const SMDS_FaceOfNodes& f1, const SMDS_FaceOfNodes& f2)
{
	set<SMDS_MeshNode> set1,set2;
	SMDS_ElemIteratorPtr it;
	const SMDS_MeshNode * n;

	it=f1.nodesIterator();

	while(it->more())
	{
		n=static_cast<const SMDS_MeshNode *>(it->next());
		set1.insert(*n);
	}

	delete it;
	it=f2.nodesIterator();
	
	while(it->more())
	{	
		n=static_cast<const SMDS_MeshNode *>(it->next());
		set2.insert(*n);
	}

	delete it;
	return set1<set2;	

}*/

