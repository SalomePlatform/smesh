//  SMESH SMDS : implementaion of Salome mesh data structure
//
//  Copyright (C) 2003  OPEN CASCADE
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
//  See http://www.opencascade.org or email : webmaster@opencascade.org 
//
//
//
//  File   : SMDSControl.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
#include "SMDSControl.ixx"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_ListOfMeshElement.hxx"
#include "SMDS_ListIteratorOfListOfMeshElement.hxx"

//=======================================================================
//function : ComputeNeighborFaces
//purpose  : 
//=======================================================================

Standard_Integer SMDSControl::ComputeNeighborFaces(const Handle(SMDS_Mesh)& M,
						   const Handle(SMDS_MeshElement)& ME,
						   const Standard_Integer idnode1,
						   const Standard_Integer idnode2)
{
  const Handle(SMDS_MeshElement)& node1 = M->FindNode(idnode1);
  const Handle(SMDS_MeshElement)& node2 = M->FindNode(idnode2);
  const SMDS_ListOfMeshElement& lstInvFaces1 = node1->InverseElements();
  const SMDS_ListOfMeshElement& lstInvFaces2 = node2->InverseElements();
  Standard_Integer neighbor=0;

  SMDS_ListIteratorOfListOfMeshElement it1(lstInvFaces1);
  for (;it1.More();it1.Next()) {
    const Handle(SMDS_MeshElement)& face = it1.Value();
    if (M->Contains(face) && !face->IsSame(ME)) {
      if (face->IsNodeInElement(idnode1) && face->IsNodeInElement(idnode2)) {
	neighbor++;
      }
    }
  }

  if (neighbor > 0) {
    return neighbor;
  }

  SMDS_ListIteratorOfListOfMeshElement it2(lstInvFaces2);
  for (;it2.More();it2.Next()) {
    const Handle(SMDS_MeshElement)& face = it2.Value();
    if (M->Contains(face) && !face->IsSame(ME)) {
      if (face->IsNodeInElement(idnode1) && face->IsNodeInElement(idnode2)) {
	neighbor++;
      }
    }
  }

  return neighbor;
}

//=======================================================================
//function : ComputeNeighborVolumes
//purpose  : 
//=======================================================================

Standard_Integer SMDSControl::ComputeNeighborVolumes(const Handle(SMDS_Mesh)& M,
						     const Handle(SMDS_MeshElement)& ME,
						     const Standard_Integer idnode1,
						     const Standard_Integer idnode2,
						     const Standard_Integer idnode3)
{

  const Handle(SMDS_MeshElement)& node1 = M->FindNode(idnode1);
  const Handle(SMDS_MeshElement)& node2 = M->FindNode(idnode2);
  const Handle(SMDS_MeshElement)& node3 = M->FindNode(idnode3);
  const SMDS_ListOfMeshElement& lstInvVol1 = node1->InverseElements();
  const SMDS_ListOfMeshElement& lstInvVol2 = node2->InverseElements();
  const SMDS_ListOfMeshElement& lstInvVol3 = node3->InverseElements();
  
  Standard_Integer neighbor=0;

  SMDS_ListIteratorOfListOfMeshElement it1(lstInvVol1);
  for (;it1.More() && neighbor == 0;it1.Next()) {
    const Handle(SMDS_MeshElement)& vol = it1.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (vol->IsNodeInElement(idnode1)
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)) {
	neighbor++;
      }
    }
  }
  
  if (neighbor > 0) {
    return neighbor;
  }
  
  SMDS_ListIteratorOfListOfMeshElement it2(lstInvVol2);
  for (;it2.More() && neighbor == 0;it2.Next()) {
    const Handle(SMDS_MeshElement)& vol = it2.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (vol->IsNodeInElement(idnode1) 
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)) {
	neighbor++;
      }
    }
  }
  
  if (neighbor > 0) {
    return neighbor;
  }
  
  SMDS_ListIteratorOfListOfMeshElement it3(lstInvVol3);
  for (;it3.More() && neighbor == 0;it3.Next()) {
    const Handle(SMDS_MeshElement)& vol = it3.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (vol->IsNodeInElement(idnode1)
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)) {
	neighbor++;
      }
    }
  }
  
  return neighbor;

}

//=======================================================================
//function : ComputeNeighborVolumes
//purpose  : 
//=======================================================================

Standard_Integer SMDSControl::ComputeNeighborVolumes(const Handle(SMDS_Mesh)& M,
						     const Handle(SMDS_MeshElement)& ME,
						     const Standard_Integer idnode1,
						     const Standard_Integer idnode2,
						     const Standard_Integer idnode3,
						     const Standard_Integer idnode4)
{

  const Handle(SMDS_MeshElement)& node1 = M->FindNode(idnode1);
  const Handle(SMDS_MeshElement)& node2 = M->FindNode(idnode2);
  const Handle(SMDS_MeshElement)& node3 = M->FindNode(idnode3);
  const Handle(SMDS_MeshElement)& node4 = M->FindNode(idnode4);
  const SMDS_ListOfMeshElement& lstInvVol1 = node1->InverseElements();
  const SMDS_ListOfMeshElement& lstInvVol2 = node2->InverseElements();
  const SMDS_ListOfMeshElement& lstInvVol3 = node3->InverseElements();
  const SMDS_ListOfMeshElement& lstInvVol4 = node4->InverseElements();
  Standard_Integer neighbor=0;

  SMDS_ListIteratorOfListOfMeshElement it1(lstInvVol1);
  for (;it1.More();it1.Next()) {
    const Handle(SMDS_MeshElement)& vol = it1.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (   vol->IsNodeInElement(idnode1) 
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)
	  && vol->IsNodeInElement(idnode4)) {
	neighbor++;
      }
    }
  }

  if (neighbor >= 2) {
    return neighbor;
  }

  SMDS_ListIteratorOfListOfMeshElement it2(lstInvVol2);
  for (;it2.More();it2.Next()) {
    const Handle(SMDS_MeshElement)& vol = it2.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (   vol->IsNodeInElement(idnode1) 
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)
	  && vol->IsNodeInElement(idnode4)) {
	neighbor++;
      }
    }
  }

  if (neighbor >= 2) {
    return neighbor;
  }

  SMDS_ListIteratorOfListOfMeshElement it3(lstInvVol3);
  for (;it3.More();it3.Next()) {
    const Handle(SMDS_MeshElement)& vol = it3.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (   vol->IsNodeInElement(idnode1) 
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)
	  && vol->IsNodeInElement(idnode4)) {
	neighbor++;
      }
    }
  }

  if (neighbor >= 2) {
    return neighbor;
  }

  SMDS_ListIteratorOfListOfMeshElement it4(lstInvVol4);
  for (;it4.More();it4.Next()) {
    const Handle(SMDS_MeshElement)& vol = it4.Value();
    if (M->Contains(vol) && !vol->IsSame(ME)) {
      if (   vol->IsNodeInElement(idnode1) 
	  && vol->IsNodeInElement(idnode2)
	  && vol->IsNodeInElement(idnode3)
	  && vol->IsNodeInElement(idnode4)) {
	neighbor++;
      }
    }
  }

  return neighbor;
}
