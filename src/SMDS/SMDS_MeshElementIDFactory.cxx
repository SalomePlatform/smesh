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
//  File   : SMDS_MeshElementIDFactory.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH


#include "SMDS_MeshElementIDFactory.hxx"
#include "SMDS_MeshElement.hxx"

using namespace std;

//=======================================================================
//function : SMDS_MeshElementIDFactory
//purpose  : 
//=======================================================================
SMDS_MeshElementIDFactory::SMDS_MeshElementIDFactory():SMDS_MeshIDFactory()
{
}

//=======================================================================
//function : BindID
//purpose  : 
//=======================================================================
bool SMDS_MeshElementIDFactory::BindID(int ID, SMDS_MeshElement * elem)
{
	bool bound=myIDElements.insert(
		map<int, SMDS_MeshElement*>::value_type(ID,elem)).second;
	if(bound) elem->myID=ID;
	return bound;
}

//=======================================================================
//function : MeshElement
//purpose  : 
//=======================================================================
SMDS_MeshElement* SMDS_MeshElementIDFactory::MeshElement(int ID)
{
    map<int, SMDS_MeshElement*>::iterator it=myIDElements.find(ID);
    if(it==myIDElements.end()) return NULL; else return (*it).second;
}


//=======================================================================
//function : GetFreeID
//purpose  : 
//=======================================================================
int SMDS_MeshElementIDFactory::GetFreeID()
{
  int ID;
  do {
    ID = SMDS_MeshIDFactory::GetFreeID();
  } while (myIDElements.find(ID) != myIDElements.end());
  return ID;
}

//=======================================================================
//function : ReleaseID
//purpose  : 
//=======================================================================
void SMDS_MeshElementIDFactory::ReleaseID(const int ID)
{
  myIDElements.erase(ID);
  SMDS_MeshIDFactory::ReleaseID(ID);
}

//=======================================================================
//function : GetMaxID
//purpose  : 
//=======================================================================

int SMDS_MeshElementIDFactory::GetMaxID() const
{
  map<int, SMDS_MeshElement*>::const_reverse_iterator it = myIDElements.rbegin();
  if ( it !=  myIDElements.rend() )
    return (*it).first;

  return 0;
}

//=======================================================================
//function : GetMinID
//purpose  : 
//=======================================================================

int SMDS_MeshElementIDFactory::GetMinID() const
{
  map<int, SMDS_MeshElement*>::const_iterator it = myIDElements.begin();
  if ( it !=  myIDElements.end() )
    return (*it).first;

  return 0;
}

