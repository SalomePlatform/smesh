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
//  File   : SMDS_MeshGroup.hxx
//  Module : SMESH

#ifndef _SMDS_MeshGroup_HeaderFile
#define _SMDS_MeshGroup_HeaderFile

#include "SMDS_Mesh.hxx"
#include <set>
using namespace std;

class SMDS_MeshGroup:public SMDS_MeshObject
{
  public:
	SMDS_MeshGroup(const SMDS_Mesh * aMesh);
	const SMDS_MeshGroup * AddSubGroup();
	virtual bool RemoveSubGroup(const SMDS_MeshGroup* aGroup);
	virtual bool RemoveFromParent();
	void Clear();
	void Add(const SMDS_MeshElement * ME);
	void Remove(const SMDS_MeshElement * ME);
	bool IsEmpty() const;
	int Extent() const;
	SMDSAbs_ElementType Type() const;
	bool Contains(const SMDS_MeshElement * ME) const;
	
	 ~SMDS_MeshGroup();
  private:
	SMDS_MeshGroup(SMDS_MeshGroup* parent);
	const SMDS_Mesh *myMesh;
	SMDSAbs_ElementType myType;
	set<const SMDS_MeshElement *> myElements;
	SMDS_MeshGroup * myParent;
	list<const SMDS_MeshGroup*> myChildren;
};
#endif
