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
//  File   : DriverDAT_R_SMESHDS_Mesh.cxx
//  Module : SMESH

using namespace std;
#include "DriverDAT_R_SMESHDS_Mesh.h"
#include "DriverDAT_R_SMDS_Mesh.h"

#include "utilities.h"

DriverDAT_R_SMESHDS_Mesh::DriverDAT_R_SMESHDS_Mesh()
{
	;
}

DriverDAT_R_SMESHDS_Mesh::~DriverDAT_R_SMESHDS_Mesh()
{
	;
}

void DriverDAT_R_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	//myMesh = SMESHDS_Mesh *::DownCast(aMesh);
	myMesh = aMesh;
}

void DriverDAT_R_SMESHDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverDAT_R_SMESHDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverDAT_R_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverDAT_R_SMESHDS_Mesh::Add()
{
	;
}

void DriverDAT_R_SMESHDS_Mesh::Read()
{
	string myClass = string("SMDS_Mesh");
	string myExtension = string("DAT");

	MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 1");
	DriverDAT_R_SMDS_Mesh *myReader = new DriverDAT_R_SMDS_Mesh;

	MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 2");
	myReader->SetMesh(myMesh);
	MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 3");
	myReader->SetFile(myFile);
	//myReader->SetFileId(myFileId);

	MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 4");
	myReader->Read();

}
