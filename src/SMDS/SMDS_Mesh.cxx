using namespace std;
// File:	SMDS_Mesh.cxx
// Created:	Wed Jan 23 16:49:00 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


#include "SMDS_Mesh.ixx"
#include "SMDS_MapIteratorOfExtendedOrientedMap.hxx"
#include "SMDS_ListOfMeshElement.hxx"
#include "SMDS_ListIteratorOfListOfMeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshTriangle.hxx"
#include "SMDS_MeshQuadrangle.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_MeshTetrahedron.hxx"
#include "SMDS_MeshPyramid.hxx"
#include "SMDS_MeshPrism.hxx"
#include "SMDS_MeshHexahedron.hxx"
#include "SMDS_ListOfMesh.hxx"
#include "SMDS_ListIteratorOfListOfMesh.hxx"


#include <Standard_ErrorHandler.hxx>
#include <Standard_NoSuchObject.hxx>

#include "utilities.h"


//=======================================================================
//function : SMDS_Mesh
//purpose  : creation of a new mesh object
//=======================================================================

SMDS_Mesh::SMDS_Mesh(const Standard_Integer nbnodes,
		     const Standard_Integer nbedges, 
		     const Standard_Integer nbfaces,
		     const Standard_Integer nbvolumes)
  :myNodes(nbnodes),myEdges(nbedges),myFaces(nbfaces),myVolumes(nbvolumes),
   myNodeIDFactory(new SMDS_MeshNodeIDFactory()), 
   myElementIDFactory(new SMDS_MeshElementIDFactory()),myHasInverse(Standard_False)
{
}

//=======================================================================
//function : SMDS_Mesh
//purpose  :  
//=======================================================================

SMDS_Mesh::SMDS_Mesh(const Handle(SMDS_Mesh)& parent,
		     const Standard_Integer nbnodes)
  :myNodes(nbnodes),myParent(parent),myNodeIDFactory(parent->myNodeIDFactory),
   myElementIDFactory(parent->myElementIDFactory),
   myHasInverse(Standard_False)
{
}

//=======================================================================
//function : AddSubMesh
//purpose  : create an submesh
//=======================================================================

Handle(SMDS_Mesh) SMDS_Mesh::AddSubMesh()
{
  Handle(SMDS_Mesh) submesh = new SMDS_Mesh(this);
  if (!submesh.IsNull()) {
    myChildren.Append(submesh);
  }

  return submesh;
}


//=======================================================================
//function : AddNode
//purpose  : create a MeshNode and returns an ID
//=======================================================================

Standard_Integer SMDS_Mesh::AddNode(const Standard_Real x, 
				    const Standard_Real y, 
				    const Standard_Real z)
{
  Standard_Integer ID = myNodeIDFactory->GetFreeID();

  Handle(SMDS_MeshElement) node = new SMDS_MeshNode(ID,x,y,z);
  AddNode(node);

  return ID;
}

//=======================================================================
//function : AddNode
//purpose  : create a MeshNode. Returns False if the ID already exists
//=======================================================================

Standard_Boolean SMDS_Mesh::AddNodeWithID(const Standard_Real x, 
					  const Standard_Real y, 
					  const Standard_Real z,
					  const Standard_Integer ID)
{

  // find the MeshNode corresponding to ID
  Handle(SMDS_MeshElement) node;
  node = GetNode(ID);

  if (node.IsNull()) {
    node = new SMDS_MeshNode(ID,x,y,z);
    AddNode(node);
    return Standard_True;
  } else
    return Standard_False;
    
}

//=======================================================================
//function : AddNode
//purpose  : add  an existing node in  the mesh (useful for submesh)
//=======================================================================

Standard_Boolean SMDS_Mesh::AddNode(const Standard_Integer ID)
{
  // find the MeshNode corresponding to ID
  Handle(SMDS_MeshElement) node;

  node = GetNode(ID);

  if (!node.IsNull()) {
    myNodes.Add(node);
    return Standard_True;;
  } else
    return Standard_False;
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::AddNode(const Handle(SMDS_MeshElement)& node)
{

  if (!node.IsNull()) {
    myNodes.Add(node);
    if (!myParent.IsNull()) {
      myParent->AddNode(node);
    }
    return Standard_True;
  } else
    return Standard_False;
}


//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================

Standard_Integer SMDS_Mesh::AddEdge(const Standard_Integer idnode1,
					    const Standard_Integer idnode2)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddEdgeWithID(idnode1,idnode2,ID))
    return ID;
  else 
    return 0;

}

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::AddEdgeWithID(const Standard_Integer idnode1,
					  const Standard_Integer idnode2,
					  const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) edge,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      elem = CreateEdge(ID,idnode1,idnode2);
      edge = FindEdge(elem);
      if (edge.IsNull()) {
	edge = elem;
	myEdges.Add(edge);
      }
      successAdd = myElementIDFactory->BindID(ID,edge);
    }
  }
  
  return successAdd;

}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Integer SMDS_Mesh::AddFace(const Standard_Integer idnode1, 
				    const Standard_Integer idnode2,
				    const Standard_Integer idnode3)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddFaceWithID(idnode1,idnode2,idnode3,ID))
    return ID;
  else 
    return 0;

}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::AddFaceWithID(const Standard_Integer idnode1, 
					  const Standard_Integer idnode2,
					  const Standard_Integer idnode3,
					  const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) face,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	elem = CreateFace(ID,idnode1,idnode2,idnode3);
	face = FindFace(elem);
	if (face.IsNull()) {
	  face = elem;
	  myFaces.Add(face);
	}
	successAdd = myElementIDFactory->BindID(ID,face);
      }
    }
  }

  return successAdd;

}


//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Integer SMDS_Mesh::AddFace(const Standard_Integer idnode1, 
					    const Standard_Integer idnode2,
					    const Standard_Integer idnode3,
					    const Standard_Integer idnode4)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddFaceWithID(idnode1,idnode2,idnode3,idnode4,ID))
    return ID;
  else
    return 0;

}


//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::AddFaceWithID(const Standard_Integer idnode1, 
					  const Standard_Integer idnode2,
					  const Standard_Integer idnode3,
					  const Standard_Integer idnode4,
					  const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) face,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	// find the MeshNode corresponding to idnode4
	if (AddNode(idnode4)) {
	  elem = CreateFace(ID,idnode1,idnode2,idnode3,idnode4);
	  face = FindFace(elem);
	  if (face.IsNull()) {
	    face = elem;
	    myFaces.Add(face);
	  }
	  successAdd = myElementIDFactory->BindID(ID,face);
	}
      }
    }
  }

  return successAdd;

}



//=======================================================================
//function : AddVolume
//purpose  : Tetrahedra
//=======================================================================

Standard_Integer SMDS_Mesh::AddVolume(const Standard_Integer idnode1, 
				      const Standard_Integer idnode2,
				      const Standard_Integer idnode3,
				      const Standard_Integer idnode4)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddVolumeWithID(idnode1,idnode2,idnode3,idnode4,ID))
    return ID;
  else
    return 0;

}

//=======================================================================
//function : AddVolume
//purpose  : Tetrahedra
//=======================================================================

Standard_Boolean SMDS_Mesh::AddVolumeWithID(const Standard_Integer idnode1, 
					    const Standard_Integer idnode2,
					    const Standard_Integer idnode3,
					    const Standard_Integer idnode4,
					    const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) volume,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	// find the MeshNode corresponding to idnode4
	if (AddNode(idnode4)) {
	  elem = CreateVolume(ID,idnode1,idnode2,idnode3,idnode4);
	  volume = FindVolume(elem);
	  if (volume.IsNull()) {
	    volume = elem;
	    myVolumes.Add(volume);
	  }
	  successAdd = myElementIDFactory->BindID(ID,volume);
	}
      }
    }
  }

  return successAdd;

}

//=======================================================================
//function : AddVolume
//purpose  : Pyramid
//=======================================================================

Standard_Integer SMDS_Mesh::AddVolume(const Standard_Integer idnode1, 
				      const Standard_Integer idnode2,
				      const Standard_Integer idnode3,
				      const Standard_Integer idnode4,
				      const Standard_Integer idnode5)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddVolumeWithID(idnode1,idnode2,idnode3,idnode4,idnode5,ID))
    return ID;
  else
    return 0;

}

//=======================================================================
//function : AddVolume
//purpose  : Pyramid
//=======================================================================

Standard_Boolean SMDS_Mesh::AddVolumeWithID(const Standard_Integer idnode1, 
						    const Standard_Integer idnode2,
						    const Standard_Integer idnode3,
						    const Standard_Integer idnode4,
						    const Standard_Integer idnode5,
						    const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) volume,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	// find the MeshNode corresponding to idnode4
	if (AddNode(idnode4)) {
	  // find the MeshNode corresponding to idnode5
	  if (AddNode(idnode5)) {
	    elem = CreateVolume(ID,idnode1,idnode2,idnode3,idnode4,idnode5);
	    volume = FindVolume(elem);
	    if (volume.IsNull()) {
	      volume = elem;
	      myVolumes.Add(volume);
	    }
	    successAdd = myElementIDFactory->BindID(ID,volume);
	  }
	}
      }
    }
  }

  return successAdd;

}

//=======================================================================
//function : AddVolume
//purpose  : Prism
//=======================================================================

Standard_Integer SMDS_Mesh::AddVolume(const Standard_Integer idnode1, 
				      const Standard_Integer idnode2,
				      const Standard_Integer idnode3,
				      const Standard_Integer idnode4,
				      const Standard_Integer idnode5,
				      const Standard_Integer idnode6)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddVolumeWithID(idnode1,idnode2,idnode3,idnode4,
		      idnode5,idnode6,ID))
    return ID;
  else
    return 0;

}

//=======================================================================
//function : AddVolume
//purpose  : Prism
//=======================================================================

Standard_Boolean SMDS_Mesh::AddVolumeWithID(const Standard_Integer idnode1, 
					    const Standard_Integer idnode2,
					    const Standard_Integer idnode3,
					    const Standard_Integer idnode4,
					    const Standard_Integer idnode5,
					    const Standard_Integer idnode6,
					    const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) volume,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	// find the MeshNode corresponding to idnode4
	if (AddNode(idnode4)) {
	  // find the MeshNode corresponding to idnode5
	  if (AddNode(idnode5)) {
	    // find the MeshNode corresponding to idnode6
	    if (AddNode(idnode6)) {
	      elem = CreateVolume(ID,idnode1,idnode2,idnode3,idnode4,idnode5,idnode6);
	      volume = FindVolume(elem);
	      if (volume.IsNull()) {
		volume = elem;
		myVolumes.Add(volume);
	      }
	      successAdd = myElementIDFactory->BindID(ID,volume);
	    }
	  }
	}
      }
    }
  }

  return successAdd;

}

//=======================================================================
//function : AddVolume
//purpose  : Hexahedra
//=======================================================================

Standard_Integer SMDS_Mesh::AddVolume(const Standard_Integer idnode1, 
				      const Standard_Integer idnode2,
				      const Standard_Integer idnode3,
				      const Standard_Integer idnode4,
				      const Standard_Integer idnode5,
				      const Standard_Integer idnode6,
				      const Standard_Integer idnode7,
				      const Standard_Integer idnode8)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddVolumeWithID(idnode1,idnode2,idnode3,idnode4,
		      idnode5,idnode6,idnode7,idnode8,ID))
    return ID;
  else
    return 0;

}

//=======================================================================
//function : AddVolume
//purpose  : Hexahedra
//=======================================================================

Standard_Boolean SMDS_Mesh::AddVolumeWithID(const Standard_Integer idnode1, 
					    const Standard_Integer idnode2,
					    const Standard_Integer idnode3,
					    const Standard_Integer idnode4,
					    const Standard_Integer idnode5,
					    const Standard_Integer idnode6,
					    const Standard_Integer idnode7,
					    const Standard_Integer idnode8,
					    const Standard_Integer ID)
{
  Handle(SMDS_MeshElement) volume,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (AddNode(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (AddNode(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (AddNode(idnode3)) {
	// find the MeshNode corresponding to idnode4
	if (AddNode(idnode4)) {
	  // find the MeshNode corresponding to idnode5
	  if (AddNode(idnode5)) {
	    // find the MeshNode corresponding to idnode6
	    if (AddNode(idnode6)) {
	      // find the MeshNode corresponding to idnode7
	      if (AddNode(idnode7)) {
		// find the MeshNode corresponding to idnode8
		if (AddNode(idnode8)) {
		  elem = CreateVolume(ID,idnode1,idnode2,idnode3,idnode4,idnode5,
				      idnode6,idnode7,idnode8);
		  volume = FindVolume(elem);
		  if (volume.IsNull()) {
		    volume = elem;
		    myVolumes.Add(volume);
		  }
		  successAdd = myElementIDFactory->BindID(ID,volume);
		}
	      }
	    }
	  }
	}
      }
    }
  }

  return successAdd;

}


//=======================================================================
//function : GetNode
//purpose  : returns the MeshNode corresponding to the ID
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::GetNode(const Standard_Integer idnode) const
{

  Handle(SMDS_MeshElement) node;

  Handle(SMDS_MeshElement) elem = FindNode(idnode);
  if (!elem.IsNull()) { // found one correspondance
    node =  elem;
  } else {
    if (!myParent.IsNull())
      node = myParent->GetNode(idnode);
  }

  return node;
}

//=======================================================================
//function : FindNode
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindNode(const Standard_Integer ID) const
{
  Handle(SMDS_MeshElement) elem;
  if (myNodes.ContainsID(ID))
    elem = myNodes.FindID(ID);
  return elem;
}

//=======================================================================
//function : FindNode
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindNode(const Handle(SMDS_MeshElement)& node) const
{
  Handle(SMDS_MeshElement) elem;
  if (myNodes.Contains(node))
    elem = myNodes.Find(node);

  return elem;
}

//=======================================================================
//function : CreateEdge
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateEdge(const Standard_Integer ID,
					       const Standard_Integer idnode1,
					       const Standard_Integer idnode2) const
{
  Handle(SMDS_MeshEdge) edge = new SMDS_MeshEdge(ID,idnode1,idnode2);
  return edge;
}


//=======================================================================
//function : CreateFace
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateFace(const Standard_Integer ID,
					       const Standard_Integer idnode1,
					       const Standard_Integer idnode2,
					       const Standard_Integer idnode3) const
{
  Handle(SMDS_MeshFace) face = new SMDS_MeshTriangle(ID,idnode1,idnode2,idnode3);
  return face;
}


//=======================================================================
//function : CreateFace
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateFace(const Standard_Integer ID,
					       const Standard_Integer idnode1,
					       const Standard_Integer idnode2,
					       const Standard_Integer idnode3,
					       const Standard_Integer idnode4) const
{
  Handle(SMDS_MeshFace) face = new SMDS_MeshQuadrangle(ID,idnode1,idnode2,idnode3,idnode4);
  return face;
}

//=======================================================================
//function : CreateVolume
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateVolume(const Standard_Integer ID,
						 const Standard_Integer	idnode1,
						 const Standard_Integer	idnode2,
						 const Standard_Integer	idnode3,
						 const Standard_Integer	idnode4) const
{
  Handle(SMDS_MeshVolume) volume = new SMDS_MeshTetrahedron(ID,idnode1,idnode2,idnode3,idnode4);
  return volume;
}

//=======================================================================
//function : CreateVolume
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateVolume(const Standard_Integer ID,
						 const Standard_Integer	idnode1,
						 const Standard_Integer	idnode2,
						 const Standard_Integer	idnode3,
						 const Standard_Integer	idnode4,
						 const Standard_Integer	idnode5) const
{
  Handle(SMDS_MeshVolume) volume = new SMDS_MeshPyramid(ID,idnode1,idnode2,idnode3,idnode4,idnode5);
  return volume;
}

//=======================================================================
//function : CreateVolume
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateVolume(const Standard_Integer ID,
						 const Standard_Integer	idnode1,
						 const Standard_Integer	idnode2,
						 const Standard_Integer	idnode3,
						 const Standard_Integer	idnode4,
						 const Standard_Integer	idnode5,
						 const Standard_Integer	idnode6) const
{
  Handle(SMDS_MeshVolume) volume = new SMDS_MeshPrism(ID,idnode1,idnode2,idnode3,idnode4,idnode5,idnode6);
  return volume;
}

//=======================================================================
//function : CreateVolume
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::CreateVolume(const Standard_Integer ID,
						 const Standard_Integer	idnode1,
						 const Standard_Integer	idnode2,
						 const Standard_Integer	idnode3,
						 const Standard_Integer	idnode4,
						 const Standard_Integer	idnode5,
						 const Standard_Integer	idnode6,
						 const Standard_Integer	idnode7,
						 const Standard_Integer	idnode8) const
{
  Handle(SMDS_MeshVolume) volume = new SMDS_MeshHexahedron(ID,idnode1,idnode2,idnode3,idnode4,
							   idnode5,idnode6,idnode7,idnode8);
  return volume;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::Contains(const Handle(SMDS_MeshElement)& elem) const
{
  Standard_Boolean isinmesh = Standard_False;
  if (myNodes.Contains(elem))
    isinmesh = Standard_True;
  else if (myEdges.Contains(elem))
    isinmesh = Standard_True;
  else if (myFaces.Contains(elem))
    isinmesh = Standard_True;
  else if (myVolumes.Contains(elem))
    isinmesh = Standard_True;

  return isinmesh;
}

//=======================================================================
//function : FindEdge
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindEdge(const Handle(SMDS_MeshElement)& edge) const
{
  Handle(SMDS_MeshElement) elem;
  if (myEdges.Contains(edge))
    elem = myEdges.Find(edge);

  return elem;
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindFace(const Handle(SMDS_MeshElement)& face) const
{
  Handle(SMDS_MeshElement) elem;
  if (myFaces.Contains(face))
    elem = myFaces.Find(face);

  return elem;
}


//=======================================================================
//function : FindVolume
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindVolume(const Handle(SMDS_MeshElement)& volume) const
{
  Handle(SMDS_MeshElement) elem;
  if (myVolumes.Contains(volume))
    elem = myVolumes.Find(volume);

  return elem;
}


//=======================================================================
//function : FreeNode
//purpose  : 
//=======================================================================

void SMDS_Mesh::FreeNode(const Handle(SMDS_MeshElement)& node)
{
    myNodes.Remove(node);

    SMDS_ListIteratorOfListOfMesh itmsh(myChildren);
    for (;itmsh.More(); itmsh.Next()) {
      const Handle(SMDS_Mesh)& submesh = itmsh.Value();
      submesh->RemoveNode(node->GetID());
    }
}



//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveNode(const Standard_Integer IDnode)
{
  // find the MeshNode corresponding to IDnode
  Handle(SMDS_MeshElement) node = FindNode(IDnode);
  if (RemoveNode(node)) {
    if (myParent.IsNull()) { // if no parent we can release the ID
      myNodeIDFactory->ReleaseID(IDnode);
    }
  }
  
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::RemoveNode(const Handle(SMDS_MeshElement)& node)
{
  Standard_Boolean successRemove = Standard_False;

  if (!node.IsNull()) {
    if (myHasInverse && myNodes.Contains(node)) {
      SMDS_MapOfMeshOrientedElement map(1);
      BuildMapNodeAncestors(node,map);
      RemoveAncestors(node,map);
    }

    FreeNode(node);
    successRemove = Standard_True;
  }

  return successRemove;
}

//=======================================================================
//function : RemoveEdge
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveEdge(const Standard_Integer idnode1, const Standard_Integer idnode2)
{
  Handle(SMDS_MeshElement) edge = FindEdge(idnode1,idnode2);
  RemoveEdge(edge);
}

//=======================================================================
//function : RemoveEdge
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveEdge(const Handle(SMDS_MeshElement)& edge)
{

  if (!edge.IsNull()) {
    myEdges.Remove(edge);

    myElementIDFactory->ReleaseID(edge->GetID());
  }
}


//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveFace(const Standard_Integer idnode1, 
			   const Standard_Integer idnode2,
			   const Standard_Integer idnode3)
{
  Handle(SMDS_MeshElement) face = FindFace(idnode1,idnode2,idnode3);
  RemoveFace(face);
}

//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveFace(const Standard_Integer idnode1, 
			   const Standard_Integer idnode2,
			   const Standard_Integer idnode3,
			   const Standard_Integer idnode4)
{
  Handle(SMDS_MeshElement) face = FindFace(idnode1,idnode2,idnode3,idnode4);
  RemoveFace(face);
}


//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveFace(const Handle(SMDS_MeshElement)& face)
{
  if (!face.IsNull()) {
    myFaces.Remove(face);
    myElementIDFactory->ReleaseID(face->GetID());
  }
}

//=======================================================================
//function : RemoveVolume
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveVolume(const Handle(SMDS_MeshElement)& volume)
{
  if (myVolumes.Contains(volume)) {
    myVolumes.Remove(volume);
    myElementIDFactory->ReleaseID(volume->GetID());
  }
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveElement(const Standard_Integer IDelem,const Standard_Boolean removenodes)
{
  Handle(SMDS_MeshElement) elem =  myElementIDFactory->MeshElement(IDelem);
  RemoveElement(elem,removenodes);


    
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveElement(const Handle(SMDS_MeshElement)& elem,const Standard_Boolean removenodes)
{
  if ( elem->IsKind(STANDARD_TYPE(SMDS_MeshEdge)) ) {
    RemoveEdge(elem);
  } else if ( elem->IsKind(STANDARD_TYPE(SMDS_MeshNode))) {
    RemoveNode(elem);
    return;
  } else if ( elem->IsKind(STANDARD_TYPE(SMDS_MeshFace))) {
    RemoveFace(elem);
  } else if ( elem->IsKind(STANDARD_TYPE(SMDS_MeshVolume))) {
    RemoveVolume(elem);
  } else {
    MESSAGE( "remove function : unknown type" );
    return;
  }

  Standard_Integer nbcnx = elem->NbNodes();
  Standard_Integer i;
  for (i=1; i <= nbcnx; ++i) {
    RemoveInverseElement(GetNode(i,elem),elem);
    
  }

  if (removenodes) {
    for (i=1; i <= nbcnx; ++i) {
      if (GetNode(i,elem)->InverseElements().IsEmpty())
	FreeNode(GetNode(i,elem));
    }
  }

}
 
//=======================================================================
//function : RemoveFromParent
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::RemoveFromParent()
{
  if (myParent.IsNull())
    return Standard_False;

  return (myParent->RemoveSubMesh(this));

}

//=======================================================================
//function : RemoveSubMesh
//purpose  : 
//=======================================================================

Standard_Boolean SMDS_Mesh::RemoveSubMesh(const Handle(SMDS_Mesh)& aMesh)
{
  Standard_Boolean found = Standard_False;

  SMDS_ListIteratorOfListOfMesh itmsh(myChildren);
  for (;itmsh.More() && !found; itmsh.Next()) {
    Handle(SMDS_Mesh) submesh;
    submesh = itmsh.Value();
    if (submesh == aMesh) {
      found = Standard_True;
      myChildren.Remove(itmsh);
    }
  }

  return found;
}

//=======================================================================
//function : RemoveInverseElement
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveInverseElement(const Handle(SMDS_MeshElement)& elem, 
				     const Handle(SMDS_MeshElement)& parent) const
{
  if (!myHasInverse) 
    return;

  Handle(SMDS_MeshNode)& node = *((Handle(SMDS_MeshNode)*)&elem);
  node->RemoveInverseElement(parent);
}
 
//=======================================================================
//function : RemoveAncestors
//purpose  : 
//=======================================================================

void SMDS_Mesh::RemoveAncestors(const Handle(SMDS_MeshElement)& elem,
				const SMDS_MapOfMeshOrientedElement& map)
{

  if (!myHasInverse) 
    return;

  SMDS_MapIteratorOfExtendedOrientedMap itAnc(map);

  for (;itAnc.More();itAnc.Next()) {
    const Handle(SMDS_MeshElement)& ME = itAnc.Key();
    Standard_Integer nbcnx = ME->NbNodes();

    for (Standard_Integer i=1; i <= nbcnx; ++i) {
      RemoveInverseElement(GetNode(i,ME),ME);
    }
  }

  SMDS_MapIteratorOfExtendedOrientedMap itAnc2(map);

  for (;itAnc2.More();itAnc2.Next()) {
    const Handle(SMDS_MeshElement)& ME = itAnc2.Key();
    RemoveElement(ME);
  }
}

//=======================================================================
//function : BuildMapNodeAncestors
//purpose  : 
//=======================================================================

void SMDS_Mesh::BuildMapNodeAncestors(const Handle(SMDS_MeshElement)& ME, 
				      SMDS_MapOfMeshOrientedElement& map) const
{

  if (!myHasInverse) 
    return;

  Standard_Integer nbcnx = ME->NbNodes();

  for (Standard_Integer i=1; i <= nbcnx; ++i) {
    const SMDS_ListOfMeshElement& lstInvElements = GetNode(i,ME)->InverseElements();

    SMDS_ListIteratorOfListOfMeshElement it(lstInvElements);
    for (;it.More();it.Next()) {
      const Handle(SMDS_MeshElement)& meParent = it.Value();
      if (Contains(meParent))
	map.Add(meParent);
    }

  }

}


//=======================================================================
//function : BuildMapEdgeAncestors
//purpose  : 
//=======================================================================

void SMDS_Mesh::BuildMapEdgeAncestors(const Handle(SMDS_MeshElement)& ME, 
				      SMDS_MapOfMeshOrientedElement& map) const
{

  if (!myHasInverse) 
    return;

  Standard_Integer nbcnx = ME->NbNodes();

  for (Standard_Integer i=1; i <= nbcnx; ++i) {
    const SMDS_ListOfMeshElement& lstInvElements = GetNode(i,ME)->InverseElements();

    SMDS_ListIteratorOfListOfMeshElement it(lstInvElements);
    for (;it.More();it.Next()) {
      const Handle(SMDS_MeshElement)& meParent = it.Value();
      if ( !meParent->IsKind(STANDARD_TYPE(SMDS_MeshEdge)) && Contains(meParent))
	map.Add(meParent);
    }

  }

}


//=======================================================================
//function : BuildMapFaceAncestors
//purpose  : 
//=======================================================================

void SMDS_Mesh::BuildMapFaceAncestors(const Handle(SMDS_MeshElement)& ME, 
				      SMDS_MapOfMeshOrientedElement& map) const
{

  if (!myHasInverse) 
    return;

  Standard_Integer nbcnx = ME->NbNodes();

  for (Standard_Integer i=1; i <= nbcnx; ++i) {
    const SMDS_ListOfMeshElement& lstInvElements = GetNode(i,ME)->InverseElements();

    SMDS_ListIteratorOfListOfMeshElement it(lstInvElements);
    for (;it.More();it.Next()) {
      const Handle(SMDS_MeshElement)& meParent = it.Value();
      if ( !meParent->IsKind(STANDARD_TYPE(SMDS_MeshEdge))
	   && ( !meParent->IsKind(STANDARD_TYPE(SMDS_MeshFace))) && Contains(meParent) )
	map.Add(meParent);
    }

  }

}


//=======================================================================
//function : FindEdge
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindEdge(const Standard_Integer idnode1,
					     const Standard_Integer idnode2 ) const
{
  Handle(SMDS_MeshEdge) edge = new SMDS_MeshEdge(0,idnode1,idnode2);
  return FindEdge(edge);
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindFace(const Standard_Integer idnode1,
					     const Standard_Integer idnode2,
					     const Standard_Integer idnode3 ) const
{
  Handle(SMDS_MeshFace) face = new SMDS_MeshTriangle(0,idnode1,idnode2,idnode3);
  return FindFace(face);
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindFace(const Standard_Integer idnode1,
					     const Standard_Integer idnode2,
					     const Standard_Integer idnode3,
					     const Standard_Integer idnode4 ) const
{
  Handle(SMDS_MeshFace) face = new SMDS_MeshQuadrangle(0,idnode1,idnode2,idnode3,idnode4);
  return FindFace(face);
}

//=======================================================================
//function : FindElement
//purpose  : 
//=======================================================================

Handle(SMDS_MeshElement) SMDS_Mesh::FindElement(const Standard_Integer IDelem) const
{
  return myElementIDFactory->MeshElement(IDelem);
}

//=======================================================================
//function : GetNode
//purpose  : 
//=======================================================================

Handle(SMDS_MeshNode) SMDS_Mesh::GetNode(const Standard_Integer rank, 
					 const Handle(SMDS_MeshElement)& ME) const

{
  const Standard_Integer idnode = ME->GetConnection(rank); // take care, no control of bounds

  Handle(SMDS_MeshElement) elem = FindNode(idnode);
  Handle(SMDS_MeshNode)& node = *((Handle(SMDS_MeshNode)*)&elem);
  return node;

}


//=======================================================================
//function : DumpNodes
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpNodes() const
{
  MESSAGE( "dump nodes of mesh : " );

  SMDS_MapIteratorOfExtendedOrientedMap itnode(myNodes);

  for (;itnode.More();itnode.Next()) {
    const Handle(SMDS_MeshElement)& node = itnode.Key();
    MESSAGE( node);
    
  }

}



//=======================================================================
//function : DumpEdges
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpEdges() const
{
  MESSAGE( "dump edges of mesh : " );

  SMDS_MapIteratorOfExtendedOrientedMap itedge(myEdges);

  for (;itedge.More();itedge.Next()) {
    const Handle(SMDS_MeshElement)& edge = itedge.Key();
    MESSAGE( edge);
  }
}



//=======================================================================
//function : DumpFaces
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpFaces() const
{
  MESSAGE( "dump faces of mesh : " );

  SMDS_MapIteratorOfExtendedOrientedMap itface(myFaces);

  for (;itface.More();itface.Next()) {
    const Handle(SMDS_MeshElement)& face = itface.Key();
    MESSAGE( face);
  }
}


//=======================================================================
//function : DumpVolumes
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpVolumes() const
{
  MESSAGE( "dump volumes of mesh : " );

  SMDS_MapIteratorOfExtendedOrientedMap itvol(myVolumes);

  for (;itvol.More();itvol.Next()) {
    const Handle(SMDS_MeshElement)& volume = itvol.Key();
    MESSAGE( volume);
  }
}



//=======================================================================
//function : DebugStats
//purpose  : 
//=======================================================================

void SMDS_Mesh::DebugStats() const
{
  //VRV: T2.4 impossible to use Logger server
  MESSAGE( "Debug stats of mesh : " );

  MESSAGE( "===== NODES =====" );
  myNodes.Statistics(cout);

  MESSAGE( "===== EDGES =====" );
  myEdges.Statistics(cout);

  MESSAGE( "===== FACES =====" );
  myFaces.Statistics(cout);

  MESSAGE( "===== VOLUMES =====" );
  myVolumes.Statistics(cout);
  //VRV: T2.4 impossible to use Logger server

  MESSAGE( "End Debug stats of mesh " );

  //#ifdef DEB
  SMDS_MapIteratorOfExtendedOrientedMap itnode(myNodes);
  Standard_Integer sizeofnodes = 0;
  Standard_Integer sizeoffaces = 0;


  for (;itnode.More();itnode.Next()) {
    const Handle(SMDS_MeshElement)& node = itnode.Key();
    
    Standard_Transient *p = node->This();
    sizeofnodes += sizeof( *((SMDS_MeshNode *)p) );

    SMDS_ListIteratorOfListOfMeshElement it(node->InverseElements());
    for (;it.More();it.Next()) {
      const Handle(SMDS_MeshElement)& me = it.Value();
      sizeofnodes += sizeof(me);
    }
    
  }

  SMDS_MapIteratorOfExtendedOrientedMap itface(myFaces);

  for (;itface.More();itface.Next()) {
    const Handle(SMDS_MeshElement)& face = itface.Key();
    
    Standard_Transient *p = face->This();
    sizeoffaces += sizeof( *((SMDS_MeshFace *)p) );
    
  }
  MESSAGE( "total size of node elements = " << sizeofnodes );;
  MESSAGE( "total size of face elements = " << sizeoffaces );;

  //#endif

}

//=======================================================================
//function : RebuildAllInverseConnections
//purpose  : 
//=======================================================================

void SMDS_Mesh::RebuildAllInverseConnections()
{
  if (!myParent.IsNull())
    myParent->RebuildAllInverseConnections();

  else {
    // Clear all inverseconnections from nodes
    SMDS_MapIteratorOfExtendedOrientedMap itnode(myNodes);
    
    for (;itnode.More();itnode.Next()) {
      const Handle(SMDS_MeshElement)& elem = itnode.Key();
      elem->ClearInverseElements();
    }
    

    RebuildInverseConnections();

    SMDS_ListIteratorOfListOfMesh itmsh(myChildren);
    for (;itmsh.More(); itmsh.Next()) {
      Handle(SMDS_Mesh) submesh;
      submesh = itmsh.Value();
    
      submesh->RebuildInverseConnections();

    }
  }
}

//=======================================================================
//function : RebuildInverseConnections
//purpose  : 
//=======================================================================

void SMDS_Mesh::RebuildInverseConnections()
{
  // rebuld inverse connections to volumes
  SMDS_MapIteratorOfExtendedOrientedMap itvol(myVolumes);

  for (;itvol.More();itvol.Next()) {
    const Handle(SMDS_MeshElement)& vol = itvol.Key();

    Standard_Integer nbcnx = vol->NbNodes();
    for (Standard_Integer inode=1; inode<=nbcnx; ++inode) {
      Standard_Integer idnode = vol->GetConnection(inode);
      Handle(SMDS_MeshElement) node = FindNode(idnode);
      if (!node.IsNull())
	node->AddInverseElement(vol);
    }
    
  }

  // rebuld inverse connections to faces
  SMDS_MapIteratorOfExtendedOrientedMap itface(myFaces);

  for (;itface.More();itface.Next()) {
    const Handle(SMDS_MeshElement)& face = itface.Key();

    Standard_Integer nbcnx = face->NbNodes();
    for (Standard_Integer inode=1; inode<=nbcnx; ++inode) {
      Standard_Integer idnode = face->GetConnection(inode);
      Handle(SMDS_MeshElement) node = FindNode(idnode);
      if (!node.IsNull())
	node->AddInverseElement(face);
    }
    
  }

  // rebuld inverse connections to edges
  SMDS_MapIteratorOfExtendedOrientedMap itedge(myEdges);

  for (;itedge.More();itedge.Next()) {
    const Handle(SMDS_MeshElement)& edge = itedge.Key();

    Standard_Integer nbcnx = edge->NbNodes();
    for (Standard_Integer inode=1; inode<=nbcnx; ++inode) {
      Standard_Integer idnode = edge->GetConnection(inode);
      Handle(SMDS_MeshElement) node = FindNode(idnode);
      if (!node.IsNull())
	node->AddInverseElement(edge);
    }
    
  }

  myHasInverse = Standard_True;
}


//=======================================================================
//function : SubMeshIterator
//purpose  : returns the ith SubMesh
//=======================================================================

void SMDS_Mesh::SubMeshIterator(SMDS_ListIteratorOfListOfMesh& itmsh) const
{
  itmsh.Initialize(myChildren);
}

