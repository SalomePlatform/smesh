//  SMESH DriverDAT : driver to read and write 'dat' files
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
//  File   : DriverDAT_R_SMDS_Mesh.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERDAT_R_SMDS_MESH
#define _INCLUDE_DRIVERDAT_R_SMDS_MESH

#include <stdio.h>

#include "SMDS_Mesh.hxx"
#include "Mesh_Reader.h"

class DriverDAT_R_SMDS_Mesh:public Mesh_Reader
{

  public:DriverDAT_R_SMDS_Mesh();
	~DriverDAT_R_SMDS_Mesh();

	void Add();
	void Read();
	void SetMesh(SMDS_Mesh * aMesh);
	void SetFile(string);

	void SetFileId(FILE *);
	void SetMeshId(int);

  private:  SMDS_Mesh * myMesh;
	string myFile;
	FILE *myFileId;
	int myMeshId;

};
#endif
