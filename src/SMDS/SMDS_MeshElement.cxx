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

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "utilities.h"

SMDS_MeshElement::SMDS_MeshElement(int ID):myID(ID)
{
}

void SMDS_MeshElement::Print(ostream & OS) const
{
	OS << "dump of mesh element" << endl;
}

ostream & operator <<(ostream & OS, const SMDS_MeshElement * ME)
{
	ME->Print(OS);
	return OS;
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on nodes owned by the element.
/// This method call elementsIterator().
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshElement *> * SMDS_MeshElement::nodesIterator() const
{
	return elementsIterator(SMDSAbs_Node);
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on edges linked with or owned by the element.
/// This method call elementsIterator().
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshElement *> * SMDS_MeshElement::edgesIterator() const
{
	return elementsIterator(SMDSAbs_Edge);
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on faces linked with or owned by the element.
/// This method call elementsIterator().
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshElement *> * SMDS_MeshElement::facesIterator() const
{
	return elementsIterator(SMDSAbs_Face);
}

///////////////////////////////////////////////////////////////////////////////
///Return The number of nodes owned by the current element
///////////////////////////////////////////////////////////////////////////////
int SMDS_MeshElement::NbNodes() const
{
	int nbnodes=0;
	SMDS_Iterator<const SMDS_MeshElement *> * it=nodesIterator();
	while(it->more())
	{
		it->next();
		nbnodes++;
	}
	delete it;
	return nbnodes;
}

///////////////////////////////////////////////////////////////////////////////
///Return the number of edges owned by or linked with the current element
///////////////////////////////////////////////////////////////////////////////
int SMDS_MeshElement::NbEdges() const
{
	int nbedges=0;
	SMDS_Iterator<const SMDS_MeshElement *> * it=edgesIterator();
	while(it->more())
	{
		it->next();
		nbedges++;
	}
	delete it;
	return nbedges;
}

///////////////////////////////////////////////////////////////////////////////
///Return the number of faces owned by or linked with the current element
///////////////////////////////////////////////////////////////////////////////
int SMDS_MeshElement::NbFaces() const
{
	int nbfaces=0;
	SMDS_Iterator<const SMDS_MeshElement *> * it=facesIterator();
	while(it->more())
	{
		it->next();
		nbfaces++;
	}
	delete it;
	return nbfaces;
}

///////////////////////////////////////////////////////////////////////////////
///Create and iterator which iterate on elements linked with the current element.
///The iterator must be free by the caller (call delete myIterator).
///@param type The of elements on which you want to iterate
///@return An iterator, that you must free when you no longer need it
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshElement *> * SMDS_MeshElement::
	elementsIterator(SMDSAbs_ElementType type) const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshElement*>
	{
		const SMDS_MeshElement * myElement;
		bool myMore;
	  public:
		MyIterator(const SMDS_MeshElement * element):
			myElement(element),myMore(true)
		{
		}

		bool more()
		{
			return myMore;
		}

		const SMDS_MeshElement* next()
		{
			myMore=false;
			return myElement;	
		}	
	};
	
	if(type==GetType()) return new MyIterator(this);
	else 
	{
		MESSAGE("Iterator not implemented");		
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///Return the ID of the element
///////////////////////////////////////////////////////////////////////////////
int SMDS_MeshElement::GetID() const
{
	return myID;
}

bool operator<(const SMDS_MeshElement& e1, const SMDS_MeshElement& e2)
{
	if(e1.GetType()!=e2.GetType()) return false;
	switch(e1.GetType())
	{
	case SMDSAbs_Node:
		return static_cast<const SMDS_MeshNode &>(e1) <
			static_cast<const SMDS_MeshNode &>(e2);

	case SMDSAbs_Edge:
		return static_cast<const SMDS_MeshEdge &>(e1) <
			static_cast<const SMDS_MeshEdge &>(e2);

	case SMDSAbs_Face:
		return static_cast<const SMDS_MeshFace &>(e1) <
			static_cast<const SMDS_MeshFace &>(e2);

	case SMDSAbs_Volume:
		return static_cast<const SMDS_MeshVolume &>(e1) <
			static_cast<const SMDS_MeshVolume &>(e2);

	default : MESSAGE("Internal Error");
	}
}
