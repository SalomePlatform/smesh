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
//  File   : SMDS_VolumeOfFaces.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH

#include "SMDS_VolumeOfFaces.hxx"
#include "SMDS_IteratorOfElements.hxx"
//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void SMDS_VolumeOfFaces::Print(ostream & OS) const
{
	OS << "volume <" << GetID() << "> : ";
	int i;
	for (i = 0; i < myFaces.size()-1; ++i) OS << myFaces[i] << ",";
	OS << myFaces[i]<< ") " << endl;
}


int SMDS_VolumeOfFaces::NbFaces() const
{
	return myFaces.size();
}

class SMDS_VolumeOfFaces_MyIterator:public SMDS_ElemIterator
{
  const vector<const SMDS_MeshFace*>& mySet;
  int index;
 public:
  SMDS_VolumeOfFaces_MyIterator(const vector<const SMDS_MeshFace*>& s):
    mySet(s),index(0) {}

  bool more()
  {
    return index<mySet.size();
  }

  const SMDS_MeshElement* next()
  {
    index++;
    return mySet[index-1];
  }
};

SMDS_ElemIteratorPtr SMDS_VolumeOfFaces::
	elementsIterator(SMDSAbs_ElementType type) const
{
  switch(type)
  {
  case SMDSAbs_Volume:
    return SMDS_MeshElement::elementsIterator(SMDSAbs_Volume);
  case SMDSAbs_Face:
    return SMDS_ElemIteratorPtr(new SMDS_VolumeOfFaces_MyIterator(myFaces));
  default:
    return SMDS_ElemIteratorPtr
      (new SMDS_IteratorOfElements
       (this,type,SMDS_ElemIteratorPtr(new SMDS_VolumeOfFaces_MyIterator(myFaces))));
  }
}

SMDS_VolumeOfFaces::SMDS_VolumeOfFaces(const SMDS_MeshFace * face1,
                                       const SMDS_MeshFace * face2,
                                       const SMDS_MeshFace * face3,
                                       const SMDS_MeshFace * face4)
{
	myFaces.resize(4);
	myFaces[0]=face1;
	myFaces[1]=face2;
	myFaces[2]=face3;
	myFaces[3]=face4;
}

SMDS_VolumeOfFaces::SMDS_VolumeOfFaces(const SMDS_MeshFace * face1,
                                       const SMDS_MeshFace * face2,
                                       const SMDS_MeshFace * face3,
                                       const SMDS_MeshFace * face4,
                                       const SMDS_MeshFace * face5)
{
	myFaces.resize(5);
	myFaces[0]=face1;
	myFaces[1]=face2;
	myFaces[2]=face3;
	myFaces[3]=face4;
	myFaces[4]=face5;
}

SMDS_VolumeOfFaces::SMDS_VolumeOfFaces(const SMDS_MeshFace * face1,
                                       const SMDS_MeshFace * face2,
                                       const SMDS_MeshFace * face3,
                                       const SMDS_MeshFace * face4,
                                       const SMDS_MeshFace * face5,
                                       const SMDS_MeshFace * face6)
{
	myFaces.resize(6);
	myFaces[0]=face1;
	myFaces[1]=face2;
	myFaces[2]=face3;
	myFaces[3]=face4;
	myFaces[4]=face5;
	myFaces[5]=face6;
}

