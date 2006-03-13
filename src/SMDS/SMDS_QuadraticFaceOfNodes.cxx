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
//
//
//
// File:      SMDS_QuadraticFaceOfNodes.cxx
// Created:   16.01.06 17:12:58
// Author:    Sergey KUUL

#include "SMDS_QuadraticFaceOfNodes.hxx"

#include "SMDS_SetIterator.hxx"
#include "SMDS_IteratorOfElements.hxx"
#include "SMDS_MeshNode.hxx"

#include "utilities.h"

using namespace std;


//=======================================================================
//function : SMDS_QuadraticFaceOfNodes()
//purpose  : Constructor
//=======================================================================

SMDS_QuadraticFaceOfNodes::SMDS_QuadraticFaceOfNodes(const SMDS_MeshNode * n1,
                                                     const SMDS_MeshNode * n2,
                                                     const SMDS_MeshNode * n3,
                                                     const SMDS_MeshNode * n12,
                                                     const SMDS_MeshNode * n23,
                                                     const SMDS_MeshNode * n31)
{
  myNodes.resize( 6 );
  myNodes[ 0 ] = n1;
  myNodes[ 1 ] = n2;
  myNodes[ 2 ] = n3;
  myNodes[ 3 ] = n12;
  myNodes[ 4 ] = n23;
  myNodes[ 5 ] = n31;
}


//=======================================================================
//function : SMDS_QuadraticFaceOfNodes()
//purpose  : Constructor
//=======================================================================

SMDS_QuadraticFaceOfNodes::SMDS_QuadraticFaceOfNodes(const SMDS_MeshNode * n1,
                                                     const SMDS_MeshNode * n2,
                                                     const SMDS_MeshNode * n3,
                                                     const SMDS_MeshNode * n4,
                                                     const SMDS_MeshNode * n12,
                                                     const SMDS_MeshNode * n23,
                                                     const SMDS_MeshNode * n34,
                                                     const SMDS_MeshNode * n41)
{
  myNodes.resize( 8 );
  myNodes[ 0 ] = n1;
  myNodes[ 1 ] = n2;
  myNodes[ 2 ] = n3;
  myNodes[ 3 ] = n4;
  myNodes[ 4 ] = n12;
  myNodes[ 5 ] = n23;
  myNodes[ 6 ] = n34;
  myNodes[ 7 ] = n41;
}


//=======================================================================
//function : IsMediumNode
//purpose  : 
//=======================================================================

bool SMDS_QuadraticFaceOfNodes::IsMediumNode(const SMDS_MeshNode * node) const
{
  int i=NbNodes()/2;
  for(; i<NbNodes(); i++) {
    if(myNodes[i]==node) return true;
  }
  return false;
}


//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================

bool SMDS_QuadraticFaceOfNodes::ChangeNodes(const SMDS_MeshNode* nodes[],
                                            const int            nbNodes)
{
  if( nbNodes==6 || nbNodes==8 ) {
    myNodes.resize(nbNodes);
    int i=0;
    for(; i<nbNodes; i++) {
      myNodes[i] = nodes[i];
    }
    return true;
  }
  return false;
}


//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
int SMDS_QuadraticFaceOfNodes::NbNodes() const
{
  return myNodes.size();
}


//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================
int SMDS_QuadraticFaceOfNodes::NbEdges() const
{
  return NbNodes()/2;
}


//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================
int SMDS_QuadraticFaceOfNodes::NbFaces() const
{
  return 1;
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================
void SMDS_QuadraticFaceOfNodes::Print(ostream & OS) const
{
  OS << "quadratic face <" << GetID() << " > : ";
  int i, nbNodes = myNodes.size();
  for (i = 0; i < nbNodes - 1; i++)
    OS << myNodes[i] << ",";
  OS << myNodes[i] << ") " << endl;
}

namespace {

  //=======================================================================
  //class : _MyInterlacedNodeIterator
  //purpose  : 
  //=======================================================================

  class _MyInterlacedNodeIterator:public SMDS_NodeIterator
  {
    const vector<const SMDS_MeshNode *>& mySet;
    int myIndex;
    const int * myInterlace;
  public:
    _MyInterlacedNodeIterator(const vector<const SMDS_MeshNode *>& s,
                              const int * interlace):
      mySet(s),myIndex(0),myInterlace(interlace) {}

    bool more()
    {
      return myIndex < mySet.size();
    }

    const SMDS_MeshNode* next()
    {
      return mySet[ myInterlace[ myIndex++ ]];
    }
  };

  //=======================================================================
  //class : _MyInterlacedNodeElemIterator
  //purpose  : 
  //=======================================================================

  class _MyInterlacedNodeElemIterator : public SMDS_ElemIterator
  {
    SMDS_NodeIteratorPtr myItr;
  public:
    _MyInterlacedNodeElemIterator(SMDS_NodeIteratorPtr interlacedNodeItr):
      myItr( interlacedNodeItr ) {}
    bool more()                    { return myItr->more(); }
    const SMDS_MeshElement* next() { return myItr->next(); }
  };

  //=======================================================================
  //class : _MyNodeIterator
  //purpose  : 
  //=======================================================================

  class _MyNodeIterator : public SMDS_NodeVectorElemIterator
  {
  public:
    _MyNodeIterator(const vector<const SMDS_MeshNode *>& s):
      SMDS_NodeVectorElemIterator( s.begin(), s.end() ) {}
  };
  
}

//=======================================================================
//function : interlacedNodesIterator
//purpose  : 
//=======================================================================

SMDS_NodeIteratorPtr SMDS_QuadraticFaceOfNodes::interlacedNodesIterator() const
{
  static int triaInterlace [] = { 0, 3, 1, 4, 2, 5 };
  static int quadInterlace [] = { 0, 4, 1, 5, 2, 6, 3, 7 };
  return SMDS_NodeIteratorPtr
    (new _MyInterlacedNodeIterator (myNodes, myNodes.size()==6 ? triaInterlace : quadInterlace));
}

//=======================================================================
//function : interlacedNodesElemIterator
//purpose  : 
//=======================================================================

SMDS_ElemIteratorPtr SMDS_QuadraticFaceOfNodes::interlacedNodesElemIterator() const
{
  return SMDS_ElemIteratorPtr
    (new _MyInterlacedNodeElemIterator ( interlacedNodesIterator() ));
}

//=======================================================================
//function : elementsIterator
//purpose  : 
//=======================================================================

SMDS_ElemIteratorPtr SMDS_QuadraticFaceOfNodes::elementsIterator
                                         (SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Face:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Face);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new _MyNodeIterator(myNodes));
  case SMDSAbs_Edge:
    MESSAGE("Error : edge iterator for SMDS_QuadraticFaceOfNodes not implemented");
    break;
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr (new _MyNodeIterator(myNodes))));
  }
  return SMDS_ElemIteratorPtr();
}

/*!
 * \brief Return node by its index
 * \param ind - node index
 * \retval const SMDS_MeshNode* - the node
 * 
 * Index is wrapped if it is out of a valid range
 */
const SMDS_MeshNode* SMDS_QuadraticFaceOfNodes::GetNode(const int ind) const
{
  return myNodes[ WrappedIndex( ind )];
}

