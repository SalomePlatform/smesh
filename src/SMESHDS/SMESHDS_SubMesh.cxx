//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESH_SubMesh.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

using namespace std;
#include "SMESHDS_SubMesh.hxx"

//=======================================================================
//function : AddElement
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddElement(const SMDS_MeshElement * ME)
{
	myElements.insert(ME);
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================
bool SMESHDS_SubMesh::RemoveElement(const SMDS_MeshElement * ME)
{
  if ( NbElements() )
    return myElements.erase(ME);
  
  return false;
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
void SMESHDS_SubMesh::AddNode(const SMDS_MeshNode * N)
{
	myNodes.insert(N);
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
bool SMESHDS_SubMesh::RemoveNode(const SMDS_MeshNode * N)
{
  if ( NbNodes() )
    return myNodes.erase(N);

  return false;
}

//=======================================================================
//function : NbElements
//purpose  : 
//=======================================================================
int SMESHDS_SubMesh::NbElements() const
{
	return myElements.size();
}

//=======================================================================
//function : NbNodes
//purpose  : 
//=======================================================================
int SMESHDS_SubMesh::NbNodes() const
{
	return myNodes.size();
}

template<typename T> class MySetIterator:public SMDS_Iterator<const T*>
{
  typedef const set<const T*> TSet;
  typename TSet::const_iterator myIt;
  TSet& mySet;

  public:
	MySetIterator(const set<const T*>& s):mySet(s), myIt(s.begin())
	{
	}

	bool more()
	{
		return myIt!=mySet.end();
	}
	const T* next()
	{
		const T* t=*myIt;
		myIt++;
		return t;			
	}
};
///////////////////////////////////////////////////////////////////////////////
///Return an iterator on the elements of submesh
///The created iterator must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_ElemIteratorPtr SMESHDS_SubMesh::GetElements() const
{
  return SMDS_ElemIteratorPtr(new MySetIterator<SMDS_MeshElement>(myElements));
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on the nodes of submesh
///The created iterator must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_NodeIteratorPtr SMESHDS_SubMesh::GetNodes() const
{
  return SMDS_NodeIteratorPtr(new MySetIterator<SMDS_MeshNode>(myNodes));
}

