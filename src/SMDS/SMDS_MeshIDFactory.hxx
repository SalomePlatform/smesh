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
//  File   : SMDS_MeshIDFactory.hxx
//  Module : SMESH

#ifndef _SMDS_MeshIDFactory_HeaderFile
#define _SMDS_MeshIDFactory_HeaderFile

#include "SMDS_MeshObject.hxx"
#include <set>


class SMDS_MeshIDFactory:public SMDS_MeshObject
{
  public:
	virtual int GetFreeID();
	virtual void ReleaseID(int ID);

  protected:
	SMDS_MeshIDFactory();
	int myMaxID;
	std::set<int> myPoolOfID;
};

#endif
