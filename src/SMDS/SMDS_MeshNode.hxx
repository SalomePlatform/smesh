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
//  File   : SMDS_MeshNode.hxx
//  Module : SMESH

#ifndef _SMDS_MeshNode_HeaderFile
#define _SMDS_MeshNode_HeaderFile

#include "SMDS_MeshElement.hxx"
#include "SMDS_Position.hxx"
#include <NCollection_List.hxx>

class SMDS_MeshNode:public SMDS_MeshElement
{

  public:
	SMDS_MeshNode(double x, double y, double z);
	void Print(std::ostream & OS) const;
	double X() const;
	double Y() const;
	double Z() const;
	void AddInverseElement(const SMDS_MeshElement * ME);
	void RemoveInverseElement(const SMDS_MeshElement * parent);
	void ClearInverseElements();
	bool emptyInverseElements();
	SMDS_ElemIteratorPtr GetInverseElementIterator() const;	
	void SetPosition(const SMDS_PositionPtr& aPos);
	const SMDS_PositionPtr& GetPosition() const;
	SMDSAbs_ElementType GetType() const;
	int NbNodes() const;
	void setXYZ(double x, double y, double z);
	friend bool operator<(const SMDS_MeshNode& e1, const SMDS_MeshNode& e2);

  protected:
	SMDS_ElemIteratorPtr
		elementsIterator(SMDSAbs_ElementType type) const;

  private:
	double myX, myY, myZ;
	SMDS_PositionPtr myPosition;
	NCollection_List<const SMDS_MeshElement*> myInverseElements;
};

#endif
