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
// File      : SMDS_VolumeTool.hxx
// Module    : SMESH
// Created   : Tue Jul 13 11:27:17 2004
// Author    : Edward AGAPOV (eap)


#ifndef SMDS_VolumeTool_HeaderFile
#define SMDS_VolumeTool_HeaderFile

class SMDS_MeshElement;
class SMDS_MeshNode;

#include <vector>
#include <set>

// =========================================================================
//
// Class providing topological and other information about SMDS_MeshVolume:
// allows iteration on faces or, to be precise, on nodes of volume sides;
// provides info on nodes connection etc.
//
// =========================================================================

class SMDS_VolumeTool
{
 public:

  SMDS_VolumeTool ();
  ~SMDS_VolumeTool ();
  SMDS_VolumeTool (const SMDS_MeshElement* theVolume);

  bool Set (const SMDS_MeshElement* theVolume);
  // Set volume.
  // Return false if theVolume is not of type SMDSAbs_Volume

  // -----------------------
  // general info
  // -----------------------

  bool IsForward() const { return myVolForward; }
  // Check volume orientation. can be changed by Inverse().
  // See node order of forward volumes at the file bottom

  void Inverse();
  // Change nodes order as if the volume changes its orientation:
  // top and bottom faces are reversed.
  // Result of IsForward() and methods returning nodes change

  const SMDS_MeshNode** GetNodes() { return myVolumeNodes; }
  // Return array of volume nodes

  int NbNodes() { return myVolumeNbNodes; }
  // Return array of volume nodes

  double GetSize() const;
  // Return element volume

  bool GetBaryCenter (double & X, double & Y, double & Z) const;


  // -----------------------
  // info on node connection
  // -----------------------

  bool IsLinked (const SMDS_MeshNode* theNode1,
                 const SMDS_MeshNode* theNode2) const;
  // Return true if theNode1 is linked with theNode2.

  bool IsLinked (const int theNode1Index,
                 const int theNode2Index) const;
  // Return true if the node with theNode1Index is linked
  // with the node with theNode2Index

  int GetNodeIndex(const SMDS_MeshNode* theNode) const;
  // Return an index of theNode

  // -------------
  // info on faces
  // -------------

  void SetExternalNormal ();
  // Node order in faces  will be so that faces normals are external.

  int NbFaces() const { return myNbFaces; }
  // Return number of faces of the volume. In the following
  // methods 0 <= faceIndex < NbFaces()

  int NbFaceNodes( int faceIndex );
  // Return number of nodes in the array of face nodes

  const int* GetFaceNodesIndices( int faceIndex );
  // Return the array of face nodes indices
  // To comfort link iteration, the array
  // length == NbFaceNodes( faceIndex ) + 1 and
  // the last node index == the first one.

  const SMDS_MeshNode** GetFaceNodes( int faceIndex );
  // Return the array of face nodes.
  // To comfort link iteration, the array
  // length == NbFaceNodes( faceIndex ) + 1 and
  // the last node == the first one.
  // WARNING: do not modify the array, some methods
  //          work basing on its contents

  bool GetFaceNodes (int faceIndex,
                     std::set<const SMDS_MeshNode*>& theFaceNodes );
  // Return a set of face nodes.

  bool IsFaceExternal( int faceIndex );
  // Check normal orientation of a face.
  // SetExternalNormal() is taken into account.

  bool IsFreeFace(  int faceIndex );
  // Check that all volumes built on the face nodes lays on one side

  bool GetFaceNormal (int faceIndex, double & X, double & Y, double & Z);
  // Return a normal to a face

  double GetFaceArea( int faceIndex );
  // Return face area

  int GetOppFaceIndex( int faceIndex ) const;
  // Return index of the opposite face if it exists, else -1.

  int GetFaceIndex( const std::set<const SMDS_MeshNode*>& theFaceNodes );
  // Return index of a face formed by theFaceNodes.
  // Return -1 if a face not found

  int GetFaceIndex( const std::set<int>& theFaceNodesIndices );
  // Return index of a face formed by theFaceNodesIndices
  // Return -1 if a face not found


 private:

  bool setFace( int faceIndex );

  const SMDS_MeshElement* myVolume;
  bool                    myVolForward;
  int                     myNbFaces;
  int                     myVolumeNbNodes;
  const SMDS_MeshNode*    myVolumeNodes[ 8 ];

  bool                    myExternalFaces;
  int*                    myFaceNodeIndices;
  int*                    myFaceNbNodes;
  const SMDS_MeshNode*    myFaceNodes[ 5 ];
  int                     myCurFace;

};
#endif


///////////////////////////////////////////////////////////////////////////
//
//                   ORDER OF NODES OF FORWARD ELEMENT
//
///////////////////////////////////////////////////////////////////////////
/*
//           N3
//           +
//          /|\
//         / | \
//        /  |  \
//    N0 +---|---+ N1                TETRAHEDRON
//       \   |   /
//        \  |  /
//         \ | /
//          \|/
//           +
//           N2

//            + N4
//           /|\
//          / | \
//         /  |  \
//        /   |   \
//    N3 +---------+ N5
//       |    |    |
//       |    + N1 |
//       |   / \   |                PENTAHEDRON
//       |  /   \  |
//       | /     \ |
//       |/       \|
//    N0 +---------+ N2

//         N5+----------+N6
//          /|         /|
//         / |        / |
//        /  |       /  |
//     N4+----------+N7 |
//       |   |      |   |           HEXAHEDRON
//       |   |      |   |
//       |   |      |   |
//       | N1+------|---+N2
//       |  /       |  /
//       | /        | /
//       |/         |/
//     N0+----------+N3
//
*/
