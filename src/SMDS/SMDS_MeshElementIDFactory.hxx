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
//  File   : SMDS_MeshElementIDFactory.hxx
//  Module : SMESH

#ifndef _SMDS_MeshElementIDFactory_HeaderFile
#define _SMDS_MeshElementIDFactory_HeaderFile

#include "SMDS_MeshIDFactory.hxx"

#include <map>
using namespace std;

class SMDS_MeshElement;

class SMDS_MeshElementIDFactory:public SMDS_MeshIDFactory
{
  public:
	SMDS_MeshElementIDFactory();
	bool BindID(int ID, SMDS_MeshElement * elem);
	SMDS_MeshElement * MeshElement(int ID);
	virtual int GetFreeID();
	virtual void ReleaseID(int ID);
  private:
	map<int, SMDS_MeshElement *> myIDElements;

};

#endif
