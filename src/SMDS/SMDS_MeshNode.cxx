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


#include "SMDS_MeshNode.hxx"
#include "SMDS_SpacePosition.hxx"
#include "SMDS_IteratorOfElements.hxx"

using namespace std;

//=======================================================================
//function : SMDS_MeshNode
//purpose  : 
//=======================================================================

SMDS_MeshNode::SMDS_MeshNode(double x, double y, double z):
	myX(x), myY(y), myZ(z),
	myPosition(SMDS_SpacePosition::originSpacePosition())
{
}

//=======================================================================
//function : RemoveInverseElement
//purpose  : 
//=======================================================================

void SMDS_MeshNode::RemoveInverseElement(const SMDS_MeshElement * parent)
{
	myInverseElements.erase(parent);
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_MeshNode::Print(ostream & OS) const
{
	OS << "Node <" << GetID() << "> : X = " << myX << " Y = "
		<< myY << " Z = " << myZ << endl;
}

//=======================================================================
//function : SetPosition
//purpose  : 
//=======================================================================

void SMDS_MeshNode::SetPosition(const SMDS_PositionPtr& aPos)
{
	myPosition = aPos;
}

//=======================================================================
//function : GetPosition
//purpose  : 
//=======================================================================

const SMDS_PositionPtr& SMDS_MeshNode::GetPosition() const
{
	return myPosition;
}

class SMDS_MeshNode_MyInvIterator:public SMDS_ElemIterator
{
  const set<const SMDS_MeshElement*>& mySet;
  set<const SMDS_MeshElement*>::iterator myIterator;
 public:
  SMDS_MeshNode_MyInvIterator(const set<const SMDS_MeshElement*>& s):
    mySet(s)
  {
    myIterator=mySet.begin();
  }

  bool more()
  {
    return myIterator!=mySet.end();
  }

  const SMDS_MeshElement* next()
  {
    const SMDS_MeshElement* current=*myIterator;
    myIterator++;
    return current;	
  }	
};

SMDS_ElemIteratorPtr SMDS_MeshNode::
	GetInverseElementIterator() const
{
  return SMDS_ElemIteratorPtr(new SMDS_MeshNode_MyInvIterator(myInverseElements));
}

// Same as GetInverseElementIterator but the create iterator only return
// wanted type elements.
class SMDS_MeshNode_MyIterator:public SMDS_ElemIterator
{
  set<const SMDS_MeshElement*> mySet;
  set<const SMDS_MeshElement*>::iterator myIterator;
 public:
  SMDS_MeshNode_MyIterator(SMDSAbs_ElementType type,
                           const set<const SMDS_MeshElement*>& s)
  {
    const SMDS_MeshElement * e;
    bool toInsert;
    set<const SMDS_MeshElement*>::iterator it=s.begin();
    while(it!=s.end())
    {
      e=*it;
      switch(type)
      {
      case SMDSAbs_Edge: toInsert=true; break;
      case SMDSAbs_Face: toInsert=(e->GetType()!=SMDSAbs_Edge); break;
      case SMDSAbs_Volume: toInsert=(e->GetType()==SMDSAbs_Volume); break;
      }
      if(toInsert) mySet.insert(e);
      it++;
    }
    myIterator=mySet.begin();
  }

  bool more()
  {
    return myIterator!=mySet.end();
  }

  const SMDS_MeshElement* next()
  {
    const SMDS_MeshElement* current=*myIterator;
    myIterator++;
    return current;
  }
};

SMDS_ElemIteratorPtr SMDS_MeshNode::
	elementsIterator(SMDSAbs_ElementType type) const
{
  if(type==SMDSAbs_Node)
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Node); 
  else
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,
        SMDS_ElemIteratorPtr(new SMDS_MeshNode_MyIterator(type, myInverseElements))));
}

int SMDS_MeshNode::NbNodes() const
{
	return 1;
}

double SMDS_MeshNode::X() const
{
	return myX;
}

double SMDS_MeshNode::Y() const
{
	return myY;
}

double SMDS_MeshNode::Z() const
{
	return myZ;
}

void SMDS_MeshNode::setXYZ(double x, double y, double z)
{
	myX=x;
	myY=y;
	myZ=z;	
}

SMDSAbs_ElementType SMDS_MeshNode::GetType() const
{
	return SMDSAbs_Node;
}

//=======================================================================
//function : AddInverseElement
//purpose  :
//=======================================================================
void SMDS_MeshNode::AddInverseElement(const SMDS_MeshElement* ME)
{
	myInverseElements.insert(ME);
}

//=======================================================================
//function : ClearInverseElements
//purpose  :
//=======================================================================
void SMDS_MeshNode::ClearInverseElements()
{
	myInverseElements.clear();
}

bool SMDS_MeshNode::emptyInverseElements()
{
	return myInverseElements.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// To be used with STL set
///////////////////////////////////////////////////////////////////////////////
bool operator<(const SMDS_MeshNode& e1, const SMDS_MeshNode& e2)
{
	return e1.GetID()<e2.GetID();
	/*if(e1.myX<e2.myX) return true;
	else if(e1.myX==e2.myX)
	{
	 	if(e1.myY<e2.myY) return true;
		else if(e1.myY==e2.myY) return (e1.myZ<e2.myZ);
		else return false;
	}
	else return false;*/
}

