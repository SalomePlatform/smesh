using namespace std;
// File:	SMDS_MeshNode.cxx
// Created:	Wed Jan 23 17:02:11 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@coulox.paris1.matra-dtv.fr>


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

