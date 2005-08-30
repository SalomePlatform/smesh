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

#include "SMDS_PolyhedralVolumeOfNodes.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

#include <set>

using namespace std;

//=======================================================================
//function : Constructor
//purpose  : Create a volume of many faces
//=======================================================================
SMDS_PolyhedralVolumeOfNodes::SMDS_PolyhedralVolumeOfNodes
                                (std::vector<const SMDS_MeshNode *> nodes,
                                 std::vector<int>                   quantities)
: SMDS_VolumeOfNodes(NULL, NULL, NULL, NULL)
{
  ChangeNodes(nodes, quantities);
}

//=======================================================================
//function : GetType
//purpose  : 
//=======================================================================
SMDSAbs_ElementType SMDS_PolyhedralVolumeOfNodes::GetType() const
{
//  return SMDSAbs_PolyhedralVolume;
  return SMDSAbs_Volume;
}

//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================
bool SMDS_PolyhedralVolumeOfNodes::ChangeNodes (std::vector<const SMDS_MeshNode *> nodes,
                                                std::vector<int>                   quantities)
{
  myNodesByFaces = nodes;
  myQuantities = quantities;

  // Init fields of parent class
  int aNbNodes = 0;
  std::set<const SMDS_MeshNode *> aSet;
  int nodes_len = nodes.size();
  for (int j = 0; j < nodes_len; j++) {
    if (aSet.find(nodes[j]) == aSet.end()) {
      aSet.insert(nodes[j]);
      aNbNodes++;
    }
  }

  int k = 0;
#ifndef WNT
  const SMDS_MeshNode* aNodes [aNbNodes];
#else
  const SMDS_MeshNode** aNodes = (const SMDS_MeshNode **)new SMDS_MeshNode*[aNbNodes];
#endif
  std::set<const SMDS_MeshNode *>::iterator anIter = aSet.begin();
  for (; anIter != aSet.end(); anIter++, k++) {
    aNodes[k] = *anIter;
  }

  //SMDS_VolumeOfNodes::ChangeNodes(aNodes, aNbNodes);
  delete [] myNodes;
  //myNbNodes = nodes.size();
  myNbNodes = aNbNodes;
  myNodes = new const SMDS_MeshNode* [myNbNodes];
  for (int i = 0; i < myNbNodes; i++) {
    //myNodes[i] = nodes[i];
    myNodes[i] = aNodes[i];
  }

#ifdef WNT
  delete [] aNodes;
#endif

  return true;
}

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================
int SMDS_PolyhedralVolumeOfNodes::NbEdges() const
{
  int nbEdges = 0;

  for (int ifa = 0; ifa < myQuantities.size(); ifa++) {
    nbEdges += myQuantities[ifa];
  }
  nbEdges /= 2;

  return nbEdges;
}

//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================
int SMDS_PolyhedralVolumeOfNodes::NbFaces() const
{
  return myQuantities.size();
}

//=======================================================================
//function : NbFaceNodes
//purpose  : 
//=======================================================================
int SMDS_PolyhedralVolumeOfNodes::NbFaceNodes (const int face_ind) const
{
  if (face_ind < 1 || myQuantities.size() < face_ind)
    return 0;
  return myQuantities[face_ind - 1];
}

//=======================================================================
//function : GetFaceNode
//purpose  : 
//=======================================================================
const SMDS_MeshNode* SMDS_PolyhedralVolumeOfNodes::GetFaceNode (const int face_ind,
                                                                const int node_ind) const
{
  if (node_ind < 1 || NbFaceNodes(face_ind) < node_ind)
    return NULL;

  int i, first_node = 0;
  for (i = 0; i < face_ind - 1; i++) {
    first_node += myQuantities[i];
  }

  return myNodesByFaces[first_node + node_ind - 1];
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================
void SMDS_PolyhedralVolumeOfNodes::Print (ostream & OS) const
{
  OS << "polyhedral volume <" << GetID() << "> : ";

  int faces_len = myQuantities.size();
  //int nodes_len = myNodesByFaces.size();
  int cur_first_node = 0;

  int i, j;
  for (i = 0; i < faces_len; i++) {
    OS << "face_" << i << " (";
    for (j = 0; j < myQuantities[i] - 1; j++) {
      OS << myNodesByFaces[cur_first_node + j] << ",";
    }
    OS << myNodesByFaces[cur_first_node + j] << ") ";
    cur_first_node += myQuantities[i];
  }
}

//=======================================================================
//function : ChangeNodes
//purpose  : usage disabled
//=======================================================================
bool SMDS_PolyhedralVolumeOfNodes::ChangeNodes (const SMDS_MeshNode* nodes[],
                                                const int            nbNodes)
{
  return false;
}
