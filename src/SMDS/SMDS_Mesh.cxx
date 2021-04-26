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

//  SMESH SMDS : implementation of Salome mesh data structure
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_Mesh.hxx"

#include "SMDS_ElementFactory.hxx"
#include "SMDS_ElementHolder.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_SpacePosition.hxx"
#include "SMDS_UnstructuredGrid.hxx"

#include <utilities.h>

#include <vtkUnstructuredGrid.h>
//#include <vtkUnstructuredGridWriter.h>
#include <vtkCell.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellLinks.h>
#include <vtkIdList.h>

#include <algorithm>
#include <iostream>
#include <fstream>

#include <boost/make_shared.hpp>

#if !defined WIN32 && !defined __APPLE__
#include <sys/sysinfo.h>
#endif

// number of added entities to check memory after
#define CHECKMEMORY_INTERVAL 100000

#define MYASSERT(val) if (!(val)) throw SALOME_Exception(LOCALIZED("assertion not verified"));

int SMDS_Mesh::chunkSize = 1024;

//================================================================================
/*!
 * \brief Raise an exception if free memory (ram+swap) too low
 * \param doNotRaise - if true, suppress exception, just return free memory size
 * \retval int - amount of available memory in MB or negative number in failure case
 */
//================================================================================

int SMDS_Mesh::CheckMemory(const bool doNotRaise)
{
  return -1;
#if !defined WIN32 && !defined __APPLE__
  struct sysinfo si;
  int err = sysinfo( &si );
  if ( err )
    return -1;

  const unsigned long Mbyte = 1024 * 1024;

  static int limit = -1;
  if ( limit < 0 ) {
    if ( si.totalswap == 0 )
    {
      int status = system("SMDS_MemoryLimit"); // it returns lower limit of free RAM
      if (status >= 0 ) {
        limit = WEXITSTATUS(status);
      }
      else {
        double factor = ( si.totalswap == 0 ) ? 0.1 : 0.2;
        limit = int(( factor * si.totalram * si.mem_unit ) / Mbyte );
      }
    }
    if ( limit < 20 )
      limit = 20;
    else
      limit = int ( limit * 1.5 );
    MESSAGE ( "SMDS_Mesh::CheckMemory() memory limit = " << limit << " MB" );
  }

  // compute separately to avoid overflow
  int freeMb =
    ( si.freeram  * si.mem_unit ) / Mbyte +
    ( si.freeswap * si.mem_unit ) / Mbyte;

  if ( freeMb > limit )
    return freeMb - limit;

  if ( doNotRaise )
    return 0;

  MESSAGE ("SMDS_Mesh::CheckMemory() throws as free memory too low: " << freeMb <<" MB" );
  throw std::bad_alloc();
#else
  return -1;
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new mesh object
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh::SMDS_Mesh():
  myNodeFactory( new SMDS_NodeFactory( this )),
  myCellFactory( new SMDS_ElementFactory( this )),
  myParent(NULL),
  myModified(false), myModifTime(0), myCompactTime(0),
  xmin(0), xmax(0), ymin(0), ymax(0), zmin(0), zmax(0)
{
  myGrid = SMDS_UnstructuredGrid::New();
  myGrid->setSMDS_mesh(this);
  myGrid->Initialize();
  myGrid->Allocate();
  vtkPoints* points = vtkPoints::New();
  // bug "21125: EDF 1233 SMESH: Degrardation of precision in a test case for quadratic conversion"
  // Use double type for storing coordinates of nodes instead of float.
  points->SetDataType(VTK_DOUBLE);
  points->SetNumberOfPoints( 0 );
  myGrid->SetPoints( points );
  points->Delete();
  this->Modified();

  // initialize static maps in SMDS_MeshCell, to be thread-safe
  SMDS_MeshCell::InitStaticMembers();
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new child mesh
/// Note that the tree structure of SMDS_Mesh seems to be unused in this version
/// (2003-09-08) of SMESH
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh::SMDS_Mesh(SMDS_Mesh * parent):
  myNodeFactory( new SMDS_NodeFactory( this )),
  myCellFactory( new SMDS_ElementFactory( this )),
  myParent(parent)
{
}

///////////////////////////////////////////////////////////////////////////////
///Create a submesh and add it to the current mesh
///////////////////////////////////////////////////////////////////////////////

SMDS_Mesh *SMDS_Mesh::AddSubMesh()
{
  SMDS_Mesh *submesh = new SMDS_Mesh(this);
  myChildren.insert(myChildren.end(), submesh);
  return submesh;
}

///////////////////////////////////////////////////////////////////////////////
///create a MeshNode and add it to the current Mesh
///An ID is automatically assigned to the node.
///@return : The created node
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshNode * SMDS_Mesh::AddNode(double x, double y, double z)
{
  return SMDS_Mesh::AddNodeWithID( x,y,z, myNodeFactory->GetFreeID() );
}

///////////////////////////////////////////////////////////////////////////////
///create a MeshNode and add it to the current Mesh
///@param ID : The ID of the MeshNode to create
///@return : The created node or NULL if a node with this ID already exists
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshNode * SMDS_Mesh::AddNodeWithID( double x, double y, double z, smIdType ID )
{
  // find the MeshNode corresponding to ID
  SMDS_MeshNode *node = myNodeFactory->NewNode( ID );
  if ( node )
  {
    node->init( x, y, z );
    myInfo.myNbNodes++;
    myModified = true;
    this->adjustBoundingBox(x, y, z);
  }
  return node;
}

///////////////////////////////////////////////////////////////////////////////
/// create a Mesh0DElement and add it to the current Mesh
/// @return : The created Mesh0DElement
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh0DElement* SMDS_Mesh::Add0DElementWithID(smIdType idnode, smIdType ID)
{
  const SMDS_MeshNode * node = myNodeFactory->FindNode(idnode);
  if (!node) return NULL;
  return SMDS_Mesh::Add0DElementWithID(node, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// create a Mesh0DElement and add it to the current Mesh
/// @return : The created Mesh0DElement
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh0DElement* SMDS_Mesh::Add0DElement(const SMDS_MeshNode * node)
{
  return SMDS_Mesh::Add0DElementWithID( node, myCellFactory->GetFreeID() );
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new Mesh0DElement and at it to the mesh
/// @param idnode ID of the node
/// @param ID ID of the 0D element to create
/// @return The created 0D element or NULL if an element with this
///         ID already exists or if input node is not found.
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh0DElement* SMDS_Mesh::Add0DElementWithID(const SMDS_MeshNode * n, smIdType ID)
{
  if (!n) return 0;

  if (Nb0DElements() % CHECKMEMORY_INTERVAL == 0) CheckMemory();

  if ( SMDS_MeshCell * cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_0D, /*nbNodes=*/1, n );
    myInfo.myNb0DElements++;
    return static_cast< SMDS_Mesh0DElement*> ( cell );
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// create a Ball and add it to the current Mesh
/// @return : The created Ball
///////////////////////////////////////////////////////////////////////////////
SMDS_BallElement* SMDS_Mesh::AddBallWithID( smIdType idnode, double diameter, smIdType ID )
{
  const SMDS_MeshNode * node = myNodeFactory->FindNode( idnode );
  if (!node) return NULL;
  return SMDS_Mesh::AddBallWithID( node, diameter, ID );
}

///////////////////////////////////////////////////////////////////////////////
/// create a Ball and add it to the current Mesh
/// @return : The created Ball
///////////////////////////////////////////////////////////////////////////////
SMDS_BallElement* SMDS_Mesh::AddBall(const SMDS_MeshNode * node, double diameter)
{
  return SMDS_Mesh::AddBallWithID(node, diameter, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new Ball and at it to the mesh
/// @param idnode ID of the node
//  @param diameter ball diameter
/// @param ID ID of the 0D element to create
/// @return The created 0D element or NULL if an element with this
///         ID already exists or if input node is not found.
///////////////////////////////////////////////////////////////////////////////
SMDS_BallElement* SMDS_Mesh::AddBallWithID(const SMDS_MeshNode * n, double diameter, smIdType ID)
{
  if (!n) return 0;

  if (NbBalls() % CHECKMEMORY_INTERVAL == 0) CheckMemory();

  SMDS_BallElement* ball = static_cast< SMDS_BallElement*>( myCellFactory->NewElement( ID ));
  if ( ball )
  {
    ball->init( n, diameter );
    myInfo.myNbBalls++;
  }
  return ball;
}

///////////////////////////////////////////////////////////////////////////////
/// create a MeshEdge and add it to the current Mesh
/// @return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(smIdType idnode1, smIdType idnode2, smIdType ID)
{
  const SMDS_MeshNode * node1 = myNodeFactory->FindNode(idnode1);
  const SMDS_MeshNode * node2 = myNodeFactory->FindNode(idnode2);
  if(!node1 || !node2) return NULL;
  return SMDS_Mesh::AddEdgeWithID(node1, node2, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// create a MeshEdge and add it to the current Mesh
/// @return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdge(const SMDS_MeshNode * node1,
                                  const SMDS_MeshNode * node2)
{
  return SMDS_Mesh::AddEdgeWithID(node1, node2, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new edge and at it to the mesh
/// @param idnode1 ID of the first node
/// @param idnode2 ID of the second node
/// @param ID ID of the edge to create
/// @return The created edge or NULL if an element with this ID already exists or
/// if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        smIdType              ID)
{
  if ( !n1 || !n2 ) return 0;

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Edge, /*nbNodes=*/2, n1, n2 );
    myInfo.myNbEdges++;
    return static_cast<SMDS_MeshEdge*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes. An ID is automatically affected to the
/// Created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType idnode1,
                                        smIdType idnode2,
                                        smIdType idnode3,
                                        smIdType ID)
{
  const SMDS_MeshNode * node1 = myNodeFactory->FindNode(idnode1);
  const SMDS_MeshNode * node2 = myNodeFactory->FindNode(idnode2);
  const SMDS_MeshNode * node3 = myNodeFactory->FindNode(idnode3);
  if(!node1 || !node2 || !node3) return NULL;
  return SMDS_Mesh::AddFaceWithID(node1, node2, node3, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 ) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Triangle, /*nbNodes=*/3, n1, n2, n3 );
    myInfo.myNbTriangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes. An ID is automatically affected to the
/// created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3,
                                  const SMDS_MeshNode * n4)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3, n4, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType idnode1,
                                        smIdType idnode2,
                                        smIdType idnode3,
                                        smIdType idnode4,
                                        smIdType ID)
{
  const SMDS_MeshNode *node1, *node2, *node3, *node4;
  node1 = myNodeFactory->FindNode(idnode1);
  node2 = myNodeFactory->FindNode(idnode2);
  node3 = myNodeFactory->FindNode(idnode3);
  node4 = myNodeFactory->FindNode(idnode4);
  if ( !node1 || !node2 || !node3 || !node4 ) return NULL;
  return SMDS_Mesh::AddFaceWithID(node1, node2, node3, node4, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n4,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 ) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quadrangle, /*nbNodes=*/4, n1, n2, n3, n4 );
    myInfo.myNbQuadrangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh.
///@return The created tetrahedron
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, myCellFactory->GetFreeID() );
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh.
///@param ID The ID of the new volume
///@return The created tetrahedron or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(smIdType idnode1,
                                             smIdType idnode2,
                                             smIdType idnode3,
                                             smIdType idnode4,
                                             smIdType ID)
{
  const SMDS_MeshNode *node1, *node2, *node3, *node4;
  node1 = myNodeFactory->FindNode(idnode1);
  node2 = myNodeFactory->FindNode(idnode2);
  node3 = myNodeFactory->FindNode(idnode3);
  node4 = myNodeFactory->FindNode(idnode4);
  if(!node1 || !node2 || !node3 || !node4) return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh.
///@param ID The ID of the new volume
///@return The created tetrahedron
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 ) return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Tetra, /*nbNodes=*/4, n1, n2, n3, n4 );
    myInfo.myNbTetras++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh.
///Nodes 1,2,3 and 4 define the base of the pyramid
///@return The created pyramid
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, myCellFactory->GetFreeID() );
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh.
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(smIdType idnode1,
                                             smIdType idnode2,
                                             smIdType idnode3,
                                             smIdType idnode4,
                                             smIdType idnode5,
                                             smIdType ID)
{
  const SMDS_MeshNode *node1, *node2, *node3, *node4, *node5;
  node1 = myNodeFactory->FindNode(idnode1);
  node2 = myNodeFactory->FindNode(idnode2);
  node3 = myNodeFactory->FindNode(idnode3);
  node4 = myNodeFactory->FindNode(idnode4);
  node5 = myNodeFactory->FindNode(idnode5);
  if(!node1 || !node2 || !node3 || !node4 || !node5) return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh.
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 || !n5 ) return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Pyramid, /*nbNodes=*/5, n1, n2, n3, n4, n5 );
    myInfo.myNbPyramids++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh.
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6)
{
  smIdType ID = myCellFactory->GetFreeID();
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh.
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(smIdType idnode1,
                                             smIdType idnode2,
                                             smIdType idnode3,
                                             smIdType idnode4,
                                             smIdType idnode5,
                                             smIdType idnode6,
                                             smIdType ID)
{
  const SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6;
  node1 = myNodeFactory->FindNode(idnode1);
  node2 = myNodeFactory->FindNode(idnode2);
  node3 = myNodeFactory->FindNode(idnode3);
  node4 = myNodeFactory->FindNode(idnode4);
  node5 = myNodeFactory->FindNode(idnode5);
  node6 = myNodeFactory->FindNode(idnode6);
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, node6, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh.
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 || !n5 || !n6 ) return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Penta, /*nbNodes=*/6, n1, n2, n3, n4, n5, n6 );
    myInfo.myNbPrisms++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexagonal prism and add it to the mesh.
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n7,
                                      const SMDS_MeshNode * n8,
                                      const SMDS_MeshNode * n9,
                                      const SMDS_MeshNode * n10,
                                      const SMDS_MeshNode * n11,
                                      const SMDS_MeshNode * n12)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6,
                                    n7, n8, n9, n10, n11, n12,
                                    myCellFactory->GetFreeID() );
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexagonal prism and add it to the mesh.
///@param ID The ID of the new volume
///@return The created prism or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(smIdType idnode1,
                                             smIdType idnode2,
                                             smIdType idnode3,
                                             smIdType idnode4,
                                             smIdType idnode5,
                                             smIdType idnode6,
                                             smIdType idnode7,
                                             smIdType idnode8,
                                             smIdType idnode9,
                                             smIdType idnode10,
                                             smIdType idnode11,
                                             smIdType idnode12,
                                             smIdType ID)
{
  const SMDS_MeshNode *node1 = myNodeFactory->FindNode(idnode1);
  const SMDS_MeshNode *node2 = myNodeFactory->FindNode(idnode2);
  const SMDS_MeshNode *node3 = myNodeFactory->FindNode(idnode3);
  const SMDS_MeshNode *node4 = myNodeFactory->FindNode(idnode4);
  const SMDS_MeshNode *node5 = myNodeFactory->FindNode(idnode5);
  const SMDS_MeshNode *node6 = myNodeFactory->FindNode(idnode6);
  const SMDS_MeshNode *node7 = myNodeFactory->FindNode(idnode7);
  const SMDS_MeshNode *node8 = myNodeFactory->FindNode(idnode8);
  const SMDS_MeshNode *node9 = myNodeFactory->FindNode(idnode9);
  const SMDS_MeshNode *node10 = myNodeFactory->FindNode(idnode10);
  const SMDS_MeshNode *node11 = myNodeFactory->FindNode(idnode11);
  const SMDS_MeshNode *node12 = myNodeFactory->FindNode(idnode12);
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, node6,
                                    node7, node8, node9, node10, node11, node12,
                                    ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexagonal prism and add it to the mesh.
///@param ID The ID of the new volume
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n7,
                                            const SMDS_MeshNode * n8,
                                            const SMDS_MeshNode * n9,
                                            const SMDS_MeshNode * n10,
                                            const SMDS_MeshNode * n11,
                                            const SMDS_MeshNode * n12,
                                            smIdType ID)
{
  SMDS_MeshVolume* volume = 0;
  if(!n1 || !n2 || !n3 || !n4 || !n5 || !n6 ||
     !n7 || !n8 || !n9 || !n10 || !n11 || !n12 )
    return volume;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Hexagonal_Prism,
                /*nbNodes=*/12, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12 );
    myInfo.myNbHexPrism++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh.
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@return The created hexahedron
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n7,
                                      const SMDS_MeshNode * n8)
{
  smIdType ID = myCellFactory->GetFreeID();
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh.
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created hexahedron or NULL if an element with this ID already
///exists or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(smIdType idnode1,
                                             smIdType idnode2,
                                             smIdType idnode3,
                                             smIdType idnode4,
                                             smIdType idnode5,
                                             smIdType idnode6,
                                             smIdType idnode7,
                                             smIdType idnode8,
                                             smIdType ID)
{
  const SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6, *node7, *node8;
  node1 = myNodeFactory->FindNode(idnode1);
  node2 = myNodeFactory->FindNode(idnode2);
  node3 = myNodeFactory->FindNode(idnode3);
  node4 = myNodeFactory->FindNode(idnode4);
  node5 = myNodeFactory->FindNode(idnode5);
  node6 = myNodeFactory->FindNode(idnode6);
  node7 = myNodeFactory->FindNode(idnode7);
  node8 = myNodeFactory->FindNode(idnode8);
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, node6,
                                    node7, node8, ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh.
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created prism or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n7,
                                            const SMDS_MeshNode * n8,
                                            smIdType ID)
{
  SMDS_MeshVolume* volume = 0;
  if ( !n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8) return volume;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Hexa,
                /*nbNodes=*/8, n1, n2, n3, n4, n5, n6, n7, n8 );
    myInfo.myNbHexas++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a polygon defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddPolygonalFaceWithID (const std::vector<smIdType> & nodes_ids,
                                                  const smIdType               ID)
{
  size_t nbNodes = nodes_ids.size();
  std::vector<const SMDS_MeshNode*> nodes (nbNodes);
  for ( size_t i = 0; i < nbNodes; i++) {
    nodes[i] = myNodeFactory->FindNode( nodes_ids[i] );
    if (!nodes[i]) return NULL;
  }
  return SMDS_Mesh::AddPolygonalFaceWithID(nodes, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add a polygon defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace*
SMDS_Mesh::AddPolygonalFaceWithID (const std::vector<const SMDS_MeshNode*> & nodes,
                                   const smIdType                                 ID)
{
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( nodes.empty() )
    throw std::invalid_argument("Polygon without nodes is forbidden");
  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Polygon, nodes );
    myInfo.myNbPolygons++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a polygon defined by its nodes.
/// An ID is automatically affected to the created face.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddPolygonalFace (const std::vector<const SMDS_MeshNode*> & nodes)
{
  return SMDS_Mesh::AddPolygonalFaceWithID(nodes, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadratic polygon defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddQuadPolygonalFaceWithID (const std::vector<smIdType> & nodes_ids,
                                                      const smIdType                ID)
{
  std::vector<const SMDS_MeshNode*> nodes( nodes_ids.size() );
  for ( size_t i = 0; i < nodes.size(); i++) {
    nodes[i] = myNodeFactory->FindNode(nodes_ids[i]);
    if (!nodes[i]) return NULL;
  }
  return SMDS_Mesh::AddQuadPolygonalFaceWithID(nodes, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadratic polygon defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace*
SMDS_Mesh::AddQuadPolygonalFaceWithID (const std::vector<const SMDS_MeshNode*> & nodes,
                                       const smIdType                                 ID)
{
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();
  if ( nodes.empty() )
    throw std::invalid_argument("Polygon without nodes is forbidden");
  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Polygon, nodes );
    myInfo.myNbQuadPolygons++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadratic polygon defined by its nodes.
/// An ID is automatically affected to the created face.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddQuadPolygonalFace (const std::vector<const SMDS_MeshNode*> & nodes)
{
  return SMDS_Mesh::AddQuadPolygonalFaceWithID(nodes, myCellFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new polyhedral volume and add it to the mesh.
/// @param ID The ID of the new volume
/// @return The created volume or NULL if an element with this ID already exists
/// or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddPolyhedralVolumeWithID (const std::vector<smIdType> & nodes_ids,
                                                        const std::vector<int>      & quantities,
                                                        const smIdType                ID)
{
  size_t nbNodes = nodes_ids.size();
  std::vector<const SMDS_MeshNode*> nodes (nbNodes);
  for ( size_t i = 0; i < nbNodes; i++) {
    nodes[i] = myNodeFactory->FindNode(nodes_ids[i]);
    if (!nodes[i]) return NULL;
  }
  return SMDS_Mesh::AddPolyhedralVolumeWithID(nodes, quantities, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new polyhedral volume and add it to the mesh.
/// @param ID The ID of the new volume
/// @return The created  volume
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume*
SMDS_Mesh::AddPolyhedralVolumeWithID (const std::vector<const SMDS_MeshNode*>& nodes,
                                      const std::vector<int>                 & quantities,
                                      const smIdType                           ID)
{
  if ( nodes.empty() || quantities.empty() )
    return NULL;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    SMDS_MeshVolume* volume = static_cast<SMDS_MeshVolume*>( cell );
    volume->init( nodes, quantities );
    myInfo.myNbPolyhedrons++;
    return volume;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new polyhedral volume and add it to the mesh.
/// @return The created  volume
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddPolyhedralVolume
(const std::vector<const SMDS_MeshNode*> & nodes,
 const std::vector<int>                  & quantities)
{
  smIdType ID = myCellFactory->GetFreeID();
  return SMDS_Mesh::AddPolyhedralVolumeWithID(nodes, quantities, ID);
}

SMDS_MeshVolume* SMDS_Mesh::AddVolumeFromVtkIds(const std::vector<vtkIdType>& vtkNodeIds)
{
  SMDS_MeshCell*   cell = myCellFactory->NewCell( myCellFactory->GetFreeID() );
  SMDS_MeshVolume * vol = static_cast<SMDS_MeshVolume*>( cell );
  vol->init( vtkNodeIds );
  myInfo.add( cell );
  return vol;
}

SMDS_MeshFace* SMDS_Mesh::AddFaceFromVtkIds(const std::vector<vtkIdType>& vtkNodeIds)
{
  SMDS_MeshCell* cell = myCellFactory->NewCell( myCellFactory->GetFreeID() );
  SMDS_MeshFace *   f = static_cast<SMDS_MeshFace*>( cell );
  f->init( vtkNodeIds );
  myInfo.add( cell );
  return f;
}

//=======================================================================
//function : MoveNode
//purpose  : 
//=======================================================================

void SMDS_Mesh::MoveNode(const SMDS_MeshNode *n, double x, double y, double z)
{
  SMDS_MeshNode * node=const_cast<SMDS_MeshNode*>(n);
  node->setXYZ(x,y,z);
}

///////////////////////////////////////////////////////////////////////////////
/// Return the node whose SMDS ID is 'ID'.
///////////////////////////////////////////////////////////////////////////////
const SMDS_MeshNode * SMDS_Mesh::FindNode(smIdType ID) const
{
  return myNodeFactory->FindNode( ID );
}

///////////////////////////////////////////////////////////////////////////////
/// Return the node whose VTK ID is 'vtkId'.
///////////////////////////////////////////////////////////////////////////////
const SMDS_MeshNode * SMDS_Mesh::FindNodeVtk(vtkIdType vtkId) const
{
  return myNodeFactory->FindNode( vtkId + 1 );
}

const SMDS_MeshElement * SMDS_Mesh::FindElementVtk(vtkIdType IDelem) const
{
  return myCellFactory->FindElement( FromVtkToSmds( IDelem ));
}

///////////////////////////////////////////////////////////////////////////////
/// Remove a node and all the elements which own this node
///////////////////////////////////////////////////////////////////////////////

void SMDS_Mesh::RemoveNode(const SMDS_MeshNode * node)
{
  RemoveElement(node, true);
}

//=======================================================================
//function : RemoveFromParent
//purpose  :
//=======================================================================

bool SMDS_Mesh::RemoveFromParent()
{
  if (myParent==NULL) return false;
  else return (myParent->RemoveSubMesh(this));
}

//=======================================================================
//function : RemoveSubMesh
//purpose  :
//=======================================================================

bool SMDS_Mesh::RemoveSubMesh(const SMDS_Mesh * aMesh)
{
  bool found = false;

  std::list<SMDS_Mesh *>::iterator itmsh=myChildren.begin();
  for (; itmsh!=myChildren.end() && !found; itmsh++)
  {
    SMDS_Mesh * submesh = *itmsh;
    if (submesh == aMesh)
    {
      found = true;
      myChildren.erase(itmsh);
    }
  }

  return found;
}

//=======================================================================
//function : ChangePolyhedronNodes
//purpose  :
//=======================================================================

bool SMDS_Mesh::ChangePolyhedronNodes(const SMDS_MeshElement *                 element,
                                      const std::vector<const SMDS_MeshNode*>& nodes,
                                      const std::vector<int>&                  quantities)
{
  // keep current nodes of element
  std::set<const SMDS_MeshNode*> oldNodes( element->begin_nodes(), element->end_nodes() );

  // change nodes
  bool Ok = false;
  if ( const SMDS_MeshVolume* vol = DownCast<SMDS_MeshVolume>( element ))
    Ok = vol->ChangeNodes( nodes, quantities );

  if ( Ok )
  {
    setMyModified();
    updateInverseElements( element, &nodes[0], nodes.size(), oldNodes );
  }
  return Ok;
}

//=======================================================================
//function : ChangeElementNodes
//purpose  :
//=======================================================================

bool SMDS_Mesh::ChangeElementNodes(const SMDS_MeshElement * element,
                                   const SMDS_MeshNode    * nodes[],
                                   const int                nbnodes)
{
  // keep current nodes of element
  std::set<const SMDS_MeshNode*> oldNodes( element->begin_nodes(), element->end_nodes() );

  // change nodes
  bool Ok = false;
  if ( SMDS_MeshCell* cell = dynamic_cast<SMDS_MeshCell*>((SMDS_MeshElement*) element))
    Ok = cell->ChangeNodes(nodes, nbnodes);

  if ( Ok )
  {
    setMyModified();
    updateInverseElements( element, nodes, nbnodes, oldNodes );
  }
  return Ok;
}

//=======================================================================
//function : updateInverseElements
//purpose  : update InverseElements when element changes node
//=======================================================================

void SMDS_Mesh::updateInverseElements( const SMDS_MeshElement *        element,
                                       const SMDS_MeshNode* const*     nodes,
                                       const int                       nbnodes,
                                       std::set<const SMDS_MeshNode*>& oldNodes )
{
  if ( GetGrid()->HasLinks() ) // update InverseElements
  {
    std::set<const SMDS_MeshNode*>::iterator it;

    // AddInverseElement to new nodes
    for ( int i = 0; i < nbnodes; i++ )
    {
      it = oldNodes.find( nodes[i] );
      if ( it == oldNodes.end() )
        // new node
        const_cast<SMDS_MeshNode*>( nodes[i] )->AddInverseElement( element );
      else
        // remove from oldNodes a node that remains in elem
        oldNodes.erase( it );
    }
    // RemoveInverseElement from the nodes removed from elem
    for ( it = oldNodes.begin(); it != oldNodes.end(); it++ )
    {
      SMDS_MeshNode * n = const_cast<SMDS_MeshNode *>( *it );
      n->RemoveInverseElement( element );
    }
  }

}

const SMDS_Mesh0DElement* SMDS_Mesh::Find0DElement(const SMDS_MeshNode * node)
{
  if (!node) return 0;
  const SMDS_Mesh0DElement* toReturn = NULL;
  SMDS_ElemIteratorPtr it1 = node->GetInverseElementIterator(SMDSAbs_0DElement);
  while (it1->more() && (toReturn == NULL)) {
    const SMDS_MeshElement* e = it1->next();
    if (e->NbNodes() == 1) {
      toReturn = static_cast<const SMDS_Mesh0DElement*>(e);
    }
  }
  return toReturn;
}

const SMDS_BallElement* SMDS_Mesh::FindBall(const SMDS_MeshNode * node)
{
  if (!node) return 0;
  const SMDS_BallElement* toReturn = NULL;
  SMDS_ElemIteratorPtr it1 = node->GetInverseElementIterator(SMDSAbs_Ball);
  while (it1->more() && (toReturn == NULL)) {
    const SMDS_MeshElement* e = it1->next();
    if (e->GetGeomType() == SMDSGeom_BALL)
      toReturn = static_cast<const SMDS_BallElement*>(e);
  }
  return toReturn;
}

const SMDS_MeshEdge* SMDS_Mesh::FindEdge(const SMDS_MeshNode * node1,
                                         const SMDS_MeshNode * node2)
{
  if ( !node1 ) return 0;
  const SMDS_MeshEdge * toReturn=NULL;
  SMDS_ElemIteratorPtr it1=node1->GetInverseElementIterator(SMDSAbs_Edge);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 2 && e->GetNodeIndex( node2 ) >= 0 ) {
      toReturn = static_cast<const SMDS_MeshEdge*>( e );
      break;
    }
  }
  return toReturn;
}

const SMDS_MeshEdge* SMDS_Mesh::FindEdge(const SMDS_MeshNode * node1,
                                         const SMDS_MeshNode * node2,
                                         const SMDS_MeshNode * node3)
{
  if ( !node1 ) return 0;
  SMDS_ElemIteratorPtr it1 = node1->GetInverseElementIterator(SMDSAbs_Edge);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 3 ) {
      SMDS_ElemIteratorPtr it2 = e->nodesIterator();
      while(it2->more()) {
        const SMDS_MeshElement* n = it2->next();
        if( n!=node1 &&
            n!=node2 &&
            n!=node3 )
        {
          e = 0;
          break;
        }
      }
      if ( e )
        return static_cast<const SMDS_MeshEdge *> (e);
    }
  }
  return 0;
}

//=======================================================================
//function : FindFace
//purpose  :
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(const SMDS_MeshNode *node1,
                                         const SMDS_MeshNode *node2,
                                         const SMDS_MeshNode *node3)
{
  if ( !node1 ) return 0;
  SMDS_ElemIteratorPtr it1 = node1->GetInverseElementIterator(SMDSAbs_Face);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 3 ) {
      SMDS_ElemIteratorPtr it2 = e->nodesIterator();
      while(it2->more()) {
        const SMDS_MeshElement* n = it2->next();
        if( n!=node1 &&
            n!=node2 &&
            n!=node3 )
        {
          e = 0;
          break;
        }
      }
      if ( e )
        return static_cast<const SMDS_MeshFace *> (e);
    }
  }
  return 0;
}

//=======================================================================
//function : FindFace
//purpose  :
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(const SMDS_MeshNode *node1,
                                         const SMDS_MeshNode *node2,
                                         const SMDS_MeshNode *node3,
                                         const SMDS_MeshNode *node4)
{
  if ( !node1 ) return 0;
  SMDS_ElemIteratorPtr it1 = node1->GetInverseElementIterator(SMDSAbs_Face);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 4 ) {
      SMDS_ElemIteratorPtr it2 = e->nodesIterator();
      while(it2->more()) {
        const SMDS_MeshElement* n = it2->next();
        if( n!=node1 &&
            n!=node2 &&
            n!=node3 &&
            n!=node4 )
        {
          e = 0;
          break;
        }
      }
      if ( e )
        return static_cast<const SMDS_MeshFace *> (e);
    }
  }
  return 0;
}

//=======================================================================
//function : FindFace
//purpose  :quadratic triangle
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(const SMDS_MeshNode *node1,
                                         const SMDS_MeshNode *node2,
                                         const SMDS_MeshNode *node3,
                                         const SMDS_MeshNode *node4,
                                         const SMDS_MeshNode *node5,
                                         const SMDS_MeshNode *node6)
{
  if ( !node1 ) return 0;
  SMDS_ElemIteratorPtr it1 = node1->GetInverseElementIterator(SMDSAbs_Face);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 6 ) {
      SMDS_ElemIteratorPtr it2 = e->nodesIterator();
      while(it2->more()) {
        const SMDS_MeshElement* n = it2->next();
        if( n!=node1 &&
            n!=node2 &&
            n!=node3 &&
            n!=node4 &&
            n!=node5 &&
            n!=node6 )
        {
          e = 0;
          break;
        }
      }
      if ( e )
        return static_cast<const SMDS_MeshFace *> (e);
    }
  }
  return 0;
}


//=======================================================================
//function : FindFace
//purpose  : quadratic quadrangle
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(const SMDS_MeshNode *node1,
                                         const SMDS_MeshNode *node2,
                                         const SMDS_MeshNode *node3,
                                         const SMDS_MeshNode *node4,
                                         const SMDS_MeshNode *node5,
                                         const SMDS_MeshNode *node6,
                                         const SMDS_MeshNode *node7,
                                         const SMDS_MeshNode *node8)
{
  if ( !node1 ) return 0;
  SMDS_ElemIteratorPtr it1 = node1->GetInverseElementIterator(SMDSAbs_Face);
  while(it1->more()) {
    const SMDS_MeshElement * e = it1->next();
    if ( e->NbNodes() == 8 ) {
      SMDS_ElemIteratorPtr it2 = e->nodesIterator();
      while(it2->more()) {
        const SMDS_MeshElement* n = it2->next();
        if( n!=node1 &&
            n!=node2 &&
            n!=node3 &&
            n!=node4 &&
            n!=node5 &&
            n!=node6 &&
            n!=node7 &&
            n!=node8 )
        {
          e = 0;
          break;
        }
      }
      if ( e )
        return static_cast<const SMDS_MeshFace *> (e);
    }
  }
  return 0;
}


//=======================================================================
//function : FindElement
//purpose  :
//=======================================================================

const SMDS_MeshElement* SMDS_Mesh::FindElement(smIdType IDelem) const
{
  return myCellFactory->FindElement( IDelem );
}

//=======================================================================
//function : FindFace
//purpose  : find polygon
//=======================================================================


const SMDS_MeshFace* SMDS_Mesh::FindFace (const std::vector<const SMDS_MeshNode *>& nodes)
{
  return (const SMDS_MeshFace*) FindElement( nodes, SMDSAbs_Face );
}


//================================================================================
/*!
 * \brief Return element based on all given nodes
 *  \param nodes - node of element
 *  \param type - type of element
 *  \param noMedium - true if medium nodes of quadratic element are not included in <nodes>
 *  \retval const SMDS_MeshElement* - found element or NULL
 */
//================================================================================

const SMDS_MeshElement* SMDS_Mesh::FindElement (const std::vector<const SMDS_MeshNode *>& nodes,
                                                const SMDSAbs_ElementType            type,
                                                const bool                           noMedium)
{
  if ( nodes.size() > 0 && nodes[0] )
  {
    SMDS_ElemIteratorPtr itF = nodes[0]->GetInverseElementIterator(type);
    while (itF->more())
    {
      const SMDS_MeshElement* e = itF->next();
      int nbNodesToCheck = noMedium ? e->NbCornerNodes() : e->NbNodes();
      if ( nbNodesToCheck == (int)nodes.size() )
      {
        for ( size_t i = 1; e && i < nodes.size(); ++i )
        {
          int nodeIndex = e->GetNodeIndex( nodes[ i ]);
          if ( nodeIndex < 0 || nodeIndex >= nbNodesToCheck )
            e = 0;
        }
        if ( e )
          return e;
      }
    }
  }
  return NULL;
}

//================================================================================
/*!
 * \brief Return elements including all given nodes
 *  \param [in] nodes - nodes to find elements around
 *  \param [out] foundElems - the found elements
 *  \param [in] type - type of elements to find
 *  \return int - a number of found elements
 */
//================================================================================

int SMDS_Mesh::GetElementsByNodes(const std::vector<const SMDS_MeshNode *>& nodes,
                                  std::vector<const SMDS_MeshElement *>&    foundElems,
                                  const SMDSAbs_ElementType                 type)
{
  // chose a node with minimal number of inverse elements
  const SMDS_MeshNode* n0 = nodes[0];
  int minNbInverse = n0 ? n0->NbInverseElements( type ) : 1000;
  for ( size_t i = 1; i < nodes.size(); ++i )
    if ( nodes[i] && nodes[i]->NbInverseElements( type ) < minNbInverse )
    {
      n0 = nodes[i];
      minNbInverse = n0->NbInverseElements( type );
    }

  foundElems.clear();
  if ( n0 && minNbInverse > 0 )
  {
    foundElems.reserve( minNbInverse );
    SMDS_ElemIteratorPtr eIt = n0->GetInverseElementIterator( type );
    while ( eIt->more() )
    {
      const SMDS_MeshElement* e = eIt->next();
      bool includeAll = true;
      for ( size_t i = 0; i < nodes.size() &&  includeAll; ++i )
        if ( nodes[i] != n0 && e->GetNodeIndex( nodes[i] ) < 0 )
          includeAll = false;
      if ( includeAll )
        foundElems.push_back( e );
    }
  }
  return foundElems.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of nodes
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbNodes() const
{
  return myInfo.NbNodes();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of elements
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbElements() const
{
  return myInfo.NbElements();
}
///////////////////////////////////////////////////////////////////////////////
/// Return the number of 0D elements
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::Nb0DElements() const
{
  return myInfo.Nb0DElements();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of 0D elements
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbBalls() const
{
  return myInfo.NbBalls();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of edges (including construction edges)
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbEdges() const
{
  return myInfo.NbEdges();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of faces (including construction faces)
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbFaces() const
{
  return myInfo.NbFaces();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of volumes
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbVolumes() const
{
  return myInfo.NbVolumes();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of child mesh of this mesh.
/// Note that the tree structure of SMDS_Mesh is unused in SMESH
///////////////////////////////////////////////////////////////////////////////
smIdType SMDS_Mesh::NbSubMesh() const
{
  return myChildren.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Destroy the mesh and all its elements
/// All pointer on elements owned by this mesh become illegals.
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh::~SMDS_Mesh()
{
  std::list<SMDS_Mesh*>::iterator itc=myChildren.begin();
  while(itc!=myChildren.end())
  {
    delete *itc;
    itc++;
  }

  delete myNodeFactory;
  delete myCellFactory;

  myGrid->Delete();
}

//================================================================================
/*!
 * \brief Clear all data
 */
//================================================================================

void SMDS_Mesh::Clear()
{
  std::set< SMDS_ElementHolder* >::iterator holder = myElemHolders.begin();
  for ( ; holder != myElemHolders.end(); ++holder )
    (*holder)->clear();

  myNodeFactory->Clear();
  myCellFactory->Clear();

  std::list<SMDS_Mesh*>::iterator itc=myChildren.begin();
  while(itc!=myChildren.end())
    (*itc)->Clear();

  myModified = false;
  myModifTime++;
  xmin = 0; xmax = 0;
  ymin = 0; ymax = 0;
  zmin = 0; zmax = 0;

  myInfo.Clear();

  myGrid->Initialize();
  myGrid->Allocate();
  vtkPoints* points = vtkPoints::New();
  // rnv: to fix bug "21125: EDF 1233 SMESH: Degrardation of precision in a test case for quadratic conversion"
  // using double type for storing coordinates of nodes instead float.
  points->SetDataType(VTK_DOUBLE);
  points->SetNumberOfPoints( 0 );
  myGrid->SetPoints( points );
  points->Delete();
  myGrid->DeleteLinks();
}

///////////////////////////////////////////////////////////////////////////////
/// Return an iterator on nodes of the current mesh factory
///////////////////////////////////////////////////////////////////////////////

SMDS_NodeIteratorPtr SMDS_Mesh::nodesIterator() const
{
  return myNodeFactory->GetIterator< SMDS_NodeIterator >( new SMDS_MeshElement::NonNullFilter );
}

SMDS_ElemIteratorPtr SMDS_Mesh::elementGeomIterator(SMDSAbs_GeometryType type) const
{
  smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbElements( type );
  return myCellFactory->GetIterator< SMDS_ElemIterator >( new SMDS_MeshElement::GeomFilter( type ),
                                                          nbElems);
}

SMDS_ElemIteratorPtr SMDS_Mesh::elementEntityIterator(SMDSAbs_EntityType type) const
{
  if ( type == SMDSEntity_Node )
  {
    return myNodeFactory->GetIterator< SMDS_ElemIterator >( new SMDS_MeshElement::NonNullFilter );
  }
  smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbElements( type );
  return myCellFactory->GetIterator<SMDS_ElemIterator>( new SMDS_MeshElement::EntityFilter( type ),
                                                        nbElems);
}

///////////////////////////////////////////////////////////////////////////////
/// Return an iterator on elements of the current mesh factory
///////////////////////////////////////////////////////////////////////////////
SMDS_ElemIteratorPtr SMDS_Mesh::elementsIterator(SMDSAbs_ElementType type) const
{
  typedef SMDS_ElemIterator TIterator;
  switch ( type ) {

  case SMDSAbs_All:
    return myCellFactory->GetIterator< TIterator >( new SMDS_MeshElement::NonNullFilter );

  case SMDSAbs_Node:
    return myNodeFactory->GetIterator< TIterator >( new SMDS_MeshElement::NonNullFilter );

  default:
    smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbElements( type );
    return myCellFactory->GetIterator< TIterator >( new SMDS_MeshElement::TypeFilter( type ),
                                                    nbElems);
  }
  return SMDS_ElemIteratorPtr();
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on edges of the current mesh.
///////////////////////////////////////////////////////////////////////////////

SMDS_EdgeIteratorPtr SMDS_Mesh::edgesIterator() const
{
  typedef SMDS_EdgeIterator TIterator;
  smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbEdges();
  return myCellFactory->GetIterator< TIterator >( new SMDS_MeshElement::TypeFilter( SMDSAbs_Edge ),
                                                  nbElems);
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on faces of the current mesh.
///////////////////////////////////////////////////////////////////////////////

SMDS_FaceIteratorPtr SMDS_Mesh::facesIterator() const
{
  typedef SMDS_FaceIterator TIterator;
  smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbFaces();
  return myCellFactory->GetIterator< TIterator >( new SMDS_MeshElement::TypeFilter( SMDSAbs_Face ),
                                                  nbElems);
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on volumes of the current mesh.
///////////////////////////////////////////////////////////////////////////////

SMDS_VolumeIteratorPtr SMDS_Mesh::volumesIterator() const
{
  typedef SMDS_VolumeIterator TIterator;
  smIdType nbElems = myCellFactory->CompactChangePointers() ? -1 : myInfo.NbVolumes();
  return
    myCellFactory->GetIterator< TIterator >( new SMDS_MeshElement::TypeFilter( SMDSAbs_Volume ),
                                             nbElems );
}

SMDS_NodeIteratorPtr SMDS_Mesh::shapeNodesIterator(int                  shapeID,
                                                   size_t               nbElemsToReturn,
                                                   const SMDS_MeshNode* sm1stNode) const
{
  return myNodeFactory->GetShapeIterator< SMDS_NodeIterator >( shapeID, nbElemsToReturn, sm1stNode );
}

SMDS_ElemIteratorPtr SMDS_Mesh::shapeElementsIterator(int                     shapeID,
                                                      size_t                  nbElemsToReturn,
                                                      const SMDS_MeshElement* sm1stElem) const
{
  return myCellFactory->GetShapeIterator< SMDS_ElemIterator >( shapeID, nbElemsToReturn, sm1stElem );
}

///////////////////////////////////////////////////////////////////////////////
/// Do intersection of sets (more than 2)
///////////////////////////////////////////////////////////////////////////////
static std::set<const SMDS_MeshElement*> *
intersectionOfSets( std::set<const SMDS_MeshElement*> vs[], int numberOfSets )
{
  std::set<const SMDS_MeshElement*>* rsetA = new std::set<const SMDS_MeshElement*>(vs[0]);
  std::set<const SMDS_MeshElement*>* rsetB;

  for(int i=0; i<numberOfSets-1; i++)
  {
    rsetB = new std::set<const SMDS_MeshElement*>();
    set_intersection(rsetA->begin(), rsetA->end(),
                     vs[i+1].begin(), vs[i+1].end(),
                     inserter(*rsetB, rsetB->begin()));
    delete rsetA;
    rsetA=rsetB;
  }
  return rsetA;
}
///////////////////////////////////////////////////////////////////////////////
/// Return the list of finite elements owning the given element: elements
/// containing all the nodes of the given element, for instance faces and
/// volumes containing a given edge.
///////////////////////////////////////////////////////////////////////////////
static std::set<const SMDS_MeshElement*> * getFinitElements(const SMDS_MeshElement * element)
{
  int numberOfSets=element->NbNodes();
  std::set<const SMDS_MeshElement*> *initSet = new std::set<const SMDS_MeshElement*>[numberOfSets];

  SMDS_NodeIteratorPtr itNodes = element->nodeIterator();

  int i = 0;
  while ( itNodes->more() )
  {
    const SMDS_MeshNode *   n = itNodes->next();
    for ( SMDS_ElemIteratorPtr itFe = n->GetInverseElementIterator(); itFe->more(); )
      initSet[i].insert( itFe->next() );
    i++;
  }
  std::set<const SMDS_MeshElement*> *retSet = intersectionOfSets( initSet, numberOfSets );
  delete [] initSet;
  return retSet;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the std::list of nodes used only by the given elements
///////////////////////////////////////////////////////////////////////////////
static
std::set<const SMDS_MeshElement*> *getExclusiveNodes(std::set<const SMDS_MeshElement*>& elements)
{
  std::set<const SMDS_MeshElement*> *           toReturn = new std::set<const SMDS_MeshElement*>();
  std::set<const SMDS_MeshElement*>::iterator itElements = elements.begin();

  while( itElements != elements.end() )
  {
    SMDS_NodeIteratorPtr itNodes = (*itElements)->nodeIterator();
    itElements++;

    while( itNodes->more() )
    {
      const SMDS_MeshNode *   n = itNodes->next();
      SMDS_ElemIteratorPtr itFe = n->GetInverseElementIterator();
      std::set<const SMDS_MeshElement*> s;
      while ( itFe->more() )
        s.insert( itFe->next() );
      if ( s == elements ) toReturn->insert(n);
    }
  }
  return toReturn;
}

///////////////////////////////////////////////////////////////////////////////
///Find the children of an element that are made of given nodes
///@param setOfChildren The set in which matching children will be inserted
///@param element The element were to search matching children
///@param nodes The nodes that the children must have to be selected
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::addChildrenWithNodes(std::set<const SMDS_MeshElement*>& setOfChildren,
                                     const SMDS_MeshElement *           element,
                                     std::set<const SMDS_MeshElement*>& nodes)
{
  switch(element->GetType())
  {
  case SMDSAbs_Node:
    throw SALOME_Exception("Internal Error: This should not happen");
    break;
  case SMDSAbs_0DElement:
  case SMDSAbs_Ball:
  {
  }
  break;
  case SMDSAbs_Edge:
  {
    SMDS_ElemIteratorPtr itn=element->nodesIterator();
    while(itn->more())
    {
      const SMDS_MeshElement * e=itn->next();
      if(nodes.find(e)!=nodes.end())
      {
        setOfChildren.insert(element);
        break;
      }
    }
  } break;
  case SMDSAbs_Face:
  {
    SMDS_ElemIteratorPtr itn=element->nodesIterator();
    while(itn->more())
    {
      const SMDS_MeshElement * e=itn->next();
      if(nodes.find(e)!=nodes.end())
      {
        setOfChildren.insert(element);
        break;
      }
    }
  } break;
  case SMDSAbs_Volume:
  case SMDSAbs_NbElementTypes:
  case SMDSAbs_All: break;
  }
}

///////////////////////////////////////////////////////////////////////////////
///@param elem The element to delete
///@param removenodes if true remaining nodes will be removed
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::RemoveElement(const SMDS_MeshElement * elem,
                              const bool               removenodes)
{
  std::vector<const SMDS_MeshElement *> removedElems;
  std::vector<const SMDS_MeshElement *> removedNodes;
  RemoveElement( elem, removedElems, removedNodes, removenodes );
}

///////////////////////////////////////////////////////////////////////////////
///@param elem The element to delete
///@param removedElems to be filled with all removed elements
///@param removedNodes to be filled with all removed nodes
///@param removenodes if true remaining nodes will be removed
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::RemoveElement(const SMDS_MeshElement *               elem,
                              std::vector<const SMDS_MeshElement *>& removedElems,
                              std::vector<const SMDS_MeshElement *>& removedNodes,
                              bool                                   removenodes)
{
  // get finite elements built on elem
  std::set<const SMDS_MeshElement*> * s1;
  if (    (elem->GetType() == SMDSAbs_0DElement)
          ||  (elem->GetType() == SMDSAbs_Ball)
          ||  (elem->GetType() == SMDSAbs_Edge)
          ||  (elem->GetType() == SMDSAbs_Face)
          ||  (elem->GetType() == SMDSAbs_Volume) )
  {
    s1 = new std::set<const SMDS_MeshElement*> ();
    s1->insert(elem);
  }
  else
    s1 = getFinitElements(elem);

  // get exclusive nodes (which would become free afterwards)
  std::set<const SMDS_MeshElement*> * s2;
  if (elem->GetType() == SMDSAbs_Node) // a node is removed
  {
    // do not remove nodes except elem
    s2 = new std::set<const SMDS_MeshElement*> ();
    s2->insert(elem);
    removenodes = true;
  }
  else
    s2 = getExclusiveNodes(*s1);

  // form the set of finite and construction elements to remove
  std::set<const SMDS_MeshElement*> s3;
  std::set<const SMDS_MeshElement*>::iterator it = s1->begin();
  while (it != s1->end())
  {
    addChildrenWithNodes(s3, *it, *s2);
    s3.insert(*it);
    it++;
  }
  if (elem->GetType() != SMDSAbs_Node)
    s3.insert(elem);

  // remove finite and construction elements
  for( it = s3.begin();it != s3.end(); ++it )
  {
    // Remove element from <InverseElements> of its nodes
    SMDS_NodeIteratorPtr itn = (*it)->nodeIterator();
    while (itn->more())
    {
      SMDS_MeshNode * n = const_cast<SMDS_MeshNode *> (itn->next());
      n->RemoveInverseElement((*it));
    }

    vtkIdType vtkid = (*it)->GetVtkID();

    switch ((*it)->GetType()) {
    case SMDSAbs_Node:
      throw SALOME_Exception(LOCALIZED("Internal Error: This should not happen"));
      break;
    case SMDSAbs_Edge:      myInfo.RemoveEdge(*it);   break;
    case SMDSAbs_Face:      myInfo.RemoveFace(*it);   break;
    case SMDSAbs_Volume:    myInfo.RemoveVolume(*it); break;
    case SMDSAbs_Ball:      myInfo.myNbBalls--;       break;
    case SMDSAbs_0DElement: myInfo.myNb0DElements--;  break;
    case SMDSAbs_All: // avoid compilation warning
    case SMDSAbs_NbElementTypes: break;
    }
    removedElems.push_back( *it);

    myCellFactory->Free( static_cast< const SMDS_MeshCell*>( *it ));

    if (vtkid >= 0)
    {
      this->myGrid->GetCellTypesArray()->SetValue(vtkid, VTK_EMPTY_CELL);
    }
  }

  // remove exclusive (free) nodes
  if (removenodes)
  {
    for ( it = s2->begin(); it != s2->end(); ++it )
    {
      myInfo.myNbNodes--;
      myNodeFactory->Free( (*it) );
      removedNodes.push_back((*it));
    }
  }

  delete s2;
  delete s1;
}


///////////////////////////////////////////////////////////////////////////////
///@param elem The element to delete
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::RemoveFreeElement(const SMDS_MeshElement * elem)
{
  const vtkIdType     vtkId = elem->GetVtkID();
  SMDSAbs_ElementType aType = elem->GetType();
  if ( aType == SMDSAbs_Node )
  {
    // only free node can be removed by this method
    const SMDS_MeshNode* n = static_cast<const SMDS_MeshNode*>( elem );
    if ( n->NbInverseElements() == 0 ) { // free node
      myInfo.myNbNodes--;
      myNodeFactory->Free( n );
    }
    else
    {
      throw SALOME_Exception( LOCALIZED( "RemoveFreeElement: not a free node" ));
    }
  }
  else
  {
    // Remove element from <InverseElements> of its nodes
    SMDS_NodeIteratorPtr itn = elem->nodeIterator();
    while (itn->more()) {
      SMDS_MeshNode * n = const_cast<SMDS_MeshNode *>(itn->next());
      n->RemoveInverseElement(elem);
    }

    // in meshes without descendants elements are always free
    switch (aType) {
    case SMDSAbs_0DElement: myInfo.remove(elem);       break;
    case SMDSAbs_Edge:      myInfo.RemoveEdge(elem);   break;
    case SMDSAbs_Face:      myInfo.RemoveFace(elem);   break;
    case SMDSAbs_Volume:    myInfo.RemoveVolume(elem); break;
    case SMDSAbs_Ball:      myInfo.remove(elem);       break;
    default: break;
    }
    myCellFactory->Free( elem );

    this->myGrid->GetCellTypesArray()->SetValue(vtkId, VTK_EMPTY_CELL);
  }
}

//=======================================================================
/*!
 * Checks if the element is present in mesh.
 */
//=======================================================================

bool SMDS_Mesh::Contains (const SMDS_MeshElement* elem) const
{
  if ( !elem || elem->IsNull() )
    return false;

  if ( elem->GetType() == SMDSAbs_Node )
    return ( elem == myNodeFactory->FindElement( elem->GetID() ));

  return ( elem == myCellFactory->FindElement( elem->GetID() ));
}

//=======================================================================
//function : MaxNodeID
//purpose  :
//=======================================================================

smIdType SMDS_Mesh::MaxNodeID() const
{
  return myNodeFactory->GetMaxID();
}

//=======================================================================
//function : MinNodeID
//purpose  :
//=======================================================================

smIdType SMDS_Mesh::MinNodeID() const
{
  return myNodeFactory->GetMinID();
}

//=======================================================================
//function : MaxElementID
//purpose  :
//=======================================================================

smIdType SMDS_Mesh::MaxElementID() const
{
  return myCellFactory->GetMaxID();
}

//=======================================================================
//function : MinElementID
//purpose  :
//=======================================================================

smIdType SMDS_Mesh::MinElementID() const
{
  return myCellFactory->GetMinID();
}

//=======================================================================
//function : Renumber
//purpose  : Renumber all nodes or elements.
//=======================================================================

// void SMDS_Mesh::Renumber (const bool isNodes, const int  startID, const int  deltaID)
// {
//   if ( deltaID == 0 )
//     return;

// }

//=======================================================================
//function : GetElementType
//purpose  : Return type of element or node with id
//=======================================================================

SMDSAbs_ElementType SMDS_Mesh::GetElementType( const smIdType id, const bool iselem ) const
{
  const SMDS_MeshElement* elem = 0;
  if( iselem )
    elem = myCellFactory->FindElement( id );
  else
    elem = myNodeFactory->FindElement( id );

  return elem ? elem->GetType() : SMDSAbs_All;
}



//********************************************************************
//********************************************************************
//********                                                   *********
//*****       Methods for addition of quadratic elements        ******
//********                                                   *********
//********************************************************************
//********************************************************************

//=======================================================================
//function : AddEdgeWithID
//purpose  :
//=======================================================================
SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(smIdType n1, smIdType n2, smIdType n12, smIdType ID)
{
  return SMDS_Mesh::AddEdgeWithID (myNodeFactory->FindNode(n1),
                                   myNodeFactory->FindNode(n2),
                                   myNodeFactory->FindNode(n12),
                                   ID);
}

//=======================================================================
//function : AddEdge
//purpose  :
//=======================================================================
SMDS_MeshEdge* SMDS_Mesh::AddEdge(const SMDS_MeshNode* n1,
                                  const SMDS_MeshNode* n2,
                                  const SMDS_MeshNode* n12)
{
  return SMDS_Mesh::AddEdgeWithID(n1, n2, n12, myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddEdgeWithID
//purpose  :
//=======================================================================
SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n12,
                                        smIdType                   ID)
{
  if ( !n1 || !n2 || !n12 ) return 0;

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Edge, /*nbNodes=*/3, n1, n2, n12 );
    myInfo.myNbQuadEdges++;
    return static_cast<SMDS_MeshEdge*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3,
                                  const SMDS_MeshNode * n12,
                                  const SMDS_MeshNode * n23,
                                  const SMDS_MeshNode * n31)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3,n12,n23,n31,
                                  myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType n1, smIdType n2, smIdType n3,
                                        smIdType n12,smIdType n23,smIdType n31, smIdType ID)
{
  return SMDS_Mesh::AddFaceWithID (myNodeFactory->FindNode(n1) ,
                                   myNodeFactory->FindNode(n2) ,
                                   myNodeFactory->FindNode(n3) ,
                                   myNodeFactory->FindNode(n12),
                                   myNodeFactory->FindNode(n23),
                                   myNodeFactory->FindNode(n31),
                                   ID);
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n12,
                                        const SMDS_MeshNode * n23,
                                        const SMDS_MeshNode * n31,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n12 || !n23 || !n31 ) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Triangle, /*nbNodes=*/6, n1, n2, n3, n12, n23, n31 );
    myInfo.myNbQuadTriangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3,
                                  const SMDS_MeshNode * n12,
                                  const SMDS_MeshNode * n23,
                                  const SMDS_MeshNode * n31,
                                  const SMDS_MeshNode * nCenter)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3,n12,n23,n31,nCenter,
                                  myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType n1, smIdType n2, smIdType n3,
                                        smIdType n12,smIdType n23,smIdType n31, smIdType nCenter, smIdType ID)
{
  return SMDS_Mesh::AddFaceWithID (myNodeFactory->FindNode(n1) ,
                                   myNodeFactory->FindNode(n2) ,
                                   myNodeFactory->FindNode(n3) ,
                                   myNodeFactory->FindNode(n12),
                                   myNodeFactory->FindNode(n23),
                                   myNodeFactory->FindNode(n31),
                                   myNodeFactory->FindNode(nCenter),
                                   ID);
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n12,
                                        const SMDS_MeshNode * n23,
                                        const SMDS_MeshNode * n31,
                                        const SMDS_MeshNode * nCenter,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n12 || !n23 || !n31 || !nCenter) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_BiQuad_Triangle, /*nbNodes=*/7, n1, n2, n3, n12, n23, n31, nCenter );
    myInfo.myNbBiQuadTriangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3,
                                  const SMDS_MeshNode * n4,
                                  const SMDS_MeshNode * n12,
                                  const SMDS_MeshNode * n23,
                                  const SMDS_MeshNode * n34,
                                  const SMDS_MeshNode * n41)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3,n4,n12,n23,n34,n41,
                                  myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                        smIdType n12,smIdType n23,smIdType n34,smIdType n41, smIdType ID)
{
  return SMDS_Mesh::AddFaceWithID (myNodeFactory->FindNode(n1) ,
                                   myNodeFactory->FindNode(n2) ,
                                   myNodeFactory->FindNode(n3) ,
                                   myNodeFactory->FindNode(n4) ,
                                   myNodeFactory->FindNode(n12),
                                   myNodeFactory->FindNode(n23),
                                   myNodeFactory->FindNode(n34),
                                   myNodeFactory->FindNode(n41),
                                   ID);
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n4,
                                        const SMDS_MeshNode * n12,
                                        const SMDS_MeshNode * n23,
                                        const SMDS_MeshNode * n34,
                                        const SMDS_MeshNode * n41,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 || !n12 || !n23 || !n34 || !n41) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Quadrangle, /*nbNodes=*/8, n1, n2, n3, n4, n12, n23, n34, n41 );
    myInfo.myNbQuadQuadrangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}

//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
                                  const SMDS_MeshNode * n2,
                                  const SMDS_MeshNode * n3,
                                  const SMDS_MeshNode * n4,
                                  const SMDS_MeshNode * n12,
                                  const SMDS_MeshNode * n23,
                                  const SMDS_MeshNode * n34,
                                  const SMDS_MeshNode * n41,
                                  const SMDS_MeshNode * nCenter)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3,n4,n12,n23,n34,n41,nCenter,
                                  myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                        smIdType n12,smIdType n23,smIdType n34,smIdType n41, smIdType nCenter, smIdType ID)
{
  return SMDS_Mesh::AddFaceWithID (myNodeFactory->FindNode(n1) ,
                                   myNodeFactory->FindNode(n2) ,
                                   myNodeFactory->FindNode(n3) ,
                                   myNodeFactory->FindNode(n4) ,
                                   myNodeFactory->FindNode(n12),
                                   myNodeFactory->FindNode(n23),
                                   myNodeFactory->FindNode(n34),
                                   myNodeFactory->FindNode(n41),
                                   myNodeFactory->FindNode(nCenter),
                                   ID);
}

//=======================================================================
//function : AddFaceWithID
//purpose  :
//=======================================================================
SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n4,
                                        const SMDS_MeshNode * n12,
                                        const SMDS_MeshNode * n23,
                                        const SMDS_MeshNode * n34,
                                        const SMDS_MeshNode * n41,
                                        const SMDS_MeshNode * nCenter,
                                        smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 || !n12 || !n23 || !n34 || !n41 || !nCenter) return 0;
  if ( NbFaces() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_BiQuad_Quadrangle,
                /*nbNodes=*/9, n1, n2, n3, n4, n12, n23, n34, n41, nCenter );
    myInfo.myNbBiQuadQuadrangles++;
    return static_cast<SMDS_MeshFace*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n31,
                                      const SMDS_MeshNode * n14,
                                      const SMDS_MeshNode * n24,
                                      const SMDS_MeshNode * n34)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n12, n23,
                                    n31, n14, n24, n34, myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                            smIdType n12,smIdType n23,smIdType n31,
                                            smIdType n14,smIdType n24,smIdType n34, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1) ,
                                     myNodeFactory->FindNode(n2) ,
                                     myNodeFactory->FindNode(n3) ,
                                     myNodeFactory->FindNode(n4) ,
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n31),
                                     myNodeFactory->FindNode(n14),
                                     myNodeFactory->FindNode(n24),
                                     myNodeFactory->FindNode(n34),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order tetrahedron of 10 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n31,
                                            const SMDS_MeshNode * n14,
                                            const SMDS_MeshNode * n24,
                                            const SMDS_MeshNode * n34,
                                            smIdType ID)
{
  if ( !n1 || !n2 || !n3 || !n4 || !n12 || !n23 || !n31 || !n14 || !n24 || !n34)
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Tetra,
                /*nbNodes=*/10, n1, n2, n3, n4, n12, n23, n31, n14, n24, n34 );
    myInfo.myNbQuadTetras++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n34,
                                      const SMDS_MeshNode * n41,
                                      const SMDS_MeshNode * n15,
                                      const SMDS_MeshNode * n25,
                                      const SMDS_MeshNode * n35,
                                      const SMDS_MeshNode * n45)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n12, n23, n34, n41,
                                    n15, n25, n35, n45, myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType n5,
                                            smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                            smIdType n15,smIdType n25,smIdType n35,smIdType n45, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1) ,
                                     myNodeFactory->FindNode(n2) ,
                                     myNodeFactory->FindNode(n3) ,
                                     myNodeFactory->FindNode(n4) ,
                                     myNodeFactory->FindNode(n5) ,
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n34),
                                     myNodeFactory->FindNode(n41),
                                     myNodeFactory->FindNode(n15),
                                     myNodeFactory->FindNode(n25),
                                     myNodeFactory->FindNode(n35),
                                     myNodeFactory->FindNode(n45),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order pyramid of 13 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n34,
                                            const SMDS_MeshNode * n41,
                                            const SMDS_MeshNode * n15,
                                            const SMDS_MeshNode * n25,
                                            const SMDS_MeshNode * n35,
                                            const SMDS_MeshNode * n45,
                                            smIdType ID)
{
  if (!n1 || !n2 || !n3 || !n4 || !n5 || !n12 || !n23 ||
      !n34 || !n41 || !n15 || !n25 || !n35 || !n45)
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Pyramid,
                /*nbNodes=*/13, n1, n2, n3, n4, n5, n12, n23, n34, n41, n15, n25, n35, n45);
    myInfo.myNbQuadPyramids++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddVolume
//purpose  : 2d order Pentahedron (prism) with 15 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n31,
                                      const SMDS_MeshNode * n45,
                                      const SMDS_MeshNode * n56,
                                      const SMDS_MeshNode * n64,
                                      const SMDS_MeshNode * n14,
                                      const SMDS_MeshNode * n25,
                                      const SMDS_MeshNode * n36)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n12, n23, n31,
                                    n45, n56, n64, n14, n25, n36, myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Pentahedron (prism) with 15 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3,
                                            smIdType n4, smIdType n5, smIdType n6,
                                            smIdType n12,smIdType n23,smIdType n31,
                                            smIdType n45,smIdType n56,smIdType n64,
                                            smIdType n14,smIdType n25,smIdType n36, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1) ,
                                     myNodeFactory->FindNode(n2) ,
                                     myNodeFactory->FindNode(n3) ,
                                     myNodeFactory->FindNode(n4) ,
                                     myNodeFactory->FindNode(n5) ,
                                     myNodeFactory->FindNode(n6) ,
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n31),
                                     myNodeFactory->FindNode(n45),
                                     myNodeFactory->FindNode(n56),
                                     myNodeFactory->FindNode(n64),
                                     myNodeFactory->FindNode(n14),
                                     myNodeFactory->FindNode(n25),
                                     myNodeFactory->FindNode(n36),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Pentahedron (prism) with 15 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n31,
                                            const SMDS_MeshNode * n45,
                                            const SMDS_MeshNode * n56,
                                            const SMDS_MeshNode * n64,
                                            const SMDS_MeshNode * n14,
                                            const SMDS_MeshNode * n25,
                                            const SMDS_MeshNode * n36,
                                            smIdType ID)
{
  if (!n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n12 || !n23 ||
      !n31 || !n45 || !n56 || !n64 || !n14 || !n25 || !n36)
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Penta, /*nbNodes=*/15,
                n1, n2, n3, n4, n5, n6, n12, n23, n31, n45, n56, n64, n14, n25, n36 );
    myInfo.myNbQuadPrisms++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

//=======================================================================
//function : AddVolume
//purpose  : 2d order Pentahedron (prism) with 18 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n31,
                                      const SMDS_MeshNode * n45,
                                      const SMDS_MeshNode * n56,
                                      const SMDS_MeshNode * n64,
                                      const SMDS_MeshNode * n14,
                                      const SMDS_MeshNode * n25,
                                      const SMDS_MeshNode * n36,
                                      const SMDS_MeshNode * n1245,
                                      const SMDS_MeshNode * n2356,
                                      const SMDS_MeshNode * n1346)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n12, n23, n31,
                                    n45, n56, n64, n14, n25, n36, n1245, n2356, n1346,
                                    myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Pentahedron (prism) with 18 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3,
                                            smIdType n4, smIdType n5, smIdType n6,
                                            smIdType n12,smIdType n23,smIdType n31,
                                            smIdType n45,smIdType n56,smIdType n64,
                                            smIdType n14,smIdType n25,smIdType n36,
                                            smIdType n1245, smIdType n2356, smIdType n1346, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1) ,
                                     myNodeFactory->FindNode(n2) ,
                                     myNodeFactory->FindNode(n3) ,
                                     myNodeFactory->FindNode(n4) ,
                                     myNodeFactory->FindNode(n5) ,
                                     myNodeFactory->FindNode(n6) ,
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n31),
                                     myNodeFactory->FindNode(n45),
                                     myNodeFactory->FindNode(n56),
                                     myNodeFactory->FindNode(n64),
                                     myNodeFactory->FindNode(n14),
                                     myNodeFactory->FindNode(n25),
                                     myNodeFactory->FindNode(n36),
                                     myNodeFactory->FindNode(n1245),
                                     myNodeFactory->FindNode(n2356),
                                     myNodeFactory->FindNode(n1346),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Pentahedron (prism) with 18 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n31,
                                            const SMDS_MeshNode * n45,
                                            const SMDS_MeshNode * n56,
                                            const SMDS_MeshNode * n64,
                                            const SMDS_MeshNode * n14,
                                            const SMDS_MeshNode * n25,
                                            const SMDS_MeshNode * n36,
                                            const SMDS_MeshNode * n1245,
                                            const SMDS_MeshNode * n2356,
                                            const SMDS_MeshNode * n1346,
                                            smIdType ID)
{
  //MESSAGE("AddVolumeWithID penta18 "<< ID);
  if (!n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n12 || !n23 ||
      !n31 || !n45 || !n56 || !n64 || !n14 || !n25 || !n36 || !n1245 || !n2356 || !n1346)
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_BiQuad_Penta, /*nbNodes=*/18, n1, n2, n3, n4, n5, n6,
                n12, n23, n31, n45, n56, n64, n14, n25, n36, n1245, n2356, n1346 );
    myInfo.myNbBiQuadPrisms++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}


//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n7,
                                      const SMDS_MeshNode * n8,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n34,
                                      const SMDS_MeshNode * n41,
                                      const SMDS_MeshNode * n56,
                                      const SMDS_MeshNode * n67,
                                      const SMDS_MeshNode * n78,
                                      const SMDS_MeshNode * n85,
                                      const SMDS_MeshNode * n15,
                                      const SMDS_MeshNode * n26,
                                      const SMDS_MeshNode * n37,
                                      const SMDS_MeshNode * n48)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, n12, n23, n34, n41,
                                    n56, n67, n78, n85, n15, n26, n37, n48,
                                    myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                            smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                            smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                            smIdType n56,smIdType n67,smIdType n78,smIdType n85,
                                            smIdType n15,smIdType n26,smIdType n37,smIdType n48, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1),
                                     myNodeFactory->FindNode(n2),
                                     myNodeFactory->FindNode(n3),
                                     myNodeFactory->FindNode(n4),
                                     myNodeFactory->FindNode(n5),
                                     myNodeFactory->FindNode(n6),
                                     myNodeFactory->FindNode(n7),
                                     myNodeFactory->FindNode(n8),
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n34),
                                     myNodeFactory->FindNode(n41),
                                     myNodeFactory->FindNode(n56),
                                     myNodeFactory->FindNode(n67),
                                     myNodeFactory->FindNode(n78),
                                     myNodeFactory->FindNode(n85),
                                     myNodeFactory->FindNode(n15),
                                     myNodeFactory->FindNode(n26),
                                     myNodeFactory->FindNode(n37),
                                     myNodeFactory->FindNode(n48),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Hexahedrons with 20 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n7,
                                            const SMDS_MeshNode * n8,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n34,
                                            const SMDS_MeshNode * n41,
                                            const SMDS_MeshNode * n56,
                                            const SMDS_MeshNode * n67,
                                            const SMDS_MeshNode * n78,
                                            const SMDS_MeshNode * n85,
                                            const SMDS_MeshNode * n15,
                                            const SMDS_MeshNode * n26,
                                            const SMDS_MeshNode * n37,
                                            const SMDS_MeshNode * n48,
                                            smIdType ID)
{
  if (!n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n12 || !n23 ||
      !n34 || !n41 || !n56 || !n67 || !n78 || !n85 || !n15 || !n26 || !n37 || !n48)
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_Quad_Hexa, /*nbNodes=*/20, n1, n2, n3, n4, n5, n6, n7, n8,
                n12, n23, n34, n41, n56, n67, n78, n85, n15, n26, n37, n48 );
    myInfo.myNbQuadHexas++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

//=======================================================================
//function : AddVolume
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
                                      const SMDS_MeshNode * n2,
                                      const SMDS_MeshNode * n3,
                                      const SMDS_MeshNode * n4,
                                      const SMDS_MeshNode * n5,
                                      const SMDS_MeshNode * n6,
                                      const SMDS_MeshNode * n7,
                                      const SMDS_MeshNode * n8,
                                      const SMDS_MeshNode * n12,
                                      const SMDS_MeshNode * n23,
                                      const SMDS_MeshNode * n34,
                                      const SMDS_MeshNode * n41,
                                      const SMDS_MeshNode * n56,
                                      const SMDS_MeshNode * n67,
                                      const SMDS_MeshNode * n78,
                                      const SMDS_MeshNode * n85,
                                      const SMDS_MeshNode * n15,
                                      const SMDS_MeshNode * n26,
                                      const SMDS_MeshNode * n37,
                                      const SMDS_MeshNode * n48,
                                      const SMDS_MeshNode * n1234,
                                      const SMDS_MeshNode * n1256,
                                      const SMDS_MeshNode * n2367,
                                      const SMDS_MeshNode * n3478,
                                      const SMDS_MeshNode * n1458,
                                      const SMDS_MeshNode * n5678,
                                      const SMDS_MeshNode * nCenter)
{
  return SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, n12, n23, n34, n41,
                                    n56, n67, n78, n85, n15, n26, n37, n48,
                                    n1234, n1256, n2367, n3478, n1458, n5678, nCenter,
                                    myCellFactory->GetFreeID());
}

//=======================================================================
//function : AddVolumeWithID
//purpose  :
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                            smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                            smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                            smIdType n56,smIdType n67,smIdType n78,smIdType n85,
                                            smIdType n15,smIdType n26,smIdType n37,smIdType n48,
                                            smIdType n1234,smIdType n1256,smIdType n2367,smIdType n3478,
                                            smIdType n1458,smIdType n5678,smIdType nCenter, smIdType ID)
{
  return SMDS_Mesh::AddVolumeWithID (myNodeFactory->FindNode(n1),
                                     myNodeFactory->FindNode(n2),
                                     myNodeFactory->FindNode(n3),
                                     myNodeFactory->FindNode(n4),
                                     myNodeFactory->FindNode(n5),
                                     myNodeFactory->FindNode(n6),
                                     myNodeFactory->FindNode(n7),
                                     myNodeFactory->FindNode(n8),
                                     myNodeFactory->FindNode(n12),
                                     myNodeFactory->FindNode(n23),
                                     myNodeFactory->FindNode(n34),
                                     myNodeFactory->FindNode(n41),
                                     myNodeFactory->FindNode(n56),
                                     myNodeFactory->FindNode(n67),
                                     myNodeFactory->FindNode(n78),
                                     myNodeFactory->FindNode(n85),
                                     myNodeFactory->FindNode(n15),
                                     myNodeFactory->FindNode(n26),
                                     myNodeFactory->FindNode(n37),
                                     myNodeFactory->FindNode(n48),
                                     myNodeFactory->FindNode(n1234),
                                     myNodeFactory->FindNode(n1256),
                                     myNodeFactory->FindNode(n2367),
                                     myNodeFactory->FindNode(n3478),
                                     myNodeFactory->FindNode(n1458),
                                     myNodeFactory->FindNode(n5678),
                                     myNodeFactory->FindNode(nCenter),
                                     ID);
}

//=======================================================================
//function : AddVolumeWithID
//purpose  : 2d order Hexahedrons with 27 nodes
//=======================================================================
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n7,
                                            const SMDS_MeshNode * n8,
                                            const SMDS_MeshNode * n12,
                                            const SMDS_MeshNode * n23,
                                            const SMDS_MeshNode * n34,
                                            const SMDS_MeshNode * n41,
                                            const SMDS_MeshNode * n56,
                                            const SMDS_MeshNode * n67,
                                            const SMDS_MeshNode * n78,
                                            const SMDS_MeshNode * n85,
                                            const SMDS_MeshNode * n15,
                                            const SMDS_MeshNode * n26,
                                            const SMDS_MeshNode * n37,
                                            const SMDS_MeshNode * n48,
                                            const SMDS_MeshNode * n1234,
                                            const SMDS_MeshNode * n1256,
                                            const SMDS_MeshNode * n2367,
                                            const SMDS_MeshNode * n3478,
                                            const SMDS_MeshNode * n1458,
                                            const SMDS_MeshNode * n5678,
                                            const SMDS_MeshNode * nCenter,
                                            smIdType ID)
{
  if (!n1 || !n2 || !n3 || !n4 || !n5 || !n6 || !n7 || !n8 || !n12 || !n23 ||
      !n34 || !n41 || !n56 || !n67 || !n78 || !n85 || !n15 || !n26 || !n37 || !n48 ||
      !n1234 || !n1256 || !n2367 || !n3478 || !n1458 || !n5678 || !nCenter )
    return 0;
  if ( NbVolumes() % CHECKMEMORY_INTERVAL == 0 ) CheckMemory();

  if ( SMDS_MeshCell* cell = myCellFactory->NewCell( ID ))
  {
    cell->init( SMDSEntity_TriQuad_Hexa, /*nbNodes=*/27, n1, n2, n3, n4, n5, n6, n7, n8,
                n12, n23, n34, n41, n56, n67, n78, n85, n15, n26, n37, n48,
                n1234, n1256, n2367, n3478, n1458, n5678, nCenter);
    myInfo.myNbTriQuadHexas++;
    return static_cast<SMDS_MeshVolume*>( cell );
  }
  return 0;
}

void SMDS_Mesh::dumpGrid(std::string ficdump)
{
  //  vtkUnstructuredGridWriter* aWriter = vtkUnstructuredGridWriter::New();
  //  aWriter->SetFileName(ficdump.c_str());
  //  aWriter->SetInput(myGrid);
  //  if(myGrid->GetNumberOfCells())
  //  {
  //    aWriter->Write();
  //  }
  //  aWriter->Delete();
  ficdump = ficdump + "_connectivity";
  std::ofstream ficcon(ficdump.c_str(), ios::out);
  int nbPoints = myGrid->GetNumberOfPoints();
  ficcon << "-------------------------------- points " <<  nbPoints << endl;
  for (int i=0; i<nbPoints; i++)
  {
    ficcon << i << " " << *(myGrid->GetPoint(i)) << " " << *(myGrid->GetPoint(i)+1) << " " << " " << *(myGrid->GetPoint(i)+2) << endl;
  }
  int nbCells = myGrid->GetNumberOfCells();
  ficcon << "-------------------------------- cells " <<  nbCells << endl;
  for (vtkIdType i=0; i<nbCells; i++)
  {
    ficcon << i << " - " << myGrid->GetCell(i)->GetCellType() << " -";
    int nbptcell = myGrid->GetCell(i)->GetNumberOfPoints();
    vtkIdList *listid = myGrid->GetCell(i)->GetPointIds();
    for (int j=0; j<nbptcell; j++)
    {
      ficcon << " " <<  listid->GetId(j);
    }
    ficcon << endl;
  }
  ficcon << "-------------------------------- connectivity " <<  nbPoints << endl;
  vtkCellLinks *links = myGrid->GetLinks();
  for (int i=0; i<nbPoints; i++)
  {
    int ncells = links->GetNcells(i);
    vtkIdType *cells = links->GetCells(i);
    ficcon << i << " - " << ncells << " -";
    for (int j=0; j<ncells; j++)
    {
      ficcon << " " << cells[j];
    }
    ficcon << endl;
  }
  ficcon.close();

}

void SMDS_Mesh::CompactMesh()
{
  this->myCompactTime = this->myModifTime;

  bool idsChange = HasNumerationHoles();
  if ( idsChange )
  {
    std::set< SMDS_ElementHolder* >::iterator holder = myElemHolders.begin();
    for ( ; holder != myElemHolders.end(); ++holder )
      (*holder)->beforeCompacting();
  }
  smIdType oldCellSize = myCellFactory->GetMaxID();

  // remove "holes" in SMDS numeration
  std::vector<smIdType> idNodesOldToNew, idCellsNewToOld, idCellsOldToNew;
  myNodeFactory->Compact( idNodesOldToNew );
  myCellFactory->Compact( idCellsNewToOld );

  // make VTK IDs correspond to SMDS IDs
  smIdType newNodeSize = myNodeFactory->NbUsedElements();
  smIdType newCellSize = myCellFactory->NbUsedElements();
  myGrid->compactGrid( idNodesOldToNew, newNodeSize, idCellsNewToOld, newCellSize );

  if ( idsChange && !myElemHolders.empty() )
  {
    // idCellsNewToOld -> idCellsOldToNew
    idCellsOldToNew.resize( oldCellSize, oldCellSize );
    for ( size_t iNew = 0; iNew < idCellsNewToOld.size(); ++iNew )
    {
      if ( idCellsNewToOld[ iNew ] >= (smIdType) idCellsOldToNew.size() )
        idCellsOldToNew.resize( ( 1 + idCellsNewToOld[ iNew ]) * 1.5, oldCellSize );
      idCellsOldToNew[ idCellsNewToOld[ iNew ]] = iNew;
    }
  }

  std::set< SMDS_ElementHolder* >::iterator holder = myElemHolders.begin();
  for ( ; holder != myElemHolders.end(); ++holder )
    if ( idsChange )
      (*holder)->restoreElements( idNodesOldToNew, idCellsOldToNew );
    else
      (*holder)->compact();

  return;
}

smIdType SMDS_Mesh::FromVtkToSmds( vtkIdType vtkid ) const
{
  return myCellFactory->FromVtkToSmds( vtkid );
}

double SMDS_Mesh::getMaxDim()
{
  double dmax = 1.e-3;
  if ((xmax - xmin) > dmax) dmax = xmax -xmin;
  if ((ymax - ymin) > dmax) dmax = ymax -ymin;
  if ((zmax - zmin) > dmax) dmax = zmax -zmin;
  return dmax;
}

//! modification that needs compact structure and redraw
void SMDS_Mesh::Modified()
{
  if (this->myModified)
  {
    myGrid->Modified();
    this->myModifTime++;
    myModified = false;
  }
}

//! get last modification timeStamp
vtkMTimeType SMDS_Mesh::GetMTime() const
{
  return this->myModifTime;
}

bool SMDS_Mesh::IsCompacted()
{
  return ( this->myCompactTime == this->myModifTime );
}

//! are there holes in elements or nodes numeration
bool SMDS_Mesh::HasNumerationHoles()
{
  return ( myNodeFactory->CompactChangePointers() ||
           myCellFactory->CompactChangePointers() );
}

void SMDS_Mesh::setNbShapes( size_t nbShapes )
{
  myNodeFactory->SetNbShapes( nbShapes );
}
