//  SMESH DriverMED : driver to read and write 'med' files
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
//  File   : DriverMED_W_SMESHDS_Mesh.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_W_SMESHDS_MESH

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Writer.h"

#include <string>

extern "C"
{
#include <med.h>
}

class DriverMED_W_SMESHDS_Mesh:public Mesh_Writer
{

  public:DriverMED_W_SMESHDS_Mesh();
	~DriverMED_W_SMESHDS_Mesh();

	void Add();
	void Write();
	void SetMesh(SMDS_Mesh * aMesh);
	void SetFile(string);

	void SetFileId(med_idt);
	void SetMeshId(int);

	void CreateFamily(char *, char *, int, med_int);

  private:  SMDS_Mesh * myMesh;
	string myFile;
	med_idt myFileId;
	int myMeshId;

};
#endif
