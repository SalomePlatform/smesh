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
//  File   : SMESH_Script.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//
#include "SMESHDS_Script.hxx"
#include <iostream>

using namespace std;

//=======================================================================
//function : Constructor
//purpose  : 
//=======================================================================
SMESHDS_Script::SMESHDS_Script(bool theIsEmbeddedMode):
  myIsEmbeddedMode(theIsEmbeddedMode)
{
  //cerr << "=========================== myIsEmbeddedMode " << myIsEmbeddedMode << endl;
}

//=======================================================================
//function : Destructor
//purpose  : 
//=======================================================================
SMESHDS_Script::~SMESHDS_Script()
{
  Clear();
}

//=======================================================================
void SMESHDS_Script::SetModified(bool theModified)
{
  myIsModified = theModified;
}

//=======================================================================
bool SMESHDS_Script::IsModified()
{
  return myIsModified;
}

//=======================================================================
//function : getCommand
//purpose  : 
//=======================================================================
SMESHDS_Command* SMESHDS_Script::getCommand(const SMESHDS_CommandType aType)
{
  SMESHDS_Command* com;
  if (myCommands.empty())
  {
    com = new SMESHDS_Command(aType);
    myCommands.insert(myCommands.end(),com);
  }
  else
  {
    com = myCommands.back();
    if (com->GetType() != aType)
    {
      com = new SMESHDS_Command(aType);
      myCommands.insert(myCommands.end(),com);
    }
  }
  return com;
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddNode(smIdType NewNodeID, double x, double y, double z)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddNode)->AddNode(NewNodeID, x, y, z);
}

//=======================================================================
//function :
//purpose  :
//=======================================================================
void SMESHDS_Script::Add0DElement (smIdType New0DElementID, smIdType idnode)
{
  if (myIsEmbeddedMode) {
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_Add0DElement)->Add0DElement(New0DElementID, idnode);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(smIdType NewEdgeID, smIdType idnode1, smIdType idnode2)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddEdge)->AddEdge(NewEdgeID, idnode1, idnode2);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID,
                             smIdType idnode1, smIdType idnode2, smIdType idnode3)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddTriangle)->AddFace(NewFaceID,
                                           idnode1, idnode2, idnode3);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID,
                             smIdType idnode1, smIdType idnode2,
                             smIdType idnode3, smIdType idnode4)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadrangle)->AddFace(NewFaceID,
                                             idnode1, idnode2,
                                             idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewID,
                               smIdType idnode1, smIdType idnode2,
                               smIdType idnode3, smIdType idnode4)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddTetrahedron)->AddVolume(NewID,
                                                idnode1, idnode2,
                                                idnode3, idnode4);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewID,
                               smIdType idnode1, smIdType idnode2,
                               smIdType idnode3, smIdType idnode4, smIdType idnode5)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddPyramid)->AddVolume(NewID,
                                            idnode1, idnode2,
                                            idnode3, idnode4, idnode5);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewID,
                               smIdType idnode1, smIdType idnode2, smIdType idnode3,
                               smIdType idnode4, smIdType idnode5, smIdType idnode6)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddPrism)->AddVolume(NewID,
                                          idnode1, idnode2, idnode3,
                                          idnode4, idnode5, idnode6);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewID,
                               smIdType idnode1, smIdType idnode2, smIdType idnode3, smIdType idnode4,
                               smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddHexahedron)->AddVolume(NewID,
                                               idnode1, idnode2, idnode3, idnode4,
                                               idnode5, idnode6, idnode7, idnode8);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                               smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8,
                               smIdType idnode9, smIdType idnode10, smIdType idnode11, smIdType idnode12)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddHexagonalPrism)->AddVolume(NewVolID,
                                                   idnode1, idnode2, idnode3, idnode4,
                                                   idnode5, idnode6, idnode7, idnode8,
                                                   idnode9, idnode10, idnode11, idnode12);
}

//=======================================================================
//function : AddPolygonalFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddPolygonalFace (smIdType NewFaceID, const std::vector<smIdType>& nodes_ids)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddPolygon)->AddPolygonalFace(NewFaceID, nodes_ids);
}

//=======================================================================
//function : AddQuadPolygonalFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddQuadPolygonalFace(smIdType NewFaceID, const std::vector<smIdType>& nodes_ids)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadPolygon)->AddQuadPolygonalFace(NewFaceID, nodes_ids);
}

//=======================================================================
//function : AddPolyhedralVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddPolyhedralVolume (smIdType                     NewID,
                                          const std::vector<smIdType>& nodes_ids,
                                          const std::vector<int>&      quantities)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddPolyhedron)->AddPolyhedralVolume
    (NewID, nodes_ids, quantities);
}

//=======================================================================
//function : AddBall
//purpose  : Record adding a Ball
//=======================================================================

void SMESHDS_Script::AddBall(smIdType NewBallID, smIdType node, double diameter)
{
  if ( myIsEmbeddedMode )
    myIsModified = true;
  else
    getCommand(SMESHDS_AddBall)->AddBall(NewBallID, node, diameter);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::MoveNode(smIdType NewNodeID, double x, double y, double z)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_MoveNode)->MoveNode(NewNodeID, x, y, z);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveNode(smIdType ID)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_RemoveNode)->RemoveNode(ID);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::RemoveElement(smIdType ElementID)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_RemoveElement)->RemoveElement(ElementID);
}

//=======================================================================
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

void SMESHDS_Script::ChangeElementNodes(smIdType ElementID, smIdType nodes[], smIdType nbnodes)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_ChangeElementNodes)->ChangeElementNodes( ElementID, nodes, nbnodes );
}

//=======================================================================
//function : ChangePolyhedronNodes
//purpose  : 
//=======================================================================
void SMESHDS_Script::ChangePolyhedronNodes (const smIdType               ElementID,
                                            const std::vector<smIdType>& nodes_ids,
                                            const std::vector<int>& quantities)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_ChangePolyhedronNodes)->ChangePolyhedronNodes
    (ElementID, nodes_ids, quantities);
}

//=======================================================================
//function : Renumber
//purpose  : 
//=======================================================================
void SMESHDS_Script::Renumber (const bool isNodes, const smIdType startID, const smIdType deltaID)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_Renumber)->Renumber( isNodes, startID, deltaID );
}

//=======================================================================
//function : ClearMesh
//purpose  : 
//=======================================================================
void SMESHDS_Script::ClearMesh ()
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  Clear();// previous commands become useless to reproduce on client side
  getCommand(SMESHDS_ClearAll);
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
void SMESHDS_Script::Clear()
{
  list<SMESHDS_Command*>::iterator anIt = myCommands.begin();
  for (; anIt != myCommands.end(); anIt++) {
    delete (*anIt);
  }
  myCommands.clear();
}

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
const list<SMESHDS_Command*>& SMESHDS_Script::GetCommands()
{
  return myCommands;
}


//********************************************************************
//*****             Methods for quadratic elements              ******
//********************************************************************

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddEdge(smIdType NewEdgeID, smIdType n1, smIdType n2, smIdType n12)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadEdge)->AddEdge(NewEdgeID, n1, n2, n12);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3,
                             smIdType n12, smIdType n23, smIdType n31)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadTriangle)->AddFace(NewFaceID, n1, n2, n3,
                                               n12, n23, n31);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3,
                             smIdType n12, smIdType n23, smIdType n31, smIdType nCenter)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddBiQuadTriangle)->AddFace(NewFaceID, n1, n2, n3,
                                                 n12, n23, n31, nCenter);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                             smIdType n12, smIdType n23, smIdType n34, smIdType n41)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadQuadrangle)->AddFace(NewFaceID, n1, n2, n3, n4,
                                                 n12, n23, n34, n41);
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                             smIdType n12, smIdType n23, smIdType n34, smIdType n41, smIdType nCenter)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddBiQuadQuadrangle)->AddFace(NewFaceID, n1, n2, n3, n4,
                                                   n12, n23, n34, n41, nCenter);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                               smIdType n12, smIdType n23, smIdType n31,
                               smIdType n14, smIdType n24, smIdType n34)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadTetrahedron)->AddVolume(NewVolID, n1, n2, n3, n4,
                                                    n12, n23, n31,
                                                    n14, n24, n34);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                               smIdType n5, smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                               smIdType n15, smIdType n25, smIdType n35, smIdType n45)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadPyramid)->AddVolume(NewVolID, n1, n2, n3, n4, n5,
                                                n12, n23, n34, n41,
                                                n15, n25, n35, n45);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                smIdType n5,smIdType n6, smIdType n12, smIdType n23, smIdType n31,
                                smIdType n45, smIdType n56, smIdType n64,
                                smIdType n14, smIdType n25, smIdType n36)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadPentahedron)->AddVolume(NewVolID, n1,n2,n3,n4,n5,n6,
                                                    n12, n23, n31,
                                                    n45, n56, n64,
                                                    n14, n25, n36);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                smIdType n5,smIdType n6, smIdType n12, smIdType n23, smIdType n31,
                                smIdType n45, smIdType n56, smIdType n64,
                                smIdType n14, smIdType n25, smIdType n36,
                                smIdType n1245, smIdType n2356, smIdType n1346)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddBiQuadPentahedron)->AddVolume(NewVolID, n1,n2,n3,n4,n5,n6,
                                                    n12, n23, n31,
                                                    n45, n56, n64,
                                                    n14, n25, n36,
                                                    n1245, n2356, n1346);
}

//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                               smIdType n4, smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                               smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                               smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                               smIdType n15, smIdType n26, smIdType n37, smIdType n48)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddQuadHexahedron)->AddVolume(NewVolID, n1, n2, n3, n4,
                                                   n5, n6, n7, n8,
                                                   n12, n23, n34, n41,
                                                   n56, n67, n78, n85,
                                                   n15, n26, n37, n48);
}

//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
void SMESHDS_Script::AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                               smIdType n4, smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                               smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                               smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                               smIdType n15, smIdType n26, smIdType n37, smIdType n48,
                               smIdType n1234,smIdType n1256,smIdType n2367,smIdType n3478,
                               smIdType n1458,smIdType n5678,smIdType nCenter)
{
  if(myIsEmbeddedMode){
    myIsModified = true;
    return;
  }
  getCommand(SMESHDS_AddTriQuadHexa)->AddVolume(NewVolID, n1, n2, n3, n4,
                                                n5, n6, n7, n8,
                                                n12, n23, n34, n41,
                                                n56, n67, n78, n85,
                                                n15, n26, n37, n48,
                                                n1234, n1256, n2367, n3478,
                                                n1458, n5678, nCenter);
}

