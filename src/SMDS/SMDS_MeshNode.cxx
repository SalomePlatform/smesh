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
//  File   : SMDS_MeshNode.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

using namespace std;
#include "SMDS_MeshNode.ixx"
#include "SMDS_ListIteratorOfListOfMeshElement.hxx"
#include "SMDS_SpacePosition.hxx"

static Handle(SMDS_Position)& StaticInstancePosition()
{
  static Handle(SMDS_SpacePosition) staticpos;
  if (staticpos.IsNull())
    staticpos = new SMDS_SpacePosition();

  return staticpos;
}

//=======================================================================
//function : SMDS_MeshNode
//purpose  : 
//=======================================================================

SMDS_MeshNode::SMDS_MeshNode(const Standard_Integer ID,
			     const Standard_Real x, const Standard_Real y, const Standard_Real z) :
  SMDS_MeshElement(ID,1,SMDSAbs_Node),myPnt(x,y,z),myPosition(StaticInstancePosition())
{
}


//=======================================================================
//function : RemoveInverseElement
//purpose  : 
//=======================================================================

void SMDS_MeshNode::RemoveInverseElement(const Handle(SMDS_MeshElement)& parent)
{

  SMDS_ListIteratorOfListOfMeshElement itLstInvCnx(myInverseElements);

  for (;itLstInvCnx.More();itLstInvCnx.Next()) {
    Handle(SMDS_MeshElement)& ME = itLstInvCnx.Value();
    if (ME->IsSame(parent))
      myInverseElements.Remove(itLstInvCnx);
    if (!itLstInvCnx.More())
      break;
  }
}


//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshNode::Print(Standard_OStream& OS) const
{
  OS << "Node <" << myID << "> : X = " << myPnt.X() << " Y = " << myPnt.Y() << " Z = " << myPnt.Z() << endl;
}


//=======================================================================
//function : SetPosition
//purpose  : 
//=======================================================================

void SMDS_MeshNode::SetPosition(const Handle(SMDS_Position)& aPos)
{
  myPosition = aPos;
}

//=======================================================================
//function : GetPosition
//purpose  : 
//=======================================================================

Handle(SMDS_Position) SMDS_MeshNode::GetPosition() const
{
  return myPosition;
}

