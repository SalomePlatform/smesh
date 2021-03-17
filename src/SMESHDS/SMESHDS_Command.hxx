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
//  File   : SMESHDS_Command.hxx
//  Module : SMESH
//
#ifndef _SMESHDS_Command_HeaderFile
#define _SMESHDS_Command_HeaderFile

#include "SMESH_SMESHDS.hxx"

#include "SMESHDS_CommandType.hxx"
#include <smIdType.hxx>
#include <list>
#include <vector>

class SMESHDS_EXPORT SMESHDS_Command
{

  public:
        SMESHDS_Command(const SMESHDS_CommandType aType);
        void AddNode(smIdType NewNodeID, double x, double y, double z);
        void Add0DElement(smIdType New0DElementID, smIdType idnode);
        void AddEdge(smIdType NewEdgeID, smIdType idnode1, smIdType idnode2);
        void AddFace(smIdType NewFaceID, smIdType idnode1, smIdType idnode2, smIdType idnode3);
        void AddFace(smIdType NewFaceID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                     smIdType idnode4);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8);
        void AddVolume(smIdType NewVolID, smIdType idnode1, smIdType idnode2, smIdType idnode3,
                       smIdType idnode4, smIdType idnode5, smIdType idnode6, smIdType idnode7, smIdType idnode8,
                       smIdType idnode9, smIdType idnode10, smIdType idnode11, smIdType idnode12);
        void AddPolygonalFace (const smIdType               ElementID,
                               const std::vector<smIdType>& nodes_ids);
        void AddQuadPolygonalFace (const smIdType               ElementID,
                                   const std::vector<smIdType>& nodes_ids);
        void AddPolyhedralVolume (const smIdType               ElementID,
                                  const std::vector<smIdType>& nodes_ids,
                                  const std::vector<int>&      quantities);
        void AddBall(smIdType NewBallID, smIdType node, double diameter);
        // special methods for quadratic elements
        void AddEdge(smIdType NewEdgeID, smIdType n1, smIdType n2, smIdType n12);
        void AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3,
                     smIdType n12, smIdType n23, smIdType n31);
        void AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3,
                     smIdType n12, smIdType n23, smIdType n31, smIdType nCenter);
        void AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                     smIdType n12, smIdType n23, smIdType n34, smIdType n41);
        void AddFace(smIdType NewFaceID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                     smIdType n12, smIdType n23, smIdType n34, smIdType n41, smIdType nCenter);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                       smIdType n12, smIdType n23, smIdType n31, smIdType n14, smIdType n24, smIdType n34);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType n5,
                       smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                       smIdType n15, smIdType n25, smIdType n35, smIdType n45);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                       smIdType n4, smIdType n5, smIdType n6,
                       smIdType n12, smIdType n23, smIdType n31,
                       smIdType n45, smIdType n56, smIdType n64,
                       smIdType n14, smIdType n25, smIdType n36);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3,
                       smIdType n4, smIdType n5, smIdType n6,
                       smIdType n12, smIdType n23, smIdType n31,
                       smIdType n45, smIdType n56, smIdType n64,
                       smIdType n14, smIdType n25, smIdType n36,
                       smIdType n1245, smIdType n2356, smIdType n1346);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                       smIdType n5,  smIdType n6,  smIdType n7,  smIdType n8,
                       smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                       smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                       smIdType n15, smIdType n26, smIdType n37, smIdType n48);
        void AddVolume(smIdType NewVolID, smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                       smIdType n5,  smIdType n6,  smIdType n7,  smIdType n8,
                       smIdType n12, smIdType n23, smIdType n34, smIdType n41,
                       smIdType n56, smIdType n67, smIdType n78, smIdType n85,
                       smIdType n15, smIdType n26, smIdType n37, smIdType n48,
                       smIdType n1234,smIdType n1256,smIdType n2367,smIdType n3478,
                       smIdType n1458,smIdType n5678,smIdType nCenter);
        
        void MoveNode(smIdType NewNodeID, double x, double y, double z);
        void RemoveNode(smIdType NodeID);
        void RemoveElement(smIdType ElementID);
        void ChangeElementNodes(smIdType ElementID, smIdType nodes[], smIdType nbnodes);
        void ChangePolyhedronNodes(const smIdType               ElementID,
                                   const std::vector<smIdType>& nodes_ids,
                                   const std::vector<int>& quantities);
        void Renumber (const bool isNodes, const smIdType startID, const smIdType deltaID);
        SMESHDS_CommandType GetType();
        smIdType GetNumber();
        const std::list<smIdType> & GetIndexes();
        const std::list<double> & GetCoords();
         ~SMESHDS_Command();
  private:
        SMESHDS_CommandType myType;
        int myNumber;
        std::list<double> myReals;
        std::list<smIdType> myIntegers;
};
#endif
