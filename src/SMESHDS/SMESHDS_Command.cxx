// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESHDS : management of mesh data and SMESH document
//  File   : SMESH_Command.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//
#include "SMESHDS_Command.hxx"

#include "utilities.h"

using namespace std;

//=======================================================================
//function : Constructor
//purpose  : 
//=======================================================================
SMESHDS_Command::SMESHDS_Command(const SMESHDS_CommandType aType):myType(aType),
myNumber(0)
{
}

//=======================================================================
//function : Destructor
//purpose  : 
//=======================================================================
SMESHDS_Command::~SMESHDS_Command()
{
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddNode(smIdType NewNodeID, double x, double y, double z)
{
        if ( myType != SMESHDS_AddNode)
        {
                MESSAGE("SMESHDS_Command::AddNode : Bad Type");
                return;
        }
        myIntegers.push_back(NewNodeID);
        myReals.push_back(x);
        myReals.push_back(y);
        myReals.push_back(z);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::MoveNode(smIdType NodeID, double x, double y, double z)
{
        if ( myType != SMESHDS_MoveNode)
        {
                MESSAGE("SMESHDS_Command::MoveNode : Bad Type");
                return;
        }
        myIntegers.push_back(NodeID);
        myReals.push_back(x);
        myReals.push_back(y);
        myReals.push_back(z);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::Add0DElement(smIdType New0DElementID, smIdType idnode)
{
  if ( myType != SMESHDS_Add0DElement)
  {
    MESSAGE("SMESHDS_Command::Add0DElement : Bad Type");
    return;
  }
  myIntegers.push_back(New0DElementID);
  myIntegers.push_back(idnode);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddEdge(smIdType NewEdgeID, smIdType idnode1, smIdType idnode2)
{
        if ( myType != SMESHDS_AddEdge)
        {
                MESSAGE("SMESHDS_Command::AddEdge : Bad Type");
                return;
        }
        myIntegers.push_back(NewEdgeID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
        smIdType idnode1, smIdType idnode2, smIdType idnode3)
{
        if ( myType != SMESHDS_AddTriangle)
        {
                MESSAGE("SMESHDS_Command::AddFace : Bad Type");
                return;
        }
        myIntegers.push_back(NewFaceID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
        smIdType idnode1, smIdType idnode2, smIdType idnode3, smIdType idnode4)
{
        if ( myType != SMESHDS_AddQuadrangle)
        {
                MESSAGE("SMESHDS_Command::AddFace : Bad Type");
                return;
        }
        myIntegers.push_back(NewFaceID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myIntegers.push_back(idnode4);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID,
        smIdType idnode1, smIdType idnode2, smIdType idnode3, smIdType idnode4)
{
        if ( myType != SMESHDS_AddTetrahedron)
        {
                MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
                return;
        }
        myIntegers.push_back(NewVolID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myIntegers.push_back(idnode4);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID,
        smIdType idnode1, smIdType idnode2, smIdType idnode3, smIdType idnode4, smIdType idnode5)
{
        if ( myType != SMESHDS_AddPyramid)
        {
                MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
                return;
        }
        myIntegers.push_back(NewVolID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myIntegers.push_back(idnode4);
        myIntegers.push_back(idnode5);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID,
        smIdType idnode1,
        smIdType idnode2, smIdType idnode3, smIdType idnode4, smIdType idnode5, smIdType idnode6)
{
        if ( myType != SMESHDS_AddPrism)
        {
                MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
                return;
        }
        myIntegers.push_back(NewVolID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myIntegers.push_back(idnode4);
        myIntegers.push_back(idnode5);
        myIntegers.push_back(idnode6);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID,
        smIdType idnode1,
        smIdType idnode2,
        smIdType idnode3,
        smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8)
{
        if ( myType != SMESHDS_AddHexahedron)
        {
                MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
                return;
        }
        myIntegers.push_back(NewVolID);
        myIntegers.push_back(idnode1);
        myIntegers.push_back(idnode2);
        myIntegers.push_back(idnode3);
        myIntegers.push_back(idnode4);
        myIntegers.push_back(idnode5);
        myIntegers.push_back(idnode6);
        myIntegers.push_back(idnode7);
        myIntegers.push_back(idnode8);
        myNumber++;
}

void SMESHDS_Command::AddVolume(smIdType NewVolID,
                                smIdType idnode1,smIdType idnode2,smIdType idnode3,smIdType idnode4,
                                smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8,
                                smIdType idnode9, smIdType idnode10, smIdType idnode11, smIdType idnode12)
{
  if (myType != SMESHDS_AddHexagonalPrism)
  {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(idnode1);
  myIntegers.push_back(idnode2);
  myIntegers.push_back(idnode3);
  myIntegers.push_back(idnode4);
  myIntegers.push_back(idnode5);
  myIntegers.push_back(idnode6);
  myIntegers.push_back(idnode7);
  myIntegers.push_back(idnode8);
  myIntegers.push_back(idnode9);
  myIntegers.push_back(idnode10);
  myIntegers.push_back(idnode11);
  myIntegers.push_back(idnode12);
  myNumber++;
}

//=======================================================================
//function : AddPolygonalFace
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddPolygonalFace (const smIdType               ElementID,
                                        const std::vector<smIdType>& nodes_ids)
{
  if ( myType != SMESHDS_AddPolygon) {
    MESSAGE("SMESHDS_Command::AddPolygonalFace : Bad Type");
    return;
  }
  myIntegers.push_back(ElementID);

  smIdType i, nbNodes = nodes_ids.size();
  myIntegers.push_back(nbNodes);
  for (i = 0; i < nbNodes; i++) {
    myIntegers.push_back(nodes_ids[i]);
  }

  myNumber++;
}

//=======================================================================
//function : AddQuadPolygonalFace
//purpose  :
//=======================================================================
void SMESHDS_Command::AddQuadPolygonalFace (const smIdType               ElementID,
                                            const std::vector<smIdType>& nodes_ids)
{
  if ( myType != SMESHDS_AddQuadPolygon) {
    MESSAGE("SMESHDS_Command::AddQuadraticPolygonalFace : Bad Type");
    return;
  }
  myIntegers.push_back(ElementID);

  smIdType i, nbNodes = nodes_ids.size();
  myIntegers.push_back(nbNodes);
  for (i = 0; i < nbNodes; i++) {
    myIntegers.push_back(nodes_ids[i]);
  }

  myNumber++;
}

//=======================================================================
//function : AddPolyhedralVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddPolyhedralVolume (const smIdType               ElementID,
                                           const std::vector<smIdType>& nodes_ids,
                                           const std::vector<int>&      quantities)
{
  if ( myType != SMESHDS_AddPolyhedron) {
    MESSAGE("SMESHDS_Command::AddPolyhedralVolume : Bad Type");
    return;
  }
  myIntegers.push_back(ElementID);

  smIdType i, nbNodes = nodes_ids.size();
  myIntegers.push_back(nbNodes);
  for (i = 0; i < nbNodes; i++) {
    myIntegers.push_back(nodes_ids[i]);
  }

  smIdType nbFaces = quantities.size();
  myIntegers.push_back(nbFaces);
  for (i = 0; i < nbFaces; i++) {
    myIntegers.push_back(quantities[i]);
  }

  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::RemoveNode(smIdType NodeID)
{
        if ( myType != SMESHDS_RemoveNode)
        {
                MESSAGE("SMESHDS_Command::RemoveNode : Bad Type");
                return;
        }
        myIntegers.push_back(NodeID);
        myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Command::RemoveElement(smIdType ElementID)
{
        if ( myType != SMESHDS_RemoveElement)
        {
                MESSAGE("SMESHDS_Command::RemoveElement : Bad Type");
                return;
        }
        myIntegers.push_back(ElementID);
        myNumber++;
}

//=======================================================================
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

void SMESHDS_Command::ChangeElementNodes(smIdType ElementID, smIdType nodes[], smIdType nbnodes)
{
  if ( myType != SMESHDS_ChangeElementNodes)
  {
    MESSAGE("SMESHDS_Command::ChangeElementNodes : Bad Type");
    return;
  }
  myIntegers.push_back(ElementID);
  myIntegers.push_back(nbnodes);
  for ( smIdType i = 0; i < nbnodes; i++ )
    myIntegers.push_back( nodes[ i ] );

  myNumber++;
}

//=======================================================================
//function : ChangePolyhedronNodes
//purpose  : 
//=======================================================================
void SMESHDS_Command::ChangePolyhedronNodes (const smIdType               ElementID,
                                             const std::vector<smIdType>& nodes_ids,
                                             const std::vector<int>& quantities)
{
  if (myType != SMESHDS_ChangePolyhedronNodes)
  {
    MESSAGE("SMESHDS_Command::ChangePolyhedronNodes : Bad Type");
    return;
  }
  myIntegers.push_back(ElementID);

  smIdType i, nbNodes = nodes_ids.size();
  myIntegers.push_back(nbNodes);
  for (i = 0; i < nbNodes; i++) {
    myIntegers.push_back(nodes_ids[i]);
  }

  int nbFaces = quantities.size();
  myIntegers.push_back(nbFaces);
  for (i = 0; i < nbFaces; i++) {
    myIntegers.push_back(quantities[i]);
  }

  myNumber++;
}

//=======================================================================
//function : Renumber
//purpose  : 
//=======================================================================

void SMESHDS_Command::Renumber (const bool isNodes, const smIdType startID, const smIdType deltaID)
{
  if ( myType != SMESHDS_Renumber)
  {
    MESSAGE("SMESHDS_Command::Renumber : Bad Type");
    return;
  }
  myIntegers.push_back(isNodes);
  myIntegers.push_back(startID);
  myIntegers.push_back(deltaID);
  myNumber++;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
SMESHDS_CommandType SMESHDS_Command::GetType()
{
        return myType;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
smIdType SMESHDS_Command::GetNumber()
{
        return myNumber;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const list < smIdType >&SMESHDS_Command::GetIndexes()
{
        return myIntegers;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const list < double >&SMESHDS_Command::GetCoords()
{
        return myReals;
}


//********************************************************************
//*****             Methods for quadratic elements              ******
//********************************************************************

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddEdge(smIdType NewEdgeID, smIdType n1, smIdType n2, smIdType n12)
{
  if ( myType != SMESHDS_AddQuadEdge) {
    MESSAGE("SMESHDS_Command::AddEdge : Bad Type");
    return;
  }
  myIntegers.push_back(NewEdgeID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n12);
  myNumber++;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
                              smIdType n1, smIdType n2, smIdType n3,
                              smIdType n12, smIdType n23, smIdType n31)
{
  if ( myType != SMESHDS_AddQuadTriangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");
    return;
  }
  myIntegers.push_back(NewFaceID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n31);
  myNumber++;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
                              smIdType n1, smIdType n2, smIdType n3,
                              smIdType n12, smIdType n23, smIdType n31, smIdType nCenter)
{
  if ( myType != SMESHDS_AddBiQuadTriangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");
    return;
  }
  myIntegers.push_back(NewFaceID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n31);
  myIntegers.push_back(nCenter);
  myNumber++;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
                              smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                              smIdType n12, smIdType n23, smIdType n34, smIdType n41)
{
  if ( myType != SMESHDS_AddQuadQuadrangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");
    return;
  }
  myIntegers.push_back(NewFaceID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n34);
  myIntegers.push_back(n41);
  myNumber++;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddFace(smIdType NewFaceID,
                              smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                              smIdType n12, smIdType n23, smIdType n34, smIdType n41, smIdType nCenter)
{
  if (myType != SMESHDS_AddBiQuadQuadrangle) {
    MESSAGE("SMESHDS_Command::AddFace : Bad Type");
    return;
  }
  myIntegers.push_back(NewFaceID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n34);
  myIntegers.push_back(n41);
  myIntegers.push_back(nCenter);
  myNumber++;
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                smIdType n12, smIdType n23, smIdType n31,
                                smIdType n14, smIdType n24, smIdType n34)
{
  if ( myType != SMESHDS_AddQuadTetrahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n31);
  myIntegers.push_back(n14);
  myIntegers.push_back(n24);
  myIntegers.push_back(n34);
  myNumber++;
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2,
                                smIdType n3, smIdType n4, smIdType n5,
                                smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                                smIdType n15, smIdType n25, smIdType n35, smIdType n45)
{
  if ( myType != SMESHDS_AddQuadPyramid) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n5);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n34);
  myIntegers.push_back(n41);
  myIntegers.push_back(n15);
  myIntegers.push_back(n25);
  myIntegers.push_back(n35);
  myIntegers.push_back(n45);
  myNumber++;
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2,
                                smIdType n3, smIdType n4, smIdType n5,smIdType n6,
                                smIdType n12, smIdType n23, smIdType n31,
                                smIdType n45, smIdType n56, smIdType n64,
                                smIdType n14, smIdType n25, smIdType n36)
{
  if ( myType != SMESHDS_AddQuadPentahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n5);
  myIntegers.push_back(n6);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n31);
  myIntegers.push_back(n45);
  myIntegers.push_back(n56);
  myIntegers.push_back(n64);
  myIntegers.push_back(n14);
  myIntegers.push_back(n25);
  myIntegers.push_back(n36);
  myNumber++;
}
//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2,
                                smIdType n3, smIdType n4, smIdType n5,smIdType n6,
                                smIdType n12, smIdType n23, smIdType n31,
                                smIdType n45, smIdType n56, smIdType n64,
                                smIdType n14, smIdType n25, smIdType n36,
                                smIdType n1245, smIdType n2356, smIdType n1346)
{
  if ( myType != SMESHDS_AddBiQuadPentahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n5);
  myIntegers.push_back(n6);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n31);
  myIntegers.push_back(n45);
  myIntegers.push_back(n56);
  myIntegers.push_back(n64);
  myIntegers.push_back(n14);
  myIntegers.push_back(n25);
  myIntegers.push_back(n36);
  myIntegers.push_back(n1245);
  myIntegers.push_back(n2356);
  myIntegers.push_back(n1346);
  myNumber++;
}


//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                                smIdType n4, smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                                smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                                smIdType n15, smIdType n26, smIdType n37, smIdType n48)
{
  if ( myType != SMESHDS_AddQuadHexahedron) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n5);
  myIntegers.push_back(n6);
  myIntegers.push_back(n7);
  myIntegers.push_back(n8);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n34);
  myIntegers.push_back(n41);
  myIntegers.push_back(n56);
  myIntegers.push_back(n67);
  myIntegers.push_back(n78);
  myIntegers.push_back(n85);
  myIntegers.push_back(n15);
  myIntegers.push_back(n26);
  myIntegers.push_back(n37);
  myIntegers.push_back(n48);
  myNumber++;
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Command::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                                smIdType n4, smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                                smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                                smIdType n15, smIdType n26, smIdType n37, smIdType n48,
                                smIdType n1234,smIdType n1256,smIdType n2367,smIdType n3478,
                                smIdType n1458,smIdType n5678,smIdType nCenter)
{
  if ( myType != SMESHDS_AddTriQuadHexa) {
    MESSAGE("SMESHDS_Command::AddVolume : Bad Type");
    return;
  }
  myIntegers.push_back(NewVolID);
  myIntegers.push_back(n1);
  myIntegers.push_back(n2);
  myIntegers.push_back(n3);
  myIntegers.push_back(n4);
  myIntegers.push_back(n5);
  myIntegers.push_back(n6);
  myIntegers.push_back(n7);
  myIntegers.push_back(n8);
  myIntegers.push_back(n12);
  myIntegers.push_back(n23);
  myIntegers.push_back(n34);
  myIntegers.push_back(n41);
  myIntegers.push_back(n56);
  myIntegers.push_back(n67);
  myIntegers.push_back(n78);
  myIntegers.push_back(n85);
  myIntegers.push_back(n15);
  myIntegers.push_back(n26);
  myIntegers.push_back(n37);
  myIntegers.push_back(n48);
  myIntegers.push_back(n1234);
  myIntegers.push_back(n1256);
  myIntegers.push_back(n2367);
  myIntegers.push_back(n3478);
  myIntegers.push_back(n1458);
  myIntegers.push_back(n5678);
  myIntegers.push_back(nCenter);
  myNumber++;
}

//================================================================================
/*!
 * \brief Record adding a Ball
 */
//================================================================================

void SMESHDS_Command::AddBall(smIdType NewBallID, smIdType node, double diameter)
{
  if ( myType != SMESHDS_AddBall)
  {
    MESSAGE("SMESHDS_Command::SMESHDS_AddBall : Bad Type");
    return;
  }
  myIntegers.push_back(NewBallID);
  myIntegers.push_back(node);
  myReals.push_back(diameter);
  myNumber++;
}
