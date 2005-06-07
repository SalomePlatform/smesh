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

#include "SMDS_PolygonalFaceOfNodes.hxx"

#include "SMDS_IteratorOfElements.hxx"
//#include "SMDS_MeshNode.hxx"
#include "utilities.h"

using namespace std;

//=======================================================================
//function : Constructor
//purpose  : 
//=======================================================================
SMDS_PolygonalFaceOfNodes::SMDS_PolygonalFaceOfNodes
                          (std::vector<const SMDS_MeshNode *> nodes)
{
  myNodes = nodes;
}

//=======================================================================
//function : GetType
//purpose  : 
//=======================================================================
SMDSAbs_ElementType SMDS_PolygonalFaceOfNodes::GetType() const
{
  return SMDSAbs_Face;
  //return SMDSAbs_PolygonalFace;
}

//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================
bool SMDS_PolygonalFaceOfNodes::ChangeNodes (std::vector<const SMDS_MeshNode *> nodes)
{
  if (nodes.size() < 3)
    return false;

  myNodes = nodes;

  return true;
}

//=======================================================================
//function : ChangeNodes
//purpose  : to support the same interface, as SMDS_FaceOfNodes
//=======================================================================
bool SMDS_PolygonalFaceOfNodes::ChangeNodes (const SMDS_MeshNode* nodes[],
                                             const int            nbNodes)
{
  if (nbNodes < 3)
    return false;

  myNodes.resize(nbNodes);
  int i = 0;
  for (; i < nbNodes; i++) {
    myNodes[i] = nodes[i];
  }

  return true;
}

//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
int SMDS_PolygonalFaceOfNodes::NbNodes() const
{
  return myNodes.size();
}

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================
int SMDS_PolygonalFaceOfNodes::NbEdges() const
{
  return NbNodes();
}

//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================
int SMDS_PolygonalFaceOfNodes::NbFaces() const
{
  return 1;
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================
void SMDS_PolygonalFaceOfNodes::Print(ostream & OS) const
{
  OS << "polygonal face <" << GetID() << " > : ";
  int i, nbNodes = myNodes.size();
  for (i = 0; i < nbNodes - 1; i++)
    OS << myNodes[i] << ",";
  OS << myNodes[i] << ") " << endl;
}

//=======================================================================
//function : elementsIterator
//purpose  : 
//=======================================================================
class SMDS_PolygonalFaceOfNodes_MyIterator:public SMDS_ElemIterator
{
  //const SMDS_MeshNode* const *mySet;
  const std::vector<const SMDS_MeshNode *> mySet;
  //int myLength;
  int index;
 public:
  //SMDS_PolygonalFaceOfNodes_MyIterator(const SMDS_MeshNode* const *s, int l):
  //  mySet(s),myLength(l),index(0) {}
  SMDS_PolygonalFaceOfNodes_MyIterator(const std::vector<const SMDS_MeshNode *> s):
    mySet(s),index(0) {}

  bool more()
  {
    return index < mySet.size();
  }

  const SMDS_MeshElement* next()
  {
    index++;
    return mySet[index-1];
  }
};

SMDS_ElemIteratorPtr SMDS_PolygonalFaceOfNodes::elementsIterator
                                         (SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Face:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Face);
  case SMDSAbs_Node:
    return SMDS_ElemIteratorPtr(new SMDS_PolygonalFaceOfNodes_MyIterator(myNodes));
  case SMDSAbs_Edge:
    MESSAGE("Error : edge iterator for SMDS_PolygonalFaceOfNodes not implemented");
    break;
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr
        (new SMDS_PolygonalFaceOfNodes_MyIterator(myNodes))));
  }
  return SMDS_ElemIteratorPtr();
}
