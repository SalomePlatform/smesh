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
//  File   : SMDS_MeshElement.hxx
//  Module : SMESH

#ifndef _SMDS_MeshElement_HeaderFile
#define _SMDS_MeshElement_HeaderFile
	
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_MeshObject.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_MeshElementIDFactory.hxx"

#include <vector>
#include <iostream>

class SMDS_MeshNode;
class SMDS_MeshEdge;
class SMDS_MeshFace;	

///////////////////////////////////////////////////////////////////////////////
/// Base class for elements
///////////////////////////////////////////////////////////////////////////////
class SMDS_MeshElement:public SMDS_MeshObject
{

  public:
	SMDS_ElemIteratorPtr nodesIterator() const;
	SMDS_ElemIteratorPtr edgesIterator() const;
	SMDS_ElemIteratorPtr facesIterator() const;
	virtual SMDS_ElemIteratorPtr
		elementsIterator(SMDSAbs_ElementType type) const;

	virtual int NbNodes() const;
	virtual int NbEdges() const;
	virtual int NbFaces() const;
	int GetID() const;

	///Return the type of the current element
	virtual SMDSAbs_ElementType GetType() const = 0;
	friend std::ostream & operator <<(std::ostream & OS, const SMDS_MeshElement *);
	friend bool SMDS_MeshElementIDFactory::BindID(int ID,SMDS_MeshElement*elem);

  protected:
	SMDS_MeshElement(int ID=-1);
	virtual void Print(std::ostream & OS) const;
	
  private:
	int myID;
};

#endif
