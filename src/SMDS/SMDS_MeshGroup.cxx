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
//  File   : SMDS_MeshGroup.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
#include "SMDS_MeshGroup.hxx"
#include "utilities.h"
//=======================================================================
//function : SMDS_MeshGroup
//purpose  : 
//=======================================================================

SMDS_MeshGroup::SMDS_MeshGroup(const SMDS_Mesh * aMesh)
	:myMesh(aMesh),myType(SMDSAbs_All), myParent(NULL)
{
}

//=======================================================================
//function : SMDS_MeshGroup
//purpose  : 
//=======================================================================

SMDS_MeshGroup::SMDS_MeshGroup(SMDS_MeshGroup * parent)
	:myMesh(parent->myMesh),myType(SMDSAbs_All), myParent(parent)
{
}

//=======================================================================
//function : AddSubGroup
//purpose  : 
//=======================================================================

const SMDS_MeshGroup *SMDS_MeshGroup::AddSubGroup()
{
	const SMDS_MeshGroup * subgroup = new SMDS_MeshGroup(this);
	myChildren.insert(myChildren.end(),subgroup);
	return subgroup;
}

//=======================================================================
//function : RemoveSubGroup
//purpose  : 
//=======================================================================

bool SMDS_MeshGroup::RemoveSubGroup(const SMDS_MeshGroup * aGroup)
{
	bool found = false;	
	list<const SMDS_MeshGroup*>::iterator itgroup;
	for(itgroup=myChildren.begin(); itgroup!=myChildren.end(); itgroup++)
	{
		const SMDS_MeshGroup* subgroup=*itgroup;
		if (subgroup == aGroup)
		{
			found = true;
			myChildren.erase(itgroup);
		}
	}

	return found;
}

//=======================================================================
//function : RemoveFromParent
//purpose  : 
//=======================================================================

bool SMDS_MeshGroup::RemoveFromParent()
{
	
	if (myParent==NULL) return false;
	else
	{
		return (myParent->RemoveSubGroup(this));
	}
}
//=======================================================================
//function : Clear
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Clear()
{
	myElements.clear();
	myType = SMDSAbs_All;
}

//=======================================================================
//function : IsEmpty
//purpose  : 
//=======================================================================

bool SMDS_MeshGroup::IsEmpty() const
{
	return myElements.empty();
}

//=======================================================================
//function : Extent
//purpose  : 
//=======================================================================

int SMDS_MeshGroup::Extent() const
{
	return myElements.size();
}

//=======================================================================
//function : Add
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Add(const SMDS_MeshElement * ME)
{
	// the type of the group is determined by the first element added
	if (myElements.empty()) myType = ME->GetType();
	else if (ME->GetType() != myType)
		MESSAGE("SMDS_MeshGroup::Add : Type Mismatch");
	
	myElements.insert(ME);
}

//=======================================================================
//function : Remove
//purpose  : 
//=======================================================================

void SMDS_MeshGroup::Remove(const SMDS_MeshElement * ME)
{
	myElements.erase(ME);
	if (myElements.empty()) myType = SMDSAbs_All;
}

//=======================================================================
//function : Type
//purpose  : 
//=======================================================================

SMDSAbs_ElementType SMDS_MeshGroup::Type() const
{
	return myType;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

bool SMDS_MeshGroup::Contains(const SMDS_MeshElement * ME) const
{
	return myElements.find(ME)!=myElements.end();
}
